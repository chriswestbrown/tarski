#include "deduction.h"
//TODO: Add a sign table for atoms


namespace tarski {


#define C_ITR const_iterator
#define ITR iterator

  void Deduction::write() {
    if (unsat) cout << "UNSAT";
    else {
      cout << name <<  ": ";
      if (!unsat) deduction->write();
    }
    if (given) cout << endl;
    else {
      std::cout << " from  [ ";
      for (unsigned int i = 0; i < deps.size(); i++) {
        if (deps[i]->getRelop() != ALOP) {
          deps[i]->write();
          if (i != deps.size()-1) std::cout << " /\\ ";
        }
      }
      cout << " ]\n";

      
  }
  }

  DedManager::DedManager(TAndRef a) : unsat(false), varSigns(ALOP) {
    PM = a->getPolyManagerPtr();
    for (TAndObj::conjunct_iterator itr = a->begin(), end = a->end(); itr != end; ++itr) {
      processGiven(*itr);
      if (unsat) return;
    }
    givenSize = deds.size();
  }

  //This requires at least one member of a
  DedManager::DedManager(const vector<TAtomRef>& a) : unsat(false), varSigns(ALOP) {
    for (vector<TAtomRef>::C_ITR itr = a.begin(), end = a.end(); itr != end; ++itr) {
      processGiven(*itr);
      if (unsat) return;
    }
    PM = a[0]->getPolyManagerPtr();
    givenSize = deds.size();
  }

  bool DedManager::ManagerComp::operator()(const TAtomRef& A, const TAtomRef& B) {
    if (A->getFactors()->numFactors() < B->getFactors()->numFactors()) return true;
    if (A->getFactors()->numFactors() > B->getFactors()->numFactors()) return false;
    int t = OCOMP(A->getFactors()->getContent(),B->getFactors()->getContent());
    if (t != 0) {
      if (t < 0) return -1;
      return 1;
    }
    FactObj::factorIterator itrA= A->getFactors()->factorBegin();
    FactObj::factorIterator itrB= B->getFactors()->factorBegin();
    while(itrA != A->getFactors()->factorEnd())
      {
        if (itrA->second < itrB->second) return true;
        if (itrA->second > itrB->second) return false;
        if (itrA->first < itrB->first) return true;
        if (itrB->first < itrA->first) return false;
        ++itrA;
        ++itrB;
      }
    return false;
  }

  /*
    Heuristic for simplest is
    Score 0 for best possible case, sign on variable
    Add 25 for every term in a polynomial
    Add 5 for every variable in a term
    Add 100 for every factor
   */
  void DedManager::DedScore::scoreDed() {
    TAtomRef a = d->getDed();
    FactRef f = a->getFactors();
    score += 100 * (f->numFactors() - 1);
    for (FactObj::factorIterator itr = f->factorBegin();
         itr != f->factorEnd(); ++itr) {
      IntPolyRef poly = itr->first;
      if (poly->isVar()) continue;
      VarKeyedMap<int> M;
      FernPolyIter F(poly, M);
      while (!F.isNull()) {
        score += 25 + F.getVars().size()*5;
      }
    }
  }


  bool DedManager::SimpleComp::operator()(const DedScore& A,
                                          const DedScore& B) {
    return A.score < B.score;
  }




  short DedManager::getSgn(TAtomRef t) {
    return (atomToDed.find(t) == atomToDed.end())
      ? ALOP : deds[atomToDed[t]]->getDed()->relop;
  }

  void DedManager::updateVarSigns(TAtomRef t) {
    if (t->F->numFactors() == 1 && t->factorsBegin()->first->isVariable().any() && t->getRelop() != ALOP) {
      VarSet v = t->getVars();
      varSigns[v] = varSigns[v] & t->getRelop();
    }
  }

  //Return 0 for UNSAT
  //Return 1 for learned, but SAT
  //Return 2 for Nothing done
  short DedManager::processDeductions(vector<Deduction *> v) {
    short finRes;
    vector<Deduction *>::ITR itr, end;
    for (itr = v.begin(), end = v.end(); itr != end; ++itr) {
      bool res = processDeduction(*itr);
      if (unsat)  {++itr; break; }
      else if (res == true) finRes = 1;
    }
    if (unsat) {
      while (itr != end) {
        delete *itr;
        ++itr;
      }
      return 0;
    }
    return finRes;
  }



  void DedManager::addGiven(TAtomRef t) {
    vector<int> tmp;
    depIdxs.push_back(tmp);
    atomToDed[t] = deds.size();
    deds.push_back(new Given(t));
  }

