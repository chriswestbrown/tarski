#ifndef _LINEAR_SUBS_
#define _LINEAR_SUBS_

#include "formula.h"

namespace tarski {

//        M - the PolyManager for the problem
//   target - Formula I'm subbing into
//        A - polynomial (best to make sure it's the cannonical ref!)
//        x - a variable, must be variable in which A is linear
// sourceEQ - NULL, or a Atom.  I this atom appears in target, it is removed.
// Substituting the degenerate case of A = 0 into target, i.e. when ldcf and trcf
// of A as a polynomial in x are both zero.
TFormRef linearSubstDegenerateCase(PolyManager &M, 
				   TFormRef target, 
				   IntPolyRef A, 
				   VarSet x, 
				   TAtomRef sourceEQ = NULL);

//        M - the PolyManager for the problem
//   target - Formula I'm subbing into
//        A - polynomial (best to make sure it's the cannonical ref!)
//        x - a variable, must be variable in which A is linear
//        E - a reference to the atomic formula that generates this
//            substitution, i.e. A = 0.
//   leaveE - if true, atomic formula E will be left in the result formula
//            untouched, otherwise it will be removed (i.e. replaced with 
//            true.  Essentially, leaveE is true when x is a free variable,
//            and false when x is quantified.
TFormRef linearSubstGenericCase(PolyManager &M, 
				TFormRef target, 
				IntPolyRef A, 
				VarSet x,
				TAtomRef E, 
				bool leaveE );
}//end namespace tarski
#endif
