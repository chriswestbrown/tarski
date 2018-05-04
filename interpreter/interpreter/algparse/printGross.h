#ifndef _PRINT_GROSS_
#define _PRINT_GROSS_

#include "prettyprint.h"
#include "utils.h"

namespace tarski {

/********************************************************************************
 * Prints formulas in Christian Gross's Maple program format
 * Notice that I only need to change the behaviour of PrettyPrint for two 
 * two types of nodes.
 ********************************************************************************/
class PrintGross : public PrettyPrint
{
public:
  PrintGross(ostream &out) : PrettyPrint(out) { }
  virtual void operator()(TarskiAtom* p)    { 
    if (isZero(p->RHS)) { p->LHS->apply(*this); (*pout) << relopStrings[p->relop] << "0"; }
    else if (isZero(p->LHS)) { p->RHS->apply(*this); (*pout) << relopStrings[reverseRelop(p->relop)] << "0"; }
    else {
      AlgebraicRef R = new BinaryOp('-',p->LHS,p->RHS);
      R->apply(*this); (*pout) << relopStrings[p->relop] << "0"; } }
  virtual void operator()(BinaryLogOp* p)   { 
    (*pout) << "("; p->LHS->apply(*this); (*pout) << ")";
    string op;
    if (p->logop == ANDOP) op = "&and";
    else if (p->logop == OROP) op = "&or";
    else op = "&unknownop";
    (*pout) << op;
    (*pout) << "("; p->RHS->apply(*this); (*pout) << ")"; }
  virtual void operator()(QuantifierBlock* p)
  {
    string qs = p->kind == UNIVER ? "&all" : "&exists";
    for(vector<AlgebraicRef>::iterator i = p->vars.begin(); i != p->vars.end(); ++i) 
    { (*pout) << qs << "("; (*i)->apply(*this); (*pout) << ","; }
    p->arg->apply(*this);
    for(vector<AlgebraicRef>::iterator i = p->vars.begin(); i != p->vars.end(); ++i) 
    { (*pout) << ")"; }
  }
};
}// end namespace tarski
#endif
