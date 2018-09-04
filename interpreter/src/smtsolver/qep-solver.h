#ifndef QEPSOLVER_H
#define QEPSOLVER_H
#include "box-solver.h"
#include "qepcad-session.h"

namespace tarski {


  /*
    QEPSolver overrides all of BoxSolver's methods in order to
    not use any partial solving capability or NuCAD
   */
  class QEPSolver : public BoxSolver {
  public:
    void getClause(Minisat::vec<Minisat::Lit>& lits, bool& conf)
    { conf = false; } //Overwrite to do nothing

    //overwrite to not use boxsolver and use  MHS code
    void getFinalClause(Minisat::vec<Minisat::Lit>& lits, bool& conf) {
      QepcadConnection q;
      TAndRef tand = BoxSolver::genTAnd(getQhead());
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
      if (unsat) return false;
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

    //Otherwise the exact same
    QEPSolver(tarski::TFormRef formula) : BoxSolver(formula) { }
    ~QEPSolver() {};
  };

}

#endif
