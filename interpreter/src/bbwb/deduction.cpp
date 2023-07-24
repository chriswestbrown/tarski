#include "qfr.h" //-- this is only to get 'verbose'!
#include "deduction.h"
#include <unordered_set>
#include <algorithm>
//TODO:Refactor writeproof and traceBack!

namespace tarski {

  
#define C_ITR const_iterator
#define ITR iterator

  const string Deduction::names[8] = {"given",  "combo", "bbstrict",
                                       "minwt", "bbcombo", "polysign",
                                      "deducesign", "substitution"};

  const int Deduction::GIVEN = 0;
  const int Deduction::COMBO = 1;
  const int Deduction::BBSTR = 2;
  const int Deduction::MINWT = 3;
  const int Deduction::BBCOM = 4;
  const int Deduction::POLYS = 5;
  const int Deduction::DEDUC = 6;
  const int Deduction::SUBST = 7;

  void Deduction::write() const {
    if (unsat) cerr << "UNSAT";
    else {
      cerr << *name <<  ": ";
      if (!unsat) deduction->write();
    }
    cout << endl;
  }

  string DedExp::toString() {
    string s;
    s += d.getName() + " = "
      + (d.getDed().is_null() ? "NULL" : tarski::toString(d.getDed()))
      + "  : ";
    for (forward_list<TAtomRef>::iterator itr = exp.begin();
         itr != exp.end(); ++itr) {
      s += tarski::toString(*itr) + ", ";
    }
    s.resize(s.size()-2);
    return s;
  }

