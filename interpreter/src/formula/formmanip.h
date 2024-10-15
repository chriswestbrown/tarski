/******************************************************
 * Formula manipulation functions.
 ******************************************************/
#ifndef _CWBFORM_MANIP_
#define _CWBFORM_MANIP_

#include "../algparse/algparse.h"
#include "formula.h"
#include "../tarski.h"
#include "../algparse/treemanip.h"
#include <set>

namespace tarski {
class MapToTForm : public Traverse
{
public:
  TFormRef res;
  PolyManager *pM;
  MapToTForm(PolyManager &M) { pM = &M; }
  virtual void operator()(TarskiConst* p);
  virtual void operator()(TarskiAtom* p); 
  virtual void operator()(ExtTarskiAtom* p); 
  virtual void operator()(NotOp* p);
  virtual void operator()(BinaryLogOp* p); 
  virtual void operator()(QuantifierBlock* p);
};

TFormRef parseTreeToTForm(TarskiRef parseTree, PolyManager &M);

IntPolyRef parseTreeToPoly(TarskiRef parseTree, PolyManager &M);

// Returns the VarSet of variables that appear in F in atoms
// that are non-strict.  NOTE: This result is not well-defined
// if the formula is not prenex!
VarSet appearsInNonStrict(TFormRef F);

bool isPrenex(TFormRef F);

  // return the number of disjuncts that would result in naive DNF expansion
  double getDNFNumDisjuncts(TFormRef F);
 
TFormRef getDNF(TFormRef F);

VarSet getFreeVars(TFormRef F);

bool isQuantifiedConjunctionOfAtoms(TFormRef F);

bool isQuantifierFree(TFormRef F);

bool isAndAndAtoms(TFormRef F);

bool isConjunctionOfAtoms(TFormRef F);

/** evalFormulaAtRational(X,R,F)
 * Input: X is a single variable
 *        R is a rational number
 *	  F is a formula
 * Output: formula F' resulting from evaluating F at X <- R
 */
TFormRef evalFormulaAtRational(VarSet X, GCWord R, TFormRef F);

/** evalFormulaAtRationalPoint(value,F)
 * Input: value is a std::map mapping each variable to either NIL or a rational number
 *	  F is a formula
 * Output: formula F' resulting from evaluating F at X <- R
 */
TFormRef evalFormulaAtRationalPoint(VarKeyedMap<GCWord> &value, TFormRef F);

 TFormRef makePrenex(TFormRef F);

 TFormRef exclose(TFormRef T);
 TFormRef exclose(TFormRef T, const vector<string> &holdout);
 
 /** splitNonStrict(T)
  * Input: F a tarski formula that is a conjunction of atomic formulas (not in
  *        in the extended language.
  * Output: formula F' equivalent to F, in which all non-strict inequalities
  *         have been split (e.g. x >= 0 --> x > 0 \/ x = 0).
  */
 TFormRef splitNonStrict(TFormRef F);

 void getFactors(TFormRef F, std::set<IntPolyRef> &W);

 /** nullifysys(p,S,pM)
 * Input: p a polynomials and S a set of vars
 * Output: formula F that is true iff p 
 */
 TFormRef nullifySys(IntPolyRef p, VarSet S, PolyManager* pM);

  
}//end namespace tarski
#endif