  void DedManager::addGCombo(TAtomRef t) {
    vector<int> tmp;
    depIdxs.push_back(tmp);
    deds.push_back(new Given(t));
    TAtomRef t1 = deds[atomToDed[t]]->getDed();
    vector<TAtomRef> atomDeps(2);
    atomDeps[0] = t;
    atomDeps[1] = t1;
    //Adds the old deduction and the new deduction as dependencies
    //Makes a new atom which represents the sign combination of the two
    vector<int> deps;
    deps.push_back(deds.size()-1);
    deps.push_back(atomToDed[t]);
    depIdxs.push_back(deps);
    TAtomRef t2 = new TAtomObj(t->F, t->relop & getSgn(t));
    atomToDed[t] = deds.size();
    if (t2->relop == NOOP) unsat = true;
    deds.push_back(new SignCombo(t2, atomDeps));
  }

   /*
    Add a given to the DedManager, if it gives us something new to learn
    Givens should all be inserted first upon construction

    Also determins wheter this is the only polynomial or variable of its kind (AKA x >0 && x <= 0 supplied in init formula)
    If it is not, then we combine the sizes and add a SignCombo deduction after the given, if the sign combination teaches us something new

    Checks by doing a table lookup
  */
  void DedManager::processGiven(TAtomRef t) {
    if (getSgn(t) == ALOP) addGiven(t);
    else if ((getSgn(t) & t->getRelop()) != getSgn(t)) addGCombo(t);
    else return;
    updateVarSigns(t);
  }

  vector<int> DedManager::getDepIdxs(Deduction * d) {
    vector<int> deps;
    const vector<TAtomRef>& dA = d->getDeps();
    for (vector<TAtomRef>::C_ITR itr = dA.begin(), end = dA.end(); itr != end; ++itr) {
      int idx = -1;
      if ((*itr)->relop == ALOP) continue;
      if (atomToDed.find(*itr) == atomToDed.end()) {
        idx = searchMap(*itr);
        if (idx == -1) {
          (*itr)->write(); cerr << " is the offender \n";
          throw TarskiException("No index for dependency!");
        }
      } 
      if (idx == -1) idx = (atomToDed.find(*itr))->second;
      if (idx == -1) throw TarskiException("Unknown Dependency!");
      deps.push_back(idx);
    }
    return deps;
  }


  void DedManager::addCycle(Deduction * d) {
    deds[atomToDed[d->getDed()]]->addCycle(getDepIdxs(d));
  }

  void DedManager::addDed(Deduction * d){
    depIdxs.push_back(getDepIdxs(d));
    atomToDed[d->getDed()] = deds.size();
    deds.push_back(d);
  }


  /*
    Note that this method modifies d by changing the learned sign
    of d->deduction and adding a dependency!
    If this method is called, the last member of d->deps is atom
    which contains the previously known sign on d!
   */
  void DedManager::addCombo(Deduction * d) {
    Deduction * earlier = deds[atomToDed[d->getDed()]];
    d->deps.push_back(earlier->getDed());
    d->deduction->relop = d->deduction->relop & earlier->getDed()->relop;
    if (d->deduction->relop == NOOP) unsat = true;
    depIdxs.push_back(getDepIdxs(d));
    atomToDed[d->getDed()] = deds.size();
    deds.push_back(d);
  }

  /*
    Returns true and adds a new deduction to the list of deduction, if the deduction learns anything new. Set unsat to true if the new deduction contradicts with known information
    If it doesn't, then don't add the deduction and terminate prematurely. return false
    do nothing if formula is already unsat
  */
  bool DedManager::processDeduction(Deduction * d) {
    //Initial processing of the deduction, check if it teaches anything useful
    //If it teaches us nothing useful, return
    if (d->isUnsat()) {
      depIdxs.push_back(getDepIdxs(d));
      deds.push_back(d);
      unsat = true;
      return true;
    }

    short combo = d->getDed()->getRelop() & getSgn(d->getDed());
    if (d->getDed()->getRelop() == getSgn(d->getDed()))  {
      addCycle(d); 
      return false;
    }
    if (combo == getSgn(d->getDed())) {
      delete d;
      return false;
    }
    updateVarSigns(d);
    if (atomToDed.find(d->getDed()) == atomToDed.end()) {
      addDed(d);
      if (combo == NOOP) unsat = true; \
      return true;
    }
    else {addCombo(d); return true; }
  }


