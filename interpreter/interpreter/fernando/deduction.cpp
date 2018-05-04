#include "deduction.h"
//TODO: Add a sign table for atoms


namespace tarski {


#define C_ITR const_iterator
#define ITR iterator

  //NOTE: Does not yet process "givens" from intpolyrefs embedded in multi factor atoms
  //Will this work? Let me test it and find out!
  DedManager::DedManager(TAndRef a) : unsat(false), varSigns(ALOP) {
    PM = a->getPolyManagerPtr();
    for (TAndObj::conjunct_iterator itr = a->begin(), end = a->end(); itr != end; ++itr) {
      processGiven(*itr);
      if (unsat) return;
    }

  }

  //This requires at least one member of a
  DedManager::DedManager(const vector<TAtomRef>& a) : unsat(false), varSigns(ALOP) {
    for (vector<TAtomRef>::C_ITR itr = a.begin(), end = a.end(); itr != end; ++itr) {
      processGiven(*itr);
      if (unsat) return;
    }
    PM = a[0]->getPolyManagerPtr();
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


    TAtomRef t1 = deds.back()->getDed();
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
    (getSgn(t) == ALOP) ? addGiven(t) : addGCombo(t);
    updateVarSigns(t);
  }

  vector<int> DedManager::getDepIdxs(Deduction * d) {
    vector<int> deps;
    const vector<TAtomRef>& dA = d->getDeps();
    for (vector<TAtomRef>::C_ITR itr = dA.begin(), end = dA.end(); itr != end; ++itr) {
      if ((*itr)->relop == ALOP) continue;
      if (atomToDed.find(*itr) == atomToDed.end()) {
        cerr << "Deduction: "; d->write();
        cerr << "Dependency: "; (*itr)->write(); cerr << endl;
        throw TarskiException("No index for dependency!");
      } 
      int idx = (atomToDed.find(*itr))->second;
      if (idx == -1) throw TarskiException("Unknown Dependency! Line 66 deduction.cpp");
      deps.push_back(idx);
    }
    return deps;
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
    //d->write();
    //cerr << endl;
    if (d->isUnsat()) {
      depIdxs.push_back(getDepIdxs(d));
      deds.push_back(d);
      unsat = true;
      return true;
    }
    updateVarSigns(d);
    short combo = d->getDed()->getRelop() & getSgn(d->getDed());
    if (combo == getSgn(d->getDed()))  {delete d;  return false; }
    else if (atomToDed.find(d->getDed()) == atomToDed.end()) { addDed(d); if (combo == NOOP) unsat = true; return true;}
    else {addCombo(d); return true; }
  }


  /*
    Given the last deduction d made in this deduction manager, determine all the deductions that were needed to deduce d.
    This function works by effectively doing a graph traversal with a priority queue through the std::vector of deductions. Since each deduction contains the list of all of its dependencies, we have to check each dependency. If a dependency is a given, then we stop searching there.
  */
  Result DedManager::traceBack() {
    return traceBack(deds.size()-1);
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



  void DedManager::writeProof() {
    writeProof(deds.size()-1);
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


  DedManager::~DedManager() {
    for (std::vector<Deduction *>::iterator it = deds.begin(); it != deds.end(); ++it) {
      delete *it;
    }
  }

}
