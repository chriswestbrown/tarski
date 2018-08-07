#include "solver-manager.h"

namespace tarski {

  SolverManager::SolverManager(const vector<QuickSolver *>& v, TAndRef tand) : solvers(v), lastDeds(v.size(), 0), hasRan(false) {
    t = new TAndObj();
    for (TAndObj::conjunct_iterator itr = tand->conjuncts.begin();
         itr != tand->conjuncts.end(); ++itr) {
      TAtomRef tf = asa<TAtomObj>(*itr);
      t->AND(tf); 
    }
    dedM = new DedManager(t);
    for (int i = 0; i < solvers.size(); i++) {
      solvers[i]->setDedM(dedM);
    }
  }


  /*
    A useful method for return a Tarski object which contains the results of deduceAll
   */
  LisRef SolverManager::genLisResult() {
      Result r = deduceAll();
      LisRef l = new LisObj();
      if (isUnsat()) l->push_back(new SymObj("UNSAT"));
      else  {
        l->push_back(new SymObj("SAT"));
        dedM->getSimplifiedFormula();
      }
      vector<TAtomRef>& vec = r.atoms;
      TAndRef res = new TAndObj();
      for (vector<TAtomRef>::iterator itr = vec.begin();
           itr != vec.end(); ++itr) {
        res->AND(*itr);
      }
      l->push_back(new TarObj(res));
      return l;
    }


  /*
    The main body of the solver manager
    Calles deduceLoop to actually make the deduction by each solver

    Remembers the last solver to learn something new. Halts when it is that
    solver's turn again but nothing new has been learned

    Also halts immediately when UNSAT is deduced
   */
  Result SolverManager::deduceAll() {
    if (hasRan) return finResult;
    hasRan = true;
    if (dedM->isUnsat()) { finResult = dedM->traceBack(); return finResult; }
    for (int i = 0; i < lastDeds.size(); i++) lastDeds[i] = dedM->size();
    int i = 0, lastChange = -1;
    while (true) {
      //case where solvers can't deduce UNSAT and all deductions exhausted
      if (i == lastChange) {
        Result r; finResult = r; return r;
      }

      if (lastChange == -1) lastChange = 0; //for first iteration
      short res = deduceLoop(i);
      if (res == 1) lastChange = i;
      else if (res == 2) {
        finResult = dedM->traceBack();
        //std::cerr << "size of result is " <<  finResult.count();
        return finResult;
      }
      i++;
      if (i >= solvers.size()) i = 0;
    } 
  }
  
  /*
    Updates the solver by getting iterators from the deduction manager
    from the last new index of the solver to the end of all known deductions
   */
  void SolverManager::updateSolver(int i) {
    vector<Deduction *>::const_iterator itr, end;
    dedM->getItrs(lastDeds[i], itr, end);
    solvers[i]->update(itr, end);
  }

  /*
    
   */
  void SolverManager::updateSolver(const vector<TAtomRef>& v) {

    if (dedM->isUnsat()) { return; }
    hasRan = false;
    int oldLast = dedM->size();
    for (std::vector<TAtomRef>::const_iterator itr = v.begin();
           itr != v.end(); ++itr) {
      dedM->addGiven(*itr);
    }
    for (int i  = 0; i < solvers.size(); i++) {
      updateSolver(i);
    }
  }
  /*
    return 0 to indicate the solver learned nothing
    return 1 to indicate a solver learned something
    return 2 to indicate the solver deduced UNSAT
   */
  short SolverManager::deduceLoop(int i) {
    short retCode = 0;
    QuickSolver * q = solvers[i];
    updateSolver(i);
    Deduction * d = q->deduce(t);
    while (d != NULL && !dedM->isUnsat()) {
      if (dedM->processDeduction(d)) {
        if (dedM->isUnsat()) return 2;
        retCode = 1;
        t->AND(d->getDed());
        q->notify();
      }
      d = q->deduce(t);
    }
    if (dedM->isUnsat()) return 2;
    lastDeds[i] = dedM->size();
    return retCode;
  }

  /*
    Pretty print a result in human readable format
    Generates a proof and a list of all deductions in numbered lists
    Throws an exception if called before deduceAll
   */
  void SolverManager::prettyPrintResult() {
    if (!hasRan) throw TarskiException("DeduceAll not yet called, but prettyPrintResult called!");
    std::cout << "##################################################" << std::endl;
    if (isUnsat()) {
      Result res = finResult;
      std::cout << "UNSAT\n";
      std::cout << "Explanation: "; res.write(); std::cout << std::endl << std::endl;
      std::cout << "Proof: \n";
      dedM->writeProof();
      std::cout << std::endl;
      std::cout << "All Deductions: \n";
      dedM->writeAll();
    }
    else {
      std::cout << "UNKNOWN\n";
      std::cout << std::endl;
      std::cout << "All Deductions: \n";
      dedM->writeAll();
    }
    std::cout << "##################################################" << std::endl;  
  }

  SolverManager::~SolverManager() {
    for (std::vector<QuickSolver * >::iterator itr = solvers.begin();
         itr != solvers.end(); ++itr) {
      delete(*itr);
    }
    solvers.clear();
    delete dedM;
  }
}//end namespace tarski