  DedManager::DedManager(TAndRef a) : unsat(false), varSigns(ALOP) {
    PM = a->getPolyManagerPtr();
    for (TAndObj::conjunct_iterator itr = a->begin(), end = a->end(); itr != end; ++itr) {
      processGiven(*itr);
      if (unsat)  { return; }
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

  //Gets the simplified Conjunct,
  //Where simplified means that if two atoms
  //x >= 0 and x <= 0 appear in the original formula,
  //the simplified formula has x = 0
  TAndRef DedManager::getInitConjunct() {
    TAndRef tand = new TAndObj();
    for (int i = 0; i < deds.size(); i++) {
      if (!isGiven[i]) continue;
      else {
        tand->AND(deds[i].getDed());
      }
    }
    return tand;
  }

  bool DedManager::ManagerComp::operator()(const TAtomRef& A, const TAtomRef& B) const {
    return A->F->cmp(B->F) < 0;
  }

  short DedManager::getSgn(TAtomRef t) {
    return (atomToDed.find(t) == atomToDed.end())
      ? ALOP : deds[atomToDed[t]].getDed()->relop;
  }

  void DedManager::updateVarSigns(TAtomRef t) {
    if (t->F->numFactors() == 1 && t->factorsBegin()->first->isVar() && t->getRelop() != ALOP && t->factorsBegin()->second == 1) {
      VarSet v = t->getVars();
      varSigns[v] = varSigns[v] & t->getRelop();
      assert(varSigns[v] != NOOP || unsat);
    }
  }


  void DedManager::addGiven(TAtomRef t) {
    depIdxs.emplace_back();
    origDep.emplace_back();
    atomToDed[t] = deds.size();
    Deduction d(t, Deduction::GIVEN);
    deds.push_back(d);
    isGiven.push_back(true);
  }

  /*
    Combines two givens, yielding two new deductions - the one that is given,
    plus the combination
   */
  void DedManager::addGCombo(TAtomRef t) {
    depIdxs.emplace_back();
    origDep.emplace_back();

    deds.emplace_back(t, Deduction::GIVEN);
    isGiven.push_back(true);
    int oldIdx = atomToDed[t];
    TAtomRef t1 = deds[oldIdx].getDed();
    vector<TAtomRef> atomDeps(2);
    atomDeps[0] = t;
    atomDeps[1] = t1;


    //Add a cycle from the two originals to the sign combo
    //that we will make
    depIdxs.back().push_back ({ (int) depIdxs.size() });
    depIdxs[oldIdx].push_back({ (int) depIdxs.size() });


    //Adds the old deduction and the new deduction as dependencies
    //Makes a new atom which represents the sign combination of the two
    set<int> deps;
    deps.insert(deds.size()-1);
    deps.insert(oldIdx);
    depIdxs.emplace_back();
    depIdxs.back().push_back(deps);
    origDep.push_back(deps);


    TAtomRef t2 = new TAtomObj(t->F, t->relop & getSgn(t));
    atomToDed[t] = deds.size();
    if (t2->relop == NOOP) unsat = true;
    deds.emplace_back(t2, Deduction::COMBO);
    isGiven.push_back(false);
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

  /*
    Gets the set of indices which correspond to the explanation of a deduction
   */
  std::set<int> DedManager::getDepIdxs(const dedList& d) {
    set<int> deps;
    for (dedList::C_ITR itr = d.begin(), end = d.end(); itr != end; ++itr) {
      int idx = -1;
      if ((*itr)->getRelop() == ALOP) continue;
      if (atomToDed.find(*itr) == atomToDed.end()) {
        idx = searchMap(*itr);
        if (idx == -1) {
          throw TarskiException("No index for dependency " + \
                                toString(*itr) +  "!");
        }
      } 
      if (idx == -1) idx = (atomToDed.find(*itr))->second;
      if (idx == -1) throw TarskiException("Unknown Dependency!");
      deps.insert(idx);
    }
    return deps;
  }


  /*
    Process a deduction which adds no new information, but does allow us
    to "re-learn" some information
    Allows for potential substitutions
   */
  void DedManager::addCycle(const Deduction& d, const dedList& dl) {
    set<int> idxs = getDepIdxs(dl);
    //We want to reject self deductions ie x = 0 is used to deduce x = 0
    if (idxs.empty() || idxs.find(atomToDed[d.getDed()]) != idxs.end()){
      return;
    }
    DedExp e(d, dl);
    int idx = atomToDed[d.getDed()];
    checkAdd(idxs, idx);
  }

  /*
    Assumes that d is stronger than the previous deduction on the atom,
    if one exists
   */
  void DedManager::addDed(const Deduction& d, const dedList& dl){
    if (searchMap(d.getDed()) != -1)
      depIdxs[searchMap(d.getDed())].push_back( { (int) deds.size()} );
    depIdxs.emplace_back();
    depIdxs.back().push_back(getDepIdxs(dl));
    origDep.push_back(getDepIdxs(dl));
    atomToDed[d.getDed()] = deds.size();
    deds.push_back(d);
    isGiven.push_back(false);

  }


  /*
    Called when we find a deduction which teaches us unique sign information
    which when combined with knonw information, yields an even stronger
    deduction.
    Adds two deductions - the one we made, and then the combination
   */
  void DedManager::addCombo(const Deduction& d, const dedList& dl) {

    int earlierIdx = atomToDed[d.getDed()];
    TAtomRef earlier = deds[earlierIdx].getDed();
    int comboSign = d.deduction->relop & earlier->relop;
    //Adding d
    depIdxs.emplace_back();
    depIdxs.back().push_back(getDepIdxs(dl));
    origDep.push_back(getDepIdxs(dl));
    deds.push_back(d);
    isGiven.push_back(false);

    //Add a dependency on d and the previous version of d
    //to the sign combination
    depIdxs[earlierIdx].push_back( {(int) deds.size()} );
    depIdxs.back().push_back( {(int) deds.size()});

    //Adding the combo itself
    if (comboSign == NOOP) unsat = true;
    TAtomRef t = new TAtomObj(d.getDed()->getFactors(), comboSign);
    Deduction nuDed(t, Deduction::COMBO);
    atomToDed[t] = deds.size();
    deds.emplace_back(nuDed);
    updateVarSigns(nuDed);
    isGiven.push_back(false);
    origDep.push_back({earlierIdx, (int) deds.size()-2});
    depIdxs.emplace_back();
    depIdxs.back().push_back({earlierIdx, (int) deds.size()-2});

  }

  /*
    Returns true and adds a new deduction to the list of deduction, if the deduction learns anything new. Set unsat to true if the new deduction contradicts with known information
    If it doesn't, then don't add the deduction and terminate prematurely. return false
    do nothing if formula is already unsat
  */
  bool DedManager::processDeduction(const Deduction& d, const dedList& dl) {
    //Initial processing of the deduction, check if it teaches anything useful
    //If it teaches us nothing useful, return
    if (d.isUnsat() || d.getDed()->getRelop() == NOOP) {
      depIdxs.emplace_back();
      depIdxs.back().push_back(getDepIdxs(dl));
      origDep.push_back(getDepIdxs(dl));
      deds.push_back(d);
      isGiven.push_back(false);
      unsat = true;
      return true;
    }

    if (atomToDed.find(d.getDed()) == atomToDed.end()) {

      updateVarSigns(d);
      addDed(d, dl);
      return true;
    }

    else if (d.getDed()->getRelop() == getSgn(d.getDed()))  {
      addCycle(d, dl);
      return false;
    }
    short combo = d.getDed()->getRelop() & getSgn(d.getDed());
    //the new ded is weaker
    if (combo == getSgn(d.getDed())) {
      return false;
    }
    //the new ded is stronger
    else if (combo == d.getDed()->getRelop()) {
      updateVarSigns(d);
      addDed(d, dl);
    }
    //the new ded and the old ded combined is stronger
    else {
      addCombo(d, dl);
      if (combo == NOOP) unsat = true;
    }
    return true;
  }

  /*
    Checks if this set is subsumed by a set in depIdxs
    Or if it subsumes another set

    If it is subsumed, then reject the add and return
    If it subsumes, replace the set which is subsumed with this set
    If neither, add the set

    This code assumes there will be at most 2 or 3 rules for each deduction
   */
  void DedManager::checkAdd(std::set<int>& idxs, int pos) {
    assert(pos <= depIdxs.size());
    for (list<set<int>>::iterator itr = depIdxs[pos].begin();
         itr != depIdxs[pos].end(); ++itr) {
      if (includes(idxs.begin(), idxs.end(), itr->begin(), itr->end())) {
        return;
      }
      else if (includes(itr->begin(), itr->end(), idxs.begin(), idxs.end())) {
        depIdxs[pos].erase(itr);
        depIdxs[pos].push_back(idxs);
        break;
      }
    }
    depIdxs[pos].push_back(idxs);


  }

  /*
    Given the last deduction d made in this deduction manager, determine all the deductions 
    that were needed to deduce d. This function works by effectively doing a graph traversal 
    with a priority queue through the std::vector of deductions. Since each deduction contains 
    the list of all of its dependencies, we have to check each dependency. If a dependency is 
    a given, then we stop searching there.
  */
  Result DedManager::traceBack(int idx) {
    std::vector<bool> seen(idx+1, false);
    queue<int> dedQ;
    std::vector<TAtomRef> atoms;

    Deduction& d = (deds[idx]);
    if (depIdxs[idx].size() == 0)
    {
      TAtomRef t = d.getDed();
      atoms.push_back(t);
      Result r(atoms);
      return r;
    }

    const std::set<int>& lDeps = *(depIdxs[idx].begin());
    for (std::set<int>::const_iterator it = lDeps.begin(); it != lDeps.end(); ++it) {
      dedQ.push(*it);
      seen[*it] = true;
    }

    while (!dedQ.empty())
    {
      int idx = dedQ.front();
      dedQ.pop();
      Deduction& d = (deds[idx]);
      if (isGiven[idx])
      {
        atoms.push_back(d.getDed());
      }
      else
      {
        for (set<int>::iterator itr = origDep[idx].begin(); itr != origDep[idx].end(); ++itr)
	{
          if (!seen[*itr]) { dedQ.push(*itr); seen[*itr] = true; }
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
    vector<Deduction> dedList;
    Deduction& d = (deds[idx]);
    if (depIdxs[idx].size() == 0) {
      cout << "Trivial deduction - Given!\n";
      return;
    }
    else {
      cout << "Prove "; d.writeActual(); cout << endl;
    }
    dedList.push_back(d);

    const std::set<int>& lDeps = *(depIdxs.back().begin());
    for (std::set<int>::const_iterator it = lDeps.begin();
         it != lDeps.end(); ++it) {
      dedQ.push(*it);
      seen[*it] = true;
    }

    while (!dedQ.empty()) {
      int idx = dedQ.top();
      dedQ.pop();
      seen[idx] = true;
      Deduction& d = (deds[idx]);
      dedList.push_back(d);
      if (isGiven[idx]) {
        atoms.push_back(d.getDed());
      }
      else {
        for (set<int>::iterator itr = origDep[idx].begin();
             itr != origDep[idx].end(); ++itr) {
          if (!seen[*itr]) dedQ.push(*itr);
        }
      }
    }
    for (int i = dedList.size()-1, j = 0; i >= 0; i--, j++) {
      cout << "(" << j << ") "; writeDedExplain(idx);
    }
  }
  
  void DedManager::writeDedExplain(int idx) {
    cout << deds[idx].toString();
    if (deds[idx].getName() != "given") {
      cout << ": ";
      for (auto itr = origDep[idx].begin(); itr != origDep[idx].end(); ++itr) {
        if (itr != origDep[idx].begin()) cout << ", ";
        cout << toString(deds[*itr].getDed());
      }
    }
    cout << endl;
  }
  void DedManager::writeAll() {
    for (size_t i = 0; i < deds.size(); i++) {
      writeDedExplain(i);
    }
  }

  //returns an empty list if t cannot be found
  //else returns an explanation for some atom
  //deduced by the dedM
  Result DedManager::explainAtom(TAtomRef t) {
    int idx = searchMap(t);
    if (idx != -1) {
      if (isGiven[idx]) {
        //cout << "t has no explanation as it is given! " + toString(t) << endl;
        Result r( {t} );
        return r;
      }
      //cout << "t has an explanation - not given! " + toString(t) << endl;
      return traceBack(idx);
    }
    Result r;
    return r;
  }


  //SIMPLIFICATION CODE ------------------------------------------------------

  void DedManager::debugWriteSorted(Orderer &ord)
  {
    vector<size_t> indices = ord.proxy_sorted_indices(deds);
    
    cout << endl << "Sorted deductions:" << endl;
    for(size_t i = 0; i < indices.size(); ++i)
    {
      int k = indices[i];
      cout << k << ": ";
      auto L = depIdxs[k];
      for(auto litr = L.begin(); litr != L.end(); ++litr)
      {
	cout << "[ ";
	for(auto sitr = litr->begin(); sitr != litr->end(); ++sitr)
	  cout << " " << *sitr;
	cout << " ] ";
      }      
      cout << deds[k].toString() << endl;
    }
    cout << endl;
  }


  // void DedManager::temporaryDebugInfo(Orderer& ord)
  // {
  //   vector<size_t> indices = ord.proxy_sorted_indices(deds);
    
  //   // Dr Brown debug
  //   if (true)
  //   {
  //     cout << endl << "Sorted deductions:" << endl;
  //     for(size_t i = 0; i < indices.size(); ++i)
  //     {
  // 	int k = indices[i];
  // 	cout << k << ": ";
  // 	auto L = depIdxs[k];
  // 	for(auto litr = L.begin(); litr != L.end(); ++litr)
  // 	{
  // 	  cout << "[ ";
  // 	  for(auto sitr = litr->begin(); sitr != litr->end(); ++sitr)
  // 	    cout << " " << *sitr;
  // 	  cout << " ] ";
  // 	}      
  // 	cout << deds[k].toString() << endl;
  //     }
  //     cout << endl;
  //   }
  // } 

  
  TAndRef DedManager::getSimplifiedFormula(Orderer& ord)
  {
    vector<size_t> indices = ord.proxy_sorted_indices(deds);
    
    // Dr Brown debug
    if (verbose)
    {
      cout << endl << "Sorted deductions:" << endl;
      for(size_t i = 0; i < indices.size(); ++i)
      {
	int k = indices[i];
	cout << k << ": ";
	auto L = depIdxs[k];
	for(auto litr = L.begin(); litr != L.end(); ++litr)
	{
	  cout << "[ ";
	  for(auto sitr = litr->begin(); sitr != litr->end(); ++sitr)
	    cout << " " << *sitr;
	  cout << " ] ";
	}      
	cout << deds[k].toString() << endl;
      }
      cout << endl;
    }
    
    vector<size_t> revIndices(indices.size());
    for (size_t i = 0; i < revIndices.size(); i++) {
      revIndices[indices[i]] = i; 
    }
    //writeIntermediate(indices, revIndices);
    TAndRef t = new TAndObj();
    set<int> skips;
    listVec asSat = genSatProblem(t, skips, indices);

    //writeSatProblem(asSat);
    solveSAT(asSat, t, skips, indices);
    return t;
  }


  /*
    Solves the sat problems which represent our simplifications, from
    "ugliest" deduction to "prettiest" deduction
    Everytime a problem is determined to be UNSAT, we remove the relevant
    clauses from our problem set, that deduction having been simplified out
   */
  void DedManager::solveSAT(listVec& asSat, TAndRef& t, set<int>& skips, vector<size_t>& indices) {
    vector<char> elim(deds.size(), false);
    int numElim = 0;

    for (int i = indices.size()-1; i >= 0; i--) {

      int toRemove = indices[i];
      if (skips.find(toRemove) != skips.end()) continue;
      set<int> appears;
      for (auto itr = asSat.begin(); itr != asSat.end(); ++itr) {
        for (int i = 0; i < itr->size(); i++) {
          appears.insert(var((*itr)[i])-1);
        }
      }
      Minisat::vec<Minisat::Lit> s;
      int j = 0;
      for (size_t i = 0; i < deds.size(); i++) {
        if (appears.find(i) == appears.end()) continue;
        if (elim[i])  continue;
        else if (i == toRemove) s.push(Minisat::mkLit(i+1, true));
        else s.push(Minisat::mkLit(i+1));
        j++;
      }
      Minisat::Solver S;

      // DEBUG
      if (verbose)
      {
	cout << "\nSAT CHECK: " << toString(deds[toRemove].getDed()) << endl;
	writeSatProblem(asSat);
	cout << "S: ";
	for (int i = 0; i < s.size(); i++) {
	  write(s[i]); cout << " ";
	}
	cout << endl;
      }
	
      S.mkProblem(asSat.begin(), asSat.end());
      Minisat::lbool ret = S.solveLimited(s);
      if (ret == Minisat::toLbool(1)) { //-- This means "toRemove" is actually necessary
        if (verbose) { cout << "\nSAT\n"; }
        for (size_t i = 0; i < depIdxs[toRemove].size(); i++)
	  ;//DRBROWN asSat.pop_front();
        elim[toRemove] = true;
        numElim++;
      }
      else {
        if (verbose) { cout << "\nUNSAT\n"; }
        for (size_t i = 0; i < depIdxs[toRemove].size(); i++) {
          //a stupid way to get around minisats restriction on
          //copy constructors and =
          asSat.emplace_back(asSat.front().size());
          for (int i = 0; i < asSat.front().size(); i++) {
            asSat.back()[i] = asSat.front()[i];
          }
          asSat.pop_front();
          if (!deds[toRemove].getDed()->getFactors()->isConstant())
            t->AND(deds[toRemove].getDed());
          simpIdx.push_back(toRemove);
        }
      }
    }
  }

  void DedManager::writeIntermediate(vector<size_t>& indices, vector<size_t>& rev) {
    cout << "ORIGINAL:\n";
    for (size_t i = 0; i < deds.size(); i++) {
      std::cout << i << ": ";
      deds[i].getDed()->write();
      if (depIdxs[i].size() == 0) {
        std::cout << " has no dependencies\n";
        continue;
      }
      std::cout << " depends on ";
      for (list<set<int>>::iterator itr = depIdxs[i].begin();
           itr != depIdxs[i].end(); ++itr) {
        for (std::set<int>::iterator vItr = itr->begin();
             vItr != itr->end(); ++vItr) {
          if (vItr != itr->begin()) std::cout << ",";
          std::cout << *vItr;
        }
        std:: cout << "/";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;


    cout << "SORTED:\n";
    for (size_t i = 0; i < deds.size(); i++) {
      std::cout << i << ": ";
      deds[indices[i]].getDed()->write();
      if (depIdxs[indices[i]].size() == 0)  {
        std::cout << " has no dependencies\n";
        continue;
      }
      std::cout << " depends on ";
      for (list<set<int>>::iterator itr =
             depIdxs[indices[i]].begin();
           itr != depIdxs[indices[i]].end(); ++itr) {
        for (std::set<int>::const_iterator vItr = itr->begin();
             vItr != itr->end(); ++vItr) {
          if (vItr != itr->begin()) std::cout << ",";
          std::cout << rev[*vItr];
        }
        std::cout << "/";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }


  /*
    Generates the sat problem which represents our choice of simplifications
    The problem is constructed s.t. if a deduction can be simplified out,
    the relevant SAT problem (which is constructed by setting that deduction's
    index to false) solves to false.
   */
  listVec DedManager::genSatProblem(TAndRef& t, set<int>& skips,
                                                vector<size_t>& order) {
    listVec lv;
    for (size_t i = 0; i < order.size(); i++) {
      int idx = order[i];
      if (depIdxs[idx].size() == 0) {
        t->AND(deds[idx].getDed());
        lv.emplace_front(1);
        lv.front()[0] = Minisat::mkLit(idx+1);
        skips.insert(idx);
        simpIdx.push_back(idx);
        continue;
      }
      //the CNF of a or b implies c
      //is (~a or c) /\ (~b or c)
      //and if a ded atom has multiple soures, we can write it
      //as as rule1 or rule2 or... rulex implies c
      //each rule is the negation of all deps
      //plus the atom implied, set to true
      for (list<set<int>>::iterator itr = depIdxs[idx].begin();
           itr != depIdxs[idx].end(); ++itr) {
        int s = itr->size()+1;
        lv.emplace_front(s);
        int j = 0;
        for (set<int>::iterator sItr = itr->begin();
             sItr != itr->end(); ++sItr) {
          Minisat::Lit p = Minisat::mkLit(*sItr+1, true);
          lv.front()[j] = p;
          j++;
        }
        lv.front()[j] = Minisat::mkLit(idx+1);
      }
    }
    return lv;
  }




  void DedManager::writeSatProblem(listVec& lv) {
    cout << "SAT PROBLEM:\n";
    for (auto itr = lv.begin(); itr != lv.end(); ++itr) {
      Minisat::vec<Minisat::Lit>& vv = *itr;
      for (int i = 0; i < vv.size(); i++) {
        write(vv[i]); cout << " ";
      }
      cout << endl;
    }
  }


  DedManager::~DedManager() {}

}
