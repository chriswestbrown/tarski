#ifndef QEPSOLVER_H
#define QEPSOLVER_H
#include "box-solver.h"

namespace tarski {
  class QEPSolver : public BoxSolver {
  public:
    void getClause(Minisat::vec<Minisat::Lit>& lits, bool& conf) {conf = false;} //Overwrite to do nothing
    void getFinalClause(Minisat::vec<Minisat::Lit>& lits, bool& conf) {BoxSolver::getFinalClause(lits, conf);}
    void getAddition(Minisat::vec<Minisat::Lit>& lits, bool& conf) {conf = false;}
    QEPSolver(tarski::TFormRef formula)
    : BoxSolver(formula) {} //Otherwise the exact same
    ~QEPSolver() {};
  };

}

#endif
