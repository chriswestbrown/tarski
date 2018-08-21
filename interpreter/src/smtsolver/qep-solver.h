#ifndef QEPSOLVER_H
#define QEPSOLVER_H
#include "box-solver.h"
#include "qepcad-session.h"

namespace tarski {
  class QEPSolver : public BoxSolver {
  public:
    void getClause(Minisat::vec<Minisat::Lit>& lits, bool& conf)
    { conf = false; } //Overwrite to do nothing

    //overwrite to not use boxsolver and use  MHS code
    void getFinalClause(Minisat::vec<Minisat::Lit>& lits, bool& conf) {
      QepcadConnection q;
      TAndRef tand = (M != NULL) ? genMHS() : BoxSolver::genTAnd(getQhead());
      TFormRef res;
      try {
        res = q.basicQepcadCall(exclose(tand), true);
      }
      catch (TarskiException& e) {
        throw TarskiException("QEPCAD timed out");
      }
      if (res->constValue() == 0) {
        conf = true;
        vector<int> core = q.getUnsatCore();
        sort(core.begin(), core.end());
        BoxSolver::getQEPUnsatCore(lits, core, tand);
      }
      else conf = false;
    }


    //override to do nothing
    void getAddition(Minisat::vec<Minisat::Lit>& lits, bool& conf)
    {conf = false;}

    //override not to use partial solver
    bool directSolve() {
      TFormRef res;
      try  {
        QepcadConnection q;
        res = q.basicQepcadCall(exclose(formula), true);
      }
      catch (TarskiException& e) {
        throw TarskiException("QEPCAD timed out");

      }
      if (res->constValue() == 0) return false;
      else return true;
    }


    QEPSolver(tarski::TFormRef formula) {
      this->formula = formula;
      IM = new IdxManager();
      pm = formula->getPolyManagerPtr();
      processAtoms(this->formula);
      if (!isPureConj){
        S = new Minisat::Solver(this);
        S->mkProblem(makeFormula(this->formula));
        M = (numAtoms > 5) ? new MHSGenerator(form, numAtoms) : NULL;
      }
    } //Otherwise the exact same
    ~QEPSolver() {};
  };

}

#endif
