#include "wbsat.h"
#include "deduce-sign.h"
#include "poly-explain.h"
#include "mono-explain.h"
#include "fern-poly-iter.h"

namespace tarski {

  //Do nothing, just initalize the list properly for inheritance purposes
  WBManager::WBManager(): varToIneq(forward_list<IntPolyRef>()) {//Do nothing, just initalize the list properly

  }


  /*
    Generates a new BoxManager
    Initializes all the data structures
    taf - the Conjunction we desire to do computations on
    knownInfo - the information we know about all the signs
  */
  WBManager::WBManager(TAndRef &taf)
    : varToIneq(forward_list<IntPolyRef>())
  {
    unsat = false;
    PM = taf->getPolyManagerPtr();
    allVars = taf->getVars();
    std::map<IntPolyRef, std::vector<short>> pSigns;
    //Go through every atom obj
    for (TAndObj::conjunct_iterator itr = taf->conjuncts.begin(); itr != taf->conjuncts.end(); ++itr) {
      TAtomRef tf = asa<TAtomObj>(*itr);
      if (tf.is_null()) {
        throw TarskiException("in WBMANAGER - unexpected non-atom!");
      }
      //Go through every factor in each atom
      for (std::map<IntPolyRef, int>::iterator fitr = tf->factorsBegin(); fitr != tf->factorsEnd(); ++fitr) {
        //If its a factor for a single atom, continue since we will process single variables later
        if (fitr->first->isVariable().any() && tf->F->MultiplicityMap.size() == 1) {
          pSigns[fitr->first].push_back(tf->getRelop());
          continue;
        }

        IntPolyRef p = fitr->first;

        //Log that we will need to run polySigns on this
        polySigns.insert(p);
        //std::cerr << "added "; p->write(*PM); std::cerr << " to multiVars\n";

        //Add this intpolyref to the std::set of all multivariable factors
        multiVars.insert(p);

        //If it is a lone factor, log its sign. Otherwise, if we already have it, skip. Otherwise, save it as ALOP (no known sign info)
        if (tf->F->MultiplicityMap.size() == 1)
          pSigns[p].push_back(tf->getRelop());
        else if (pSigns.find(p) == pSigns.end())
          pSigns[p].push_back(ALOP);

        //Construct std::map from vars to polyrefs. This lets us lookup a variable and determine which multivar polys it appears in 
        VarSet vars = p->getVars();
        for (VarSet::iterator itr = vars.begin(); itr != vars.end(); ++itr) {
          forward_list<IntPolyRef>& tmp = varToIneq[*itr];
          tmp.emplace_front(p);
        }
      }
    }

    //Generate an IntPolyRef for every single variable. Also, log the sign
    for (VarSet::iterator itr = allVars.begin(); itr != allVars.end(); ++itr) {
      IntPolyRef ipr = new IntPolyObj(*itr);
      singleVars.insert(ipr);
    }


    //populate singleVarsDeduction. Each single var gets a deduction chance on every multivar
    for (std::set<IntPolyRef>::iterator sItr = singleVars.begin(); sItr != singleVars.end(); ++sItr) {
      for (std::set<IntPolyRef>::iterator mItr = multiVars.begin(); mItr != multiVars.end(); ++mItr){
        IntPolyRef p1 = *sItr;
        IntPolyRef p2 = *mItr;
        std::cerr << "adding first: "; p1->write(*PM); std::cerr << " second: ";  p2->write(*PM); std::cerr << " done0" << std::endl;
        pair<IntPolyRef, IntPolyRef> p((p1), (p2));
        singleVarsDed.insert(p);
      }
    }

    //populate multiVarsDed. Every multivariable factor gets a deduction chance on all multivariable factors
    for (std::set<IntPolyRef>::iterator mItr1 = multiVars.begin(); mItr1 != multiVars.end(); ++mItr1){
      IntPolyRef p1 = *mItr1;
      for (std::set<IntPolyRef>::iterator mItr2 = multiVars.begin(); mItr2 != multiVars.end(); ++mItr2){
        IntPolyRef p2 = *mItr2;
        if (!(p1->equal(p2))) {
          pair<IntPolyRef, IntPolyRef> p((p1), (p2));
          multiVarsDed.insert(p);
        }
      }

      for (std::set<IntPolyRef>::iterator sItr = singleVars.begin(); sItr != singleVars.end(); ++sItr) {
        IntPolyRef p2 = *sItr;
        pair<IntPolyRef, IntPolyRef> p((p1), (p2));
        multiVarsDed.insert(p);
      }



    }
    dedM = new DedManager(taf);
  }



