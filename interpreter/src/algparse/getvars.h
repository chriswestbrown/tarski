#ifndef _GET_VARS_
#define _GET_VARS_
#include "treemanip.h"

namespace tarski {
/********************************************************************************
 * If T is a Tarski Formula and G is a GetVars object, T->apply(G) leaves G in
 * a state in which G.vars is a the std::set of variables appearing in T.
 ********************************************************************************/
class GetVars : public TarskiApply
{
public:
  std::set<std::string> vars;
  std::set<std::string> freeVars;
  std::set<std::string> boundVars;
  virtual void operator()(Var* p);
  virtual void operator()(UnaryOp* p)       { p->arg->apply(*this); }
  virtual void operator()(BinaryOp* p)      { p->arg1->apply(*this); p->arg2->apply(*this); }
  virtual void operator()(TarskiAtom* p)    { p->LHS->apply(*this); p->RHS->apply(*this); }
  virtual void operator()(ExtTarskiAtom* p) { p->LHS->apply(*this); p->rootIndex->apply(*this); 
                                              p->RHS->apply(*this); }
  virtual void operator()(BinaryLogOp* p)   { p->LHS->apply(*this); p->RHS->apply(*this); }
  virtual void operator()(NotOp* p)         { p->arg->apply(*this); }  
  virtual void operator()(QuantifierBlock *p);
};


// Renames variable OldName as NewName
// Modifying version!
class RenameVar : public Traverse
{
public:
  std::string oldName, newName;
  RenameVar(const std::string &oldN, const std::string &newN) : oldName(oldN), newName(newN) { }
  virtual void operator()(Var* p);
  virtual void operator()(QuantifierBlock *p);
};

std::string distinctName(std::set<std::string> &S, std::string x);

// Modifying version!
class DistinctVarNames : public Traverse
{
public:
  GetVars GV; // When done, GV contains all variable names
              // as well as a break-down into free and bound.

  virtual void operator()(Var *p);
  virtual void operator()(QuantifierBlock *p);
};
}//end namespace tarski
#endif
