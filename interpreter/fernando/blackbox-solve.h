#include "deduciton.h"
#include "solver-manager.h"

namespace tarski{

  class BBSolver : QuickSolver {
  private:

  public:
    BBSolver(TAndRef tf) {};
    Deduction * deduce(TAndRef t);
    void notify() {}
  }