  /*
    If whitebox determines the formula is unsat, generate a Result object which explains why the formula is UNSAT
    If not, generate a Result object which contains all the known sign information after applying WhiteBox
  */
  Result WBManager::deduceAll() {
    while (notDone()) {
      if (verbose) std::cerr << "WB: Beginning round \n";
      Deduction * res = doWBRound();
      if (res != NULL) {
        bool useful = dedM->processDeduction(res);

        if (dedM->isUnsat()) {
          if (verbose) std::cerr << "WB: Unsat detected\n";
          return dedM->traceBack();
        }
        else if (useful) {
          if (verbose) std::cerr << "WB: Useful, so saving new deductions\n";
          saveDeductions();
        }
      }
      else {
        if (verbose) std::cerr << "WB: Null deduction\n";
        delete res;
      }
    }
    Result r;
    return r;

  }


  /*
    Given a polynomial poly, for all variables in poly,
    generate a pair between that variable and poly and save it as a future deduceSigns pair
  */
  void WBManager::saveAllVarsDed(IntPolyRef poly) {

    if (poly->isVariable().any()) return;
    for (std::set<IntPolyRef>::iterator iter = singleVars.begin(); iter != singleVars.end(); ++iter) {
      IntPolyRef var = *iter;
      if ((var->getVars() | poly->getVars()) == poly->getVars()) {
        pair<IntPolyRef, IntPolyRef> p(var, poly);
        singleVarsDed.insert(p);
      }
    }
  }

  /*
    Given an IntPolyRef mono which contains only one variable,
    for all polynomials p which contain mono.variable,
    save p as a future poly-sign calculation
  */
  void WBManager::saveAllPolySigns(IntPolyRef mono) {
    if (!(mono->isVariable().any())) return;
    VarSet v = mono->getVars();
    for (std::set<IntPolyRef>::iterator iter = multiVars.begin(); iter != multiVars.end(); ++iter) {
      IntPolyRef p = *iter;
      if ((p->getVars() & v) == v) {
        polySigns.insert(p);
      }
    }
  }

  /*
    Given a polynomial poly,
    for all polynomials p which contain a variable in common with poly,
    generate a pair between p and poly and save it as a future deduceSigns pair
  */
  void WBManager::saveAllPolysDed(IntPolyRef poly) {
    polySigns.insert(poly);
    for (std::set<IntPolyRef>::iterator iter = multiVars.begin(); iter != multiVars.end(); ++iter) {
      IntPolyRef poly2 = *iter;
      if (((poly2->getVars() & poly->getVars()).any())  &&  !(poly2->equal(poly))) {
        pair<IntPolyRef, IntPolyRef> p(poly2, poly);
        multiVarsDed.insert(p);
      }

      for (std::set<IntPolyRef>::iterator sItr = singleVars.begin(); sItr != singleVars.end(); ++sItr) {
        IntPolyRef var = *sItr;
        pair<IntPolyRef, IntPolyRef> p((var), (poly));
        singleVarsDed.insert(p);
      }
    }
  }

  //If it's a single variable, then I need to recalculate for every polynomical which includes that variable,  and then every multiVar which contains it to every other multiVar that contains it)
  //If its a multi variable, then I need to recalculate for every variable that is in it that variable to this multi. I also neeed to recalculate from this multi to every other multi which has a variable in common
  void WBManager::saveDeductions(){
    //Single variable case
    VarSet vars = lastUsed->getVars();
    for (VarSet::iterator iter = vars.begin(); iter != vars.end(); ++iter) {
      VarSet var = *iter;
      if (lastUsed->isVariable().any()) {
        forward_list<IntPolyRef> depPolys = varToIneq[var];
        for (forward_list<IntPolyRef>::iterator flitr = depPolys.begin(); flitr != depPolys.end(); ++flitr) {
          if (verbose)  {std::cerr << "Saving Deds for "; (*flitr)->write(*PM); std::cerr << std::endl;}
          saveAllVarsDed(*flitr); //Every polynomial which contains this var
          saveAllPolysDed(*flitr); //Every polynomial which contains this var to all other polys
          saveAllPolySigns(*flitr);
        }
      }
      else {
        saveAllVarsDed(lastUsed); //Every variable in this polynomial
        saveAllPolysDed(lastUsed); //Every polynomial with a variable in common with this var
      }
    }

  }


