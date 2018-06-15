#include "solver-manager.h"

namespace tarski {

  SolverManager::SolverManager(vector<QuickSolver *>& v, TAndRef tand) : lastChange(-1), dedM(tand), hasRan(false) {
      solvers = v;
      t = tand;
      for (int i = 0; i < solvers.size(); i++) {
	solvers[i]->setDedM(&dedM);
      }
    }
  
  Result SolverManager::deduceAll() {
    hasRan = true;
    if (dedM.isUnsat()) return dedM.traceBack();
    for (int i = 0; i < solvers.size(); i++) {
      if (i == lastChange) {
	Result r; finResult = r; return r;
      }
      short res = deduceLoop(solvers[i]);
      if (res == 1) lastChange = i;
      else if (res == 2) { finResult = dedM.traceBack(); return finResult; }
    }
    if (dedM.isUnsat()) {  finResult = dedM.traceBack(); return finResult; }
    else { Result r; finResult = r; return r;}
  }

  short SolverManager::deduceLoop(QuickSolver * q) {
    Deduction * d = q->deduce(t);
    short retCode = 0;
    while (d != NULL && !dedM.isUnsat()) {
      if (dedM.processDeduction(d)) {
	if (dedM.isUnsat()) return 2;
	retCode = 1;
	t->AND(d->getDed());
	q->notify();
      }
      d = q->deduce(t);
    }
    if (dedM.isUnsat()) return 2;
    return retCode;
  }

  void SolverManager::prettyPrintResult() {
    if (!hasRan) throw TarskiException("DeduceAll not yet called, but prettyPrintResult called!");
    std::cout << "##################################################" << std::endl;
    if (isUnsat()) {
      Result res = finResult;
      std::cout << "UNSAT\n";
      std::cout << "Explanation: "; res.write(); std::cout << std::endl << std::endl;
      std::cout << "Proof: \n";
      dedM.writeProof();
      std::cout << std::endl;
      std::cout << "All Deductions: \n";
      dedM.writeAll();
    }
    else {
      std::cout << "UNKNOWN\n";
      std::cout << std::endl;
      std::cout << "All Deductions: \n";
      dedM.writeAll();
    }
    std::cout << "##################################################" << std::endl;  
  }
}//end namespace tarski
