#ifndef QEPSOLVER_H
#define QEPSOLVER_H
#include "box-solver.h"

namespace Minisat {
  class QEPSolver : public BoxSolver {
  public:
    void getClause(vec<Lit>& lits, bool& conf) {conf = false;} //Overwrite to do nothing
    void getFinalClause(vec<Lit>& lits, bool& conf) {BoxSolver::getFinalClause(lits, conf);}
    void getAddition(vec<Lit>& lits, bool& conf) {conf = false;}
    QEPSolver(tarski::TFormRef formula)
    : BoxSolver(formula) {} //Otherwise the exact same
    ~QEPSolver() {};
  };

}

#endif
