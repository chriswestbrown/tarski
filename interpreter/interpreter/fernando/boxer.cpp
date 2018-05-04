#include "boxer.h"
#include "deduce-sign.h"
#include "poly-explain.h"
#include "mono-explain.h"
#include "fern-poly-iter.h"
#include "../formula/formula.h"

namespace tarski {
  void Boxer::updateAtom(IntPolyRef p, short sgn) {
    if (refToAtom.find(p) == refToAtom.end()) {
      FactRef F = new FactObj(*PM);
      F->addFactor(p, 1);
      TAtomRef tat = new TAtomObj(F, sgn);
      refToAtom[p] = tat;
      tand->AND(tat);
    }
    else {
      TAtomRef old = refToAtom[p];
      refToAtom[p] = new TAtomObj(old->F, (old->relop&sgn));
      tand->AND(refToAtom[p]);

    }
  }


  /*
    Constructs and initializes the object.
    TODO: Change the maps provided to the dedmanager so its a queue of signs (in the case multiple are provided)
  */
  Boxer::Boxer(TAndRef &taf) : WBManager() {

    tand = taf;
    unsat = false;
    PM = taf->getPolyManagerPtr();
    allVars = taf->getVars();
    std::map<IntPolyRef, std::vector<short>> pSigns;
    VarSet seen;
    for (TAndObj::conjunct_iterator itr = taf->conjuncts.begin(); itr != taf->conjuncts.end(); ++itr) {
      TAtomRef tf = asa<TAtomObj>(*itr);
      if (tf.is_null()) {
        throw TarskiException("in BOXER - unexpected non-atom!");
      }

      //Go through every factor in each atom

      for (std::map<IntPolyRef, int>::iterator fitr = tf->factorsBegin(); fitr != tf->factorsEnd(); ++fitr) {
        //If its a factor for a single atom, continue since we will process single variables later
        IntPolyRef p = fitr->first;
        if (!(p->isVariable().any()) &&  tf->F->MultiplicityMap.size() == 1) {
          refToAtom[p] = tf;
          short sgn = tf->getRelop();
          if (fitr->second < 0) sgn = reverseRelop(sgn);
          pSigns[p].push_back(sgn);
        }
        else if (p->isVariable().any() && tf->F->MultiplicityMap.size() == 1) {
          refToAtom[p] = tf;
          short sgn = tf->getRelop();
          if (fitr->second < 0) sgn = reverseRelop(sgn);
          pSigns[p].push_back(sgn);
          seen = seen | fitr->first->getVars();
          singleVars.insert(fitr->first);
          continue;
        }
        else if (pSigns.find(p) == pSigns.end()) {
          pSigns[p].push_back(ALOP);
        }
        //Log that we will need to run polySigns on this
        polySigns.insert(p);
        //Add this intpolyref to the std::set of all multivariable factors
        if (!(p->isVariable().any())) {
          multiVars.insert(p);
        }
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

      if ((seen | *itr) != seen) {
        IntPolyRef ipr = new IntPolyObj(*itr);
        singleVars.insert(ipr);

      }

    }


    //populate singleVarsDeduction. Each single var gets a deduction chance on every multivar
    for (std::set<IntPolyRef>::iterator sItr = singleVars.begin(); sItr != singleVars.end(); ++sItr) {
      for (std::set<IntPolyRef>::iterator mItr = multiVars.begin(); mItr != multiVars.end(); ++mItr){
        IntPolyRef p1 = *sItr;
        IntPolyRef p2 = *mItr;
        pair<IntPolyRef, IntPolyRef> p((p1), (p2));
        singleVarsDed.insert(p);
        if (verbose ) {
          std::cerr << "adding first: "; p1->write(*PM);
          std::cerr << " second: ";  p2->write(*PM);
          std::cerr << singleVarsDed.size() << std::endl;
        }
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


  Result Boxer::deduceAll() {

    bool dedRound = true;
    bool first = true;
    if (dedM->isUnsat()) {
      if (verbose) std::cerr << "dedM: given conflict\n";
      return dedM->traceBack();

    }

    while (dedRound) {
      //BlackBox Part
      if (verbose) std::cerr << "BB: beginning BB\n";
      dedRound = false;
      std::vector<Deduction *> bbdeds = bbsat(tand);
      short res = dedM->processDeductions(bbdeds);
      if (res == 0) return dedM->traceBack();
      else if (res == 1) dedRound = true;


      //WhiteBox Part
      if (first || dedRound) {
        dedRound = false;
        while (notDone()) {
          //std::cerr << "WB: Beginning round \n";
          Deduction * d = doWBRound();
          if (d != NULL) {
            bool useful = dedM->processDeduction(d);

            if (dedM->isUnsat()) {
              if (verbose) std::cerr << "WB: Unsat detected\n";
              return dedM->traceBack();
            }
            else if (useful) {
              if (verbose) std::cerr << "Boxer: WB: dedRound std::set to true\n";
              dedRound = true;

              if (verbose) {
                TAtomRef t = d->getDed();
                std::cerr << "Deduced"; t->write(); std::cerr << endl;
              }
              IntPolyRef p = d->getDed()->F->MultiplicityMap.begin()->first;
              short sgn = d->getDed()->relop;
              updateAtom(p, sgn);
              saveDeductions();
            }
          }
          else {
            //std::cerr << "WB: Null deduction\n";
            delete d;
          }
        }
        first = false;
      }
    
    }
    Result r;
    return r;
  }


}//namespace tarski
