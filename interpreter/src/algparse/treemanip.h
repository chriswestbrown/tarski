/**********************************************************************
 ** treemanip.h: some basic functions and polymorphic function objects
 ** for manipulating TarskiObj parse trees.
 **********************************************************************/
#ifndef _TREE_MANIP_
#define _TREE_MANIP_
#include "cwbpt.h"

namespace tarski {
class Traverse : public TarskiApply
{
 public:
  virtual void operator()(UnaryOp* p)       { p->arg->apply(*this); }
  virtual void operator()(BinaryOp* p)      { p->arg1->apply(*this); p->arg2->apply(*this); }
  virtual void operator()(TarskiAtom* p)    { p->LHS->apply(*this); p->RHS->apply(*this); }
  virtual void operator()(ExtTarskiAtom* p) { p->LHS->apply(*this); p->rootIndex->apply(*this); 
                                              p->RHS->apply(*this); }
  virtual void operator()(BinaryLogOp* p)   { p->LHS->apply(*this); p->RHS->apply(*this); }
  virtual void operator()(NotOp* p)         { p->arg->apply(*this); }  
  virtual void operator()(QuantifierBlock *p);
};

class FormulaMassage : public TarskiApply
{
public:
  FormulaRef res;
  virtual void operator()(TarskiAtom *p) { res = p; }
  virtual void operator()(ExtTarskiAtom *p) { res = p; }
  virtual void operator()(QuantifierBlock *p) { res = p; }
  virtual void operator()(BinaryLogOp *p)  { res = p; }
  virtual void operator()(NotOp *p) { res = p; }  
  virtual void operator()(TarskiConst *p) { res = p; }  
};

// Given an Algebraic ref returns true if it is the constant zero, false otherswise
inline bool isZero(AlgebraicRef a)
{ 
  Num *p = dynamic_cast<Num*>(&(*a));
  return p != 0 && p->value == "0";
}

// Only call if you're sure you've got a Var!
inline string varName(AlgebraicRef a) { return dynamic_cast<Var*>(&(*a))->value; }
}//end namespace tarski
#endif
