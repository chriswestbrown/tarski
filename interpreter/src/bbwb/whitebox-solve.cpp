#include "whitebox-solve.h"
#include "deduce-sign.h"
#include "poly-explain.h"


//TODO: REFACTOR "TODED" Code
//TODO: REFACTOR DEDUCE-SIGN DATA STRUCTURES
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
  
  //Generate an IntPolyRef for every single variable. Also, log the sign
  void WBSolver::varsToPoly() {
    for (VarSet::iterator itr = allVars.begin(); itr != allVars.end(); ++itr) {
      IntPolyRef ipr = new IntPolyObj(*itr);
      singleVars.insert(ipr);
    }
  }

  //populate singleVarsDeduction. Each single var gets a deduction chance on every multivar
  void WBSolver::populateSingleVars() {
    for (set<IntPolyRef>::iterator sItr = singleVars.begin(); sItr != singleVars.end(); ++sItr) {
      for (set<IntPolyRef>::iterator mItr = multiVars.begin(); mItr != multiVars.end(); ++mItr){
        IntPolyRef p1 = *sItr;
        IntPolyRef p2 = *mItr;
        if ((p1->getVars() & p2->getVars()).any()) {
          //std::cerr << "adding first: "; p1->write(*PM); std::cerr << " second: ";  p2->write(*PM); std::cerr << " done0" << std::endl;
          pair<IntPolyRef, IntPolyRef> p((p1), (p2));
          singleVarsDed.insert(p);
        }
      }
    }
  }

  //populate multiVarsDed. Every multivariable factor gets a deduction chance on all multivariable factors
  void WBSolver::populateMultiVars() {
    for (set<IntPolyRef>::iterator mItr1 =
	   multiVars.begin(); mItr1 != multiVars.end(); ++mItr1){
      IntPolyRef p1 = *mItr1;
      for (set<IntPolyRef>::iterator mItr2 = multiVars.begin(); mItr2 != multiVars.end(); ++mItr2){
        IntPolyRef p2 = *mItr2;
        if (!(p1->equal(p2)) && (p1->getVars() & p2->getVars()).any()) {
          pair<IntPolyRef, IntPolyRef> p((p1), (p2));
          multiVarsDed.insert(p);
        }
      }
      
      for (set<IntPolyRef>::iterator sItr = singleVars.begin();
	   sItr != singleVars.end(); ++sItr) {
        IntPolyRef p2 = *sItr;
        if ((p1->getVars() & p2->getVars()).any()) {
          pair<IntPolyRef, IntPolyRef> p((p1), (p2));
          multiVarsDed.insert(p);
        }
      }
      
    }
  }

  /*
    Does a single deduceSign2 call on a single variable factor to some multi variable factor
    If the returned deduction teaches us nothing new, then do a different call
    If we exhaust all possible single-to-multi variable deduceSign2 instances, then 
    attempt polynomialSign
   */
  DedExp WBSolver::doSingleDeduce(bool& success) {
    if (singleVarsDed.empty()) return doMultiDeduce(success);
    std::set<pair<IntPolyRef, IntPolyRef>>::iterator it = singleVarsDed.begin();
    pair<IntPolyRef, IntPolyRef> p = *it;
    lastUsed = p.first;
    tuple<VarKeyedMap<int>, VarSet, short> res = Interval::deduceSign2(dedM->getVars(), PM, p.first, p.second, dedM->getSign(p.first), dedM->getSign(p.second));
    singleVarsDed.erase(it);
    if (get<2>(res) == ALOP) return doSingleDeduce(success);
    return toDed(get<0>(res), get<1>(res), p.first, p.second, get<2>(res), dedM->getSign(p.second), Deduction::DEDUC);
  }

  /*
    Performs a single call of polynomial sign
    If the returned deduction teaches us nothing new, then do a different call
    If we exhaust all polynomials and learn nothing, 
    attempt deduceSign2 from multivariable factor to another multi variable factor
   */
  DedExp WBSolver::doPolySigns(bool& success) {
    if (polySigns.empty()) return doSingleDeduce(success);
    std::set<IntPolyRef>::iterator it = polySigns.begin();
    IntPolyRef p = *it;
    polySigns.erase(it);
    lastUsed = p;
    short sign = p->signDeduce(dedM->getVars());
    assert(sign != NOOP);
    VarKeyedMap<int> res;
    if (sign == ALOP) return doPolySigns(success);
    else {
      FernPolyIter F(p, dedM->getVars());
      bool success = true;
      res = select(dedM->getVars(), F, sign, success);
      if (!success) {
        throw TarskiException("Unable to prove a sign in PolySign in WBSATMANAGER");
      }
    }
    return toDed(res, p->getVars(), p, sign, Deduction::POLYS);
  }


  /*
    Attempt to do deducesign2 with two multi variable factors
    If we learn nothing from a first attempt, try again
    If there is nothing left, return null
   */
  DedExp WBSolver::doMultiDeduce(bool& success) {
    if (multiVarsDed.empty()) {
      DedExp d;
      success = false;
      return d;
    }
    std::set<pair<IntPolyRef, IntPolyRef>>::iterator it = multiVarsDed.begin();
    pair<IntPolyRef, IntPolyRef> p = *it;
    multiVarsDed.erase(it);
    lastUsed = p.first;
    tuple<VarKeyedMap<int>, VarSet, short> res;
    res = Interval::deduceSign2(dedM->getVars(), PM, p.first, p.second, dedM->getSign(p.first), dedM->getSign(p.second));
    if (get<2>(res) == ALOP) return doMultiDeduce(success);
    return toDed(get<0>(res), get<1>(res), p.first, p.second, get<2>(res), dedM->getSign(p.second), Deduction::DEDUC);
  }

  /*
    Attempt to make a deduction via a whitebox algorithm
    Priorities:
    DeduceSign2 from a single variable factor to learn about a multivariable factor
    PolynomialSign on a multi variable factor
    DeduceSign2 from multi to multi variable factor
    If nothing can be learned, then this method is guaranteed to return NULL
   */
  DedExp WBSolver::deduce(TAndRef t, bool& res) {
    if (!polySigns.empty()) return doPolySigns(res);
    else if (!singleVarsDed.empty()) return doSingleDeduce(res);
    else if (!multiVarsDed.empty()) return doMultiDeduce(res);
    res = false;
    DedExp d;
    return d;
  }


  void WBSolver::update(std::vector<Deduction>::const_iterator begin, std::vector<Deduction>::const_iterator end) {
    while (begin != end) {
      TAtomRef t = begin->getDed();
      if (t->F->numFactors() == 1 && !t->F->factorBegin()->first->isConstant()) {
        lastUsed = t->F->factorBegin()->first;
        notify();
      }
      ++begin;
    }
  };


  /*
    If it's a single variable, then I need to recalculate for every polynomical which includes that variable,  and then every multiVar which contains it to every other multiVar that contains it)
    If its a multi variable, then I need to recalculate for every variable that is in it that variable to this multi. I also neeed to recalculate from this multi to every other multi which has a variable in common
  */
  void WBSolver::notify() {
    //Single variable case
    if (!lastUsed->isVar()) {
      VarSet vars = lastUsed->getVars();
      for (VarSet::iterator iter = vars.begin(); iter != vars.end(); ++iter) {
        VarSet var = *iter;
        forward_list<IntPolyRef> depPolys = varToIneq[var];
        for (forward_list<IntPolyRef>::iterator flitr = depPolys.begin(); flitr != depPolys.end(); ++flitr) {
          saveAllVarsDed(*flitr); //Every polynomial which contains this var
          saveAllPolysDed(*flitr); //Every polynomial which contains this var to all other polys
        }
      }
    }
    else {
      saveAllVarsDed(lastUsed); //Every variable in this polynomial
      saveAllPolysDed(lastUsed); //Every polynomial with a variable in common with this var
      saveAllPolySigns(lastUsed);
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
    if (!(mono->isVar())) return;
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
    for (std::set<IntPolyRef>::iterator iter = multiVars.begin(); iter != multiVars.end(); ++iter) {
      IntPolyRef poly2 = *iter;
      if (((poly2->getVars() & poly->getVars()).any())  &&
          !(poly2->equal(poly))) {
        pair<IntPolyRef, IntPolyRef> p(poly2, poly);
        multiVarsDed.insert(p);
      }

    }
    if (!poly->isVar()) {
      for (std::set<IntPolyRef>::iterator sItr = singleVars.begin(); sItr != singleVars.end(); ++sItr) {
        IntPolyRef poly2 = *sItr;
        if (((poly2->getVars() & poly->getVars()).any())  &&
            !(poly2->equal(poly))) {
          pair<IntPolyRef, IntPolyRef> p((poly2), (poly));
          singleVarsDed.insert(p);
        }
      }
    }
  }



  
  
  /*
    Turn some learned sign information about a polynomial into the object Deduction format
   */
  DedExp WBSolver::toDed(VarKeyedMap<int>& signs, const VarSet& v, IntPolyRef pMain, short sgn, int type) {
    forward_list<TAtomRef> deps;
    if (!pMain->isVar()) {
      for (VarSet::iterator itr = v.begin(), end = v.end(); itr != end; ++itr){
        if (signs[*itr] == ALOP) continue;
        IntPolyRef p = new IntPolyObj(*itr);
        FactRef F = new FactObj(PM);
        F->addFactor(p, 1);
        TAtomRef t = new TAtomObj(F, signs[*itr]);
        deps.push_front(t);
      }
    }
    FactRef F = new FactObj(PM);
    F->addFactor(pMain, 1);
    TAtomRef t = new TAtomObj(F, sgn);
    DedExp d(t, type, deps);
    return d;
  }
  
    /*
    Turn some learned sign information about a polynomial into the object Deduction format
   */
  DedExp WBSolver::toDed(VarKeyedMap<int>& signs, const VarSet& v, IntPolyRef pMain, IntPolyRef p2, short lsgn, short sgn2, int type) {
    forward_list<TAtomRef> deps;
    for (VarSet::iterator itr = v.begin(); itr != v.end(); ++itr) {
      if (signs[*itr] == ALOP) continue;
      IntPolyRef p = new IntPolyObj(*itr);
      FactRef F = new FactObj(PM);
      F->addFactor(p, 1);
      TAtomRef t = new TAtomObj(F, signs[*itr]);
      deps.push_front(t);
    }
    if (sgn2 != ALOP) {
      FactRef F = new FactObj(PM);
      F->addFactor(p2, 1);
      TAtomRef t = new TAtomObj(F, sgn2);
      deps.push_front(t);
    }
    FactRef F = new FactObj(PM);
    F->addFactor(pMain, 1);
    TAtomRef t = new TAtomObj(F, lsgn);
    DedExp d(t, type, deps);
    return d;
  }

} //end namespace
