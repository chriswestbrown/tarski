#include "qep-solver.h"
#include "../shell/qepcad-inter/qepcad-session.h"
#include "box-solver.h"
#include "boxer.h"
#include "idx-manager.h"

namespace Minisat {
  using namespace tarski;
  bool QEPSolver::directSolve() {
    TAndRef t = asa<TAndObj>(formula);
    if (t->constValue() == true) return true;
    TFormRef res;
    try  {
      QepcadConnection q;
      res = q.basicQepcadCall(exclose(t), true);
    }
    catch (TarskiException& e) {
      throw TarskiException("QEPCAD timed out");
    }
    if (res->constValue() == 0) return false;
    else return true;
  }
}
