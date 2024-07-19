#ifndef _WRITE_FOR_QE_
#define _WRITE_FOR_QE_

#include "formula.h"
#include "../onecell/varorder.h"
#include <vector>

namespace tarski {
  
// Factors a conjunction into chunks that are disjoint
// with respect to the given std::set of variables.
// Returns std::vector<TFormRef> res with the property that
// 1) each element is an atom or conjunction of atoms
// 2) the conjunction of all elements of res is exactly F
// 3) if i != j, then res[i] and res[j] do not share any variable in S
// The upshot of this is that if Q is a subset of S,
// ex Q [ F ] <==> ex Q[res[0]] /\ ... /\ ex Q[res[k-1]] and
// all Q [ F ] <==> all Q[res[0]] /\ ... /\ all Q[res[k-1]] and
std::vector<TFormRef> factorDisjointVars(TAndRef F, VarSet S);

// Returns a std::set of references to atoms in F that contain
// the unique occurence of an element of S. Requires an AND!!!!
std::map<TFormRef,VarSet> atomsContainingUniqueVar(TAndRef F, VarSet S);


// Returns a std::vector with the suggested variable order for CAD
// construction following the Brown heuristic.  Note that this
// requires a prenex input formula. (throws exception if not)
std::vector<VarSet> getBrownVariableOrder(TFormRef F);

// takes formula F and returns std::set of all factors appearing in F
std::set<IntPolyRef> collectFactors(TFormRef F);

// well ... the name says it all!
std::string naiveButCompleteWriteForQepcad(TFormRef F,
					   TFormRef &introducedAssumptions,
					   bool endWithQuit = true,
					   bool trackUnsatCore = false,
					   char solFormType = 'T',
					   VarOrderRef ord = NULL
					   );


std::string writeForQEPCADB(TFormRef F,
			    TFormRef &introducedAssumptions,
			    bool endWithQuit = true,
			    bool trackUnsatCore = false,
			    char solFormType = 'T',
			    VarOrderRef ord = NULL
			    );

// F : a prenex formula, ord : a preferred variable order
// makeOrderProper(F,ord) reorders ord (if needed) to make
// it proper with respect to any _root_ expressions, while
// respecting quantifier block structure.  Throws a TarskiException
// if this is not proper without replacing _root_ expressions.
// NOTE: if F has no _root_ expressions this does nothing, so in
//       that case it's up to the caller to make sure ord respects
//       the quantifier block structure!
void makeOrderProper(TFormRef F, std::vector<VarSet> &ord);
  
// Takes formula F and variables x and y appearing in F and returns
// a "feature vector" describing how the x and y compare w.r.t. F.
 std::vector<float> getFeatures2Vars(TFormRef F, VarSet x, VarSet y);
  

 
}//end namespace tarski

#endif