  /*
    Given the last deduction d made in this deduction manager, determine all the deductions that were needed to deduce d.
    This function works by effectively doing a graph traversal with a priority queue through the std::vector of deductions. Since each deduction contains the list of all of its dependencies, we have to check each dependency. If a dependency is a given, then we stop searching there.
  */
  Result DedManager::traceBack(int idx) {
    std::vector<bool> seen(idx+1, false);
    queue<int> dedQ;
    std::vector<TAtomRef> atoms;

    Deduction * d = (deds[idx]);
    if (d->getDeps().size() == 0) {
      TAtomRef t = d->getDed();
      atoms.push_back(t);
      Result r(atoms);
      return r;
    }

    const std::vector<int>& lDeps = depIdxs.back();
    for (std::vector<int>::const_iterator it = lDeps.begin(); it != lDeps.end(); ++it) {
      dedQ.push(*it);
      seen[*it] = true;
    }

    while (!dedQ.empty()) {

      int idx = dedQ.front();
      dedQ.pop();
      seen[idx] = true;
      Deduction * d = (deds[idx]);
      if (d->isGiven()) {
        atoms.push_back(d->getDed());
      }
      else {
        for (int i = 0; i < depIdxs[idx].size(); i++) {
          int x = depIdxs[idx][i];
          if (!seen[x]) dedQ.push(x);
        }
      }
    }
    Result r(atoms);
    return r;
  }

  short DedManager::getSign(IntPolyRef p) {
    FactRef F = new FactObj(PM);
    F->addFactor(p, 1);
    TAtomRef t = new TAtomObj(F, ALOP);
    return getSgn(t);

  }


  int DedManager::searchMap(TAtomRef A) {
    int ret = -1;
    for (map<TAtomRef, int, ManagerComp>::iterator itr = atomToDed.begin(); itr != atomToDed.end(); ++itr) {
      if (isEquiv(A, itr->first)) {
        ret = itr->second;
        break;
      }
    }
    return ret;
  }

  bool DedManager::isEquiv(TAtomRef A, TAtomRef B) {
    if (A->getFactors()->numFactors() != B->getFactors()->numFactors()) return false;
    int t = OCOMP(A->getFactors()->getContent(),B->getFactors()->getContent());
    if (t != 0) { return false; }
    FactObj::factorIterator itrA= A->getFactors()->factorBegin();
    FactObj::factorIterator itrB= B->getFactors()->factorBegin();
    while(itrA != A->getFactors()->factorEnd())
      {
        if (itrA->second != itrB->second) return false;
        if (itrA->first < itrB->first || itrB->first < itrA->first) return false;
        ++itrA;
        ++itrB;
      }
    return true;
  }

  void DedManager::writeProof(int idx) {
    std::vector<bool> seen(idx+1, false);
    priority_queue<int> dedQ;
    std::vector<TAtomRef> atoms;
    vector<Deduction *> dedList;
    Deduction * d = (deds[idx]);
    if (d->getDeps().size() == 0) {
      cout << "Trivial deduction - Given!\n";
      return;
    }
    else {
      cout << "Prove "; d->writeActual(); cout << endl;
    }
    dedList.push_back(d);

    const std::vector<int>& lDeps = depIdxs.back();
    for (std::vector<int>::const_iterator it = lDeps.begin(); it != lDeps.end(); ++it) {
      dedQ.push(*it);
      seen[*it] = true;
    }

    while (!dedQ.empty()) {
      int idx = dedQ.top();
      dedQ.pop();
      seen[idx] = true;
      Deduction * d = (deds[idx]);
      dedList.push_back(d);
      if (d->getDeps().size() == 0) {
        atoms.push_back(d->getDed());
      }
      else {
        for (int i = 0; i < depIdxs[idx].size(); i++) {
          int x = depIdxs[idx][i];
          if (!seen[x]) dedQ.push(x);
        }
      }
    }
    for (int i = dedList.size()-1, j = 0; i >= 0; i--, j++) {
      cout << "(" << j << ") "; dedList[i]->write();
    }
  }





  void DedManager::writeAll() {
    int b = 0;
    for (int i = 0; i < deds.size(); i++) {

      Deduction * d = deds[i];
      cout << "(" << i << ") "; d->write();
    }
  }


  TAndRef DedManager::getSimplifiedFormula() {
    TAndRef t;
    vector<DedScore> vd;
    for (std::vector<Deduction *>::iterator itr = deds.begin();
         itr != deds.end(); ++itr) {
      vd.emplace_back(*itr);
    }
    std::sort(vd.begin(), vd.end(), SimpleComp());
    for (std::vector<DedScore>::iterator itr = vd.begin();
         itr != vd.end(); ++itr) {
      itr->d->getDed()->write(); std::cerr << " ";
    }
    std::cerr << std::endl;
    return t;
  }

  DedManager::~DedManager() {
    for (std::vector<Deduction *>::iterator it = deds.begin(); it != deds.end(); ++it) {
      delete *it;
    }
  }

}