  /*
    Returns true or false based on whether or not there are any more whitebox calculations to make
  */
  bool WBManager::notDone() {
    return (!singleVarsDed.empty() || !multiVarsDed.empty() || !polySigns.empty());
  }



  Deduction *  WBManager::doWBRound() {

    //std::cerr << "In a round\n";
    if (!singleVarsDed.empty()) {
      //std::cerr << "sVD\n";
      std::set<pair<IntPolyRef, IntPolyRef>>::iterator it = singleVarsDed.begin();
      pair<IntPolyRef, IntPolyRef> p = *it;
      if (verbose) {
        std::cerr << std::endl <<  "WB: Single Vars Deduction on: p: "; p.first->write(*PM);
        std::cerr  << " q: ";  p.second->write(*PM); std::cerr << std::endl;
      }

      lastUsed = p.first;
      tuple<VarKeyedMap<int>, VarSet, short> res = Interval::deduceSign2(dedM->getVars(), PM, p.first, p.second, dedM->getSign(p.first), dedM->getSign(p.second));
      singleVarsDed.erase(it);
      if (verbose)  std::cerr << "WB: LEARNED " << FernPolyIter::numToRelop(get<2>(res)) << std::endl;
      if (get<2>(res) == ALOP) return NULL;
      return toDed(get<0>(res), get<1>(res), p.first, p.second, get<2>(res), dedM->getSign(p.second));
    }
    else if (!polySigns.empty()){
      std::set<IntPolyRef>::iterator it = polySigns.begin();
      IntPolyRef p = *it;
      polySigns.erase(it);
      lastUsed = p;
      short sign = p->signDeduce(dedM->getVars());

      /*
	VarSet testVars = p->getVars();
	cerr << "Poly: "; p->write(*PM); cerr << endl;
	for (VarSet::iterator itr = testVars.begin(), end = testVars.end(); itr != end; ++itr){
        IntPolyRef p = new IntPolyObj(*itr);
        //p->write(*PM);cerr << " sign is " << numToRelop(dedM->getVars()[*itr]) << " " << dedM->getVars()[*itr];
        cerr<<endl;
      }
      */
      VarKeyedMap<int> res;
      //if (sign == NOOP) res = dedM->getVars(); If this case occurs, theres a bug!
      if (sign == ALOP || (sign & dedM->getSign(p)) == dedM->getSign(p)) return NULL;
      else {
        if (verbose ) {std::cerr << "attempting to prove " << numToRelop(sign) << " on "; p->write(*PM); std::cerr << std::endl; }
        FernPolyIter F(p, dedM->getVars());
        bool success = true;
        res = select(dedM->getVars(), F, sign, success);
        if (!success) {
          throw new TarskiException("Unable to prove a sign in PolySign in WBSATMANAGER");
        }
      }
      return toDed(res, p->getVars(), p, sign);
    }
    else if (!multiVarsDed.empty()) {
      std::set<pair<IntPolyRef, IntPolyRef>>::iterator it = multiVarsDed.begin();
      pair<IntPolyRef, IntPolyRef> p = *it;
      if (verbose)  {
        std::cerr << std::endl << "WB: Multi Vars Deduction on:      p: ";
        p.first->write(*PM);
        std::cerr << "      q: "; p.second->write(*PM);  std::cerr << std::endl;
      }
      multiVarsDed.erase(it);
      lastUsed = p.first;
      tuple<VarKeyedMap<int>, VarSet, short> res;
      res = Interval::deduceSign2(dedM->getVars(), PM, p.first, p.second, dedM->getSign(p.first), dedM->getSign(p.second));
      if (get<2>(res) == ALOP || (get<2>(res) & dedM->getSign(p.first)) == dedM->getSign(p.first)) return NULL;
      return toDed(get<0>(res), get<1>(res), p.first, p.second, get<2>(res), dedM->getSign(p.second));
    }


    else {
      throw new TarskiException("ERROR! doWBRound() called when no more deductions exist to be made");
    }


  }
}
