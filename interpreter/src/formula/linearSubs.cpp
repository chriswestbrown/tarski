#include "linearSubs.h"
#include "formmanip.h"

namespace tarski {
//        M - the PolyManager for the problem
//   target - Formula I'm subbing into
//        A - polynomial (best to make sure it's the cannonical ref!)
//        x - a variable, must be variable in which A is linear
// sourceEQ - NULL, or a Atom.  I this atom appears in target, it is removed.
// Substituting the degenerate case of A = 0 into target, i.e. when ldcf and trcf
// of A as a polynomial in x are both zero.
// NOTE: this actually provides the degenerate case for any A of the form c1 x^k + c2
TFormRef linearSubstDegenerateCase(PolyManager &M, 
				   TFormRef target, 
				   IntPolyRef A, 
				   VarSet x, 
				   TAtomRef sourceEQ)
{
  IntPolyRef c1 = M.ldcf(A,x);
  IntPolyRef c0 = M.trcf(A,x);
  if (c1->isConstant() || (c0->isConstant() && ! c0->isZero())) { return new TConstObj(FALSE); }
  TAndRef res = new TAndObj;
  res->AND(copyAndRemove(target,sourceEQ));
  res->AND(makeAtom(M,c1,EQOP));
  if (!c0->isZero()) res->AND(makeAtom(M,c0,EQOP));
  return res;
}


// target - atom I'm substituting into
//      A - polynomial (best to make sure it's the cannonical ref!)
//      x - a variable, must be variable in which A is linear
// Substitutes generic result of solving A = 0 for x.  "Generic" means
// we're assuming the denominator does not vanish.
TFormRef linearSubstGenericCase(TAtomRef target, IntPolyRef A, VarSet x)
{
  PolyManager &M = *(target->F->M);
  TAtomRef res = new TAtomObj(M);
  res->F->content = target->F->content;
  res->relop = target->relop;
  int lcpow = 0; // track the total power of A's ldcf that gets implicitly multiplied by the resultants
  for(map<IntPolyRef,int>::iterator itr = target->F->MultiplicityMap.begin(); 
      itr != target->F->MultiplicityMap.end(); 
      ++itr)
  {
    IntPolyRef g = itr->first;
    int mg = itr->second;
    int d = g->degree(x);
    if (d == 0) { res->F->addFactor(g,mg); }
    else 
    {
      IntPolyRef R = M.resultant(A,g,x); // gives a^mg * g(root of A), where a = ldcf(A,x)
      res->F->addMultiple(R,mg);
      lcpow += d*mg;
      if (res->F->content == 0) { return constInEqToBoolConst(0,target->relop); }
    }
  }
  if (lcpow % 2 == 1)
  {
    IntPolyRef a = M.ldcf(A,x);
    res->F->addMultiple(a,1);
  }
  if (res->F->isConstant()) return constInEqToBoolConst(ISIGNF(res->F->content),res->relop);
  return res;
}

// This is the function object that does the linear substitutions.
class LinSubGC : public TFPolyFun
{
  TAtomRef E; // this is the Atomic formula A = 0 from which this subst arose.
  bool leaveE;// if this is true, E will be left in the formula, if false, E will be removed.
  IntPolyRef A;
  VarSet x;
public:
  TFormRef res;
  LinSubGC(IntPolyRef _A, VarSet _x, TAtomRef _E, bool _leaveE) 
  { 
    A = _A; x = _x; E = _E; leaveE = _leaveE; 
  }
  virtual void action(TConstObj* p) 
  { 
    res = p;  
  }
  virtual void action(TAtomObj* p) 
  { 
    if (E.vpval() == p)
      if (leaveE) { 
	TAtomObj *q = new TAtomObj(*(p->F->M)); 
	q->relop = p->relop; 
	*(q->F) = *(p->F); 
	res = q; }
      else { res = new TConstObj(TRUE); } 
    else
      res = linearSubstGenericCase(p,A,x);  
  }
  virtual void action(TAndObj* p) 
  { 
    TAndRef C = new TAndObj;
    for(set<TFormRef>::iterator i = p->conjuncts.begin(); i != p->conjuncts.end(); ++i)
    { 
      this->actOn(*i); 
      TConstRef constRes = asa<TConstObj>(res);
      if (!constRes.is_null() && constRes->value == FALSE) { res = new TConstObj(FALSE); return; }
      if (!constRes.is_null() && constRes->value == TRUE) { continue; }
      C->AND(res); 
    }
    if (C->size() == 0) 
      res = new TConstObj(TRUE);
    else
      res = C;
  }

  virtual void action(TOrObj* p) 
  { 
    TOrRef C = new TOrObj;
    for(set<TFormRef>::iterator i = p->disjuncts.begin(); i != p->disjuncts.end(); ++i)
    { 
      this->actOn(*i); 
      TConstRef constRes = asa<TConstObj>(res);
      if (!constRes.is_null() && constRes->value == TRUE) { res = new TConstObj(TRUE); return; }
      if (!constRes.is_null() && constRes->value == FALSE) { continue; }
      C->OR(res); 
    }
    if (C->size() == 0) 
      res = new TConstObj(FALSE);
    else
      res = C;
  }
  virtual void action(TQBObj* p) 
  { 
    // If x is not one of the quantified variable sub for what's inside. Otherwise
    // return unchanged.
    
    TQBRef T = p;
    unsigned int i;
    for(i = 0; i < T->blocks.size() && (T->blocks[i] & x).none(); ++i);
    if (i < T->blocks.size() || (T->formulaPart->getVars() & x).none()) { res = p; return; }

    TQBRef S = new TQBObj(p->getPolyManagerPtr());
    S->outermostBlockType = T->outermostBlockType;
    for(unsigned int j = 0; j < T->blocks.size(); ++j)
      S->blocks.push_back(T->blocks[j]);
    this->actOn(T->formulaPart);
    S->formulaPart = res;
    res = S;
    return;
  }

};

// The function that calls the above to do the linear substitution for the generic case
TFormRef linearSubstGenericCase(PolyManager &M, 
				TFormRef target, 
				IntPolyRef A, 
				VarSet x,
				TAtomRef E, 
				bool leaveE )
{  
  LinSubGC GCPF(A,x,E,leaveE);
  GCPF(target);
  if (constValue(GCPF.res) == FALSE) return GCPF.res;

  IntPolyRef c1 = M.ldcf(A,x);
  if (c1->isConstant()) return GCPF.res;
  TAtomRef A1 = makeAtom(M,c1,NEOP);

  if (constValue(GCPF.res) == TRUE) return A1;

  TAndRef res = asa<TAndObj>(GCPF.res);
  if (res.is_null()) { res = new TAndObj; res->AND(GCPF.res); }
  res->AND(A1);
  return res;
}
}//end namespace tarski
