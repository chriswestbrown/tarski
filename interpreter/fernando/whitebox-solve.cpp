#include "whitebox-solve.h"
#include "deduce-sign.h"
#include "poly-explain.h"

namespace tarski {


  using namespace std;
  
  WBSolver::WBSolver(TAndRef taf) :varToIneq(forward_list<IntPolyRef>()) {
    PM = taf->getPolyManagerPtr();
    allVars = taf->getVars(); 
    loadVars(taf);
    varsToPoly();
    populateSingleVars();
    populateMultiVars();
  }

  //Preprocesses variables, populates polysigns, multivars, and varToIneq
  void WBSolver::loadVars(TAndRef taf) {
    for (TAndObj::conjunct_iterator itr = taf->conjuncts.begin();
	 itr != taf->conjuncts.end(); ++itr) {
      TAtomRef tf = asa<TAtomObj>(*itr);
      if (tf.is_null()) {
        throw TarskiException("in WBMANAGER - unexpected non-atom!");
      }

      for (map<IntPolyRef, int>::iterator fitr = tf->factorsBegin();
	   fitr != tf->factorsEnd(); ++fitr) {
        //If its a factor for a single atom, continue since we will process single variables later
        if (fitr->first->isVariable().any() && tf->F->MultiplicityMap.size() == 1) continue;

	IntPolyRef p = fitr->first;

        //Log that we will need to run polySigns on this
        polySigns.insert(p);
        //std::cerr << "added "; p->write(*PM); std::cerr << " to multiVars\n";

        //Add this intpolyref to the std::set of all multivariable factors
        multiVars.insert(p);

        //Populate varToineq
        VarSet vars = p->getVars();
        for (VarSet::iterator itr = vars.begin(); itr != vars.end(); ++itr) {
	  varToIneq[*itr].emplace_front(p);
        }
      }
    }
  }
  
  void WBSolver::varsToPoly() {
    //Generate an IntPolyRef for every single variable. Also, log the sign
    for (VarSet::iterator itr = allVars.begin(); itr != allVars.end(); ++itr) {
      IntPolyRef ipr = new IntPolyObj(*itr);
      singleVars.insert(ipr);
    }
  }

  void WBSolver::populateSingleVars() {
    //populate singleVarsDeduction. Each single var gets a deduction chance on every multivar
    for (set<IntPolyRef>::iterator sItr = singleVars.begin(); sItr != singleVars.end(); ++sItr) {
      for (set<IntPolyRef>::iterator mItr = multiVars.begin(); mItr != multiVars.end(); ++mItr){
        IntPolyRef p1 = *sItr;
        IntPolyRef p2 = *mItr;
        //std::cerr << "adding first: "; p1->write(*PM); std::cerr << " second: ";  p2->write(*PM); std::cerr << " done0" << std::endl;
        pair<IntPolyRef, IntPolyRef> p((p1), (p2));
        singleVarsDed.insert(p);
      }
    }
  }

  void WBSolver::populateMultiVars() {
        //populate multiVarsDed. Every multivariable factor gets a deduction chance on all multivariable factors
    for (set<IntPolyRef>::iterator mItr1 =
	   multiVars.begin(); mItr1 != multiVars.end(); ++mItr1){
      IntPolyRef p1 = *mItr1;
      for (set<IntPolyRef>::iterator mItr2 = multiVars.begin(); mItr2 != multiVars.end(); ++mItr2){
        IntPolyRef p2 = *mItr2;
        if (!(p1->equal(p2))) {
          pair<IntPolyRef, IntPolyRef> p((p1), (p2));
          multiVarsDed.insert(p);
        }
      }
      
      for (set<IntPolyRef>::iterator sItr = singleVars.begin();
	   sItr != singleVars.end(); ++sItr) {
        IntPolyRef p2 = *sItr;
        pair<IntPolyRef, IntPolyRef> p((p1), (p2));
        multiVarsDed.insert(p);
      }
      
      
      
      
    }
  }

  Deduction * WBSolver::doSingleDeduce() {
    //std::cerr << "sVD\n";
      std::set<pair<IntPolyRef, IntPolyRef>>::iterator it = singleVarsDed.begin();
      pair<IntPolyRef, IntPolyRef> p = *it;

      lastUsed = p.first;
      tuple<VarKeyedMap<int>, VarSet, short> res = Interval::deduceSign2(dedM->getVars(), PM, p.first, p.second, dedM->getSign(p.first), dedM->getSign(p.second));
      singleVarsDed.erase(it);
      if (get<2>(res) == ALOP) return NULL;
      return toDed(get<0>(res), get<1>(res), p.first, p.second, get<2>(res), dedM->getSign(p.second));
  }


