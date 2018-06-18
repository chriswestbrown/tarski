#include "blackbox-solve.h"
#include "bbded.h"

namespace tarski {

  Deduction * BBSolver::deduce(TAndRef t) {
    if (deductions.size() == 0) {
      if (!once)  { once = true; return NULL; }
      else {
	deductions = bbsat(t);
	if (deductions.size() == 0) return NULL;
	else once = false;
      }
    }
    Deduction * d = deductions.back();
    deductions.pop_back();
    return d;
  }


} //end namespace 