  Deduction * WBSolver::doPolySigns() {
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
        FernPolyIter F(p, dedM->getVars());
        bool success = true;
        res = select(dedM->getVars(), F, sign, success);
        if (!success) {
          throw new TarskiException("Unable to prove a sign in PolySign in WBSATMANAGER");
        }
      }
      return toDed(res, p->getVars(), p, sign);
  }

  Deduction * WBSolver::doMultiDeduce() {
    if (multiVarsDed.empty()) return NULL;
    std::set<pair<IntPolyRef, IntPolyRef>>::iterator it = multiVarsDed.begin();
    pair<IntPolyRef, IntPolyRef> p = *it;
    multiVarsDed.erase(it);
    lastUsed = p.first;
    tuple<VarKeyedMap<int>, VarSet, short> res;
    res = Interval::deduceSign2(dedM->getVars(), PM, p.first, p.second, dedM->getSign(p.first), dedM->getSign(p.second));
    if (get<2>(res) == ALOP || (get<2>(res) & dedM->getSign(p.first)) == dedM->getSign(p.first)) return doMultiDeduce();
    return toDed(get<0>(res), get<1>(res), p.first, p.second, get<2>(res), dedM->getSign(p.second));
  }
  
  
  Deduction * WBSolver::deduce(TAndRef t) {
    if (!singleVarsDed.empty()) return doSingleDeduce();
    else if (!polySigns.empty()) return doPolySigns();
    else if (!multiVarsDed.empty()) return doMultiDeduce();
    else return NULL;
  }


    //If it's a single variable, then I need to recalculate for every polynomical which includes that variable,  and then every multiVar which contains it to every other multiVar that contains it)
  //If its a multi variable, then I need to recalculate for every variable that is in it that variable to this multi. I also neeed to recalculate from this multi to every other multi which has a variable in common
  void WBSolver::notify() {
    //Single variable case
    VarSet vars = lastUsed->getVars();
    for (VarSet::iterator iter = vars.begin(); iter != vars.end(); ++iter) {
      VarSet var = *iter;
      if (lastUsed->isVariable().any()) {
        forward_list<IntPolyRef> depPolys = varToIneq[var];
        for (forward_list<IntPolyRef>::iterator flitr = depPolys.begin(); flitr != depPolys.end(); ++flitr) {

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
    Given a polynomial poly, for all variables in poly,
    generate a pair between that variable and poly and save it as a future deduceSigns pair
  */
  void WBSolver::saveAllVarsDed(IntPolyRef poly) {

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
  void WBSolver::saveAllPolySigns(IntPolyRef mono) {
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
  void WBSolver::saveAllPolysDed(IntPolyRef poly) {
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



  
  
    
  WBDed * WBSolver::toDed(VarKeyedMap<int> signs, VarSet v, IntPolyRef pMain, short sgn) {
    vector<TAtomRef> deps;
    for (VarSet::iterator itr = v.begin(), end = v.end(); itr != end; ++itr){
      if (signs[*itr] == ALOP) continue;
      IntPolyRef p = new IntPolyObj(*itr);
      FactRef F = new FactObj(PM);
      F->addFactor(p, 1);
      TAtomRef t = new TAtomObj(F, signs[*itr]);
      deps.push_back(t);
    }
    FactRef F = new FactObj(PM);
    F->addFactor(pMain, 1);
    TAtomRef t = new TAtomObj(F, sgn);
    return new WBDed(t, deps, PSGN);
  }


  WBDed * WBSolver::toDed(VarKeyedMap<int> signs, VarSet v, IntPolyRef pMain, IntPolyRef p2, short lsgn, short sgn2) {
    vector<TAtomRef> deps;
    for (VarSet::iterator itr = v.begin(), end = v.end(); itr != end; ++itr){
      if (signs[*itr] == ALOP) continue;
      IntPolyRef p = new IntPolyObj(*itr);
      FactRef F = new FactObj(PM);
      F->addFactor(p, 1);
      TAtomRef t = new TAtomObj(F, signs[*itr]);
      deps.push_back(t);
    }
    FactRef F = new FactObj(PM);
    F->addFactor(pMain, 1);
    TAtomRef t = new TAtomObj(F, lsgn);
    if (sgn2 != ALOP) {
      FactRef F2 = new FactObj(PM);
      F2->addFactor(p2, 1);
      TAtomRef t2 = new TAtomObj(F2, sgn2);
      deps.push_back(t2);
    }
    return new WBDed(t, deps, DEDSGN);
  }

} //end namespace
