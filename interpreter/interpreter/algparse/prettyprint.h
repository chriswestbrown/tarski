/********************************************************************************
 * This file defines the following class
 *
 *    class PrettyPrint : public TarskiApply
 *
 * which can be used to "pretty print" a formula ... i.e. print without
 * redundant parenthesization.  It's a function object, whose constructor takes
 * an object of type PrettyPrint::Format, which allows you to specify printing
 * in qepcad, redlog or "native" (i.e. algparse's own input format) formats.
 ********************************************************************************/
#ifndef _PRETTY_PRINT_
#define _PRETTY_PRINT_
#include "treemanip.h"

namespace tarski {
/********************************************************************************
 * parensNeeded considers an expression X a Y b Z and determines whether parens
 * are needed
 ********************************************************************************/
class Precedence : public TarskiApply
{
public:
  int N;  
  virtual void operator()(Num* p) { N = 1; }
  virtual void operator()(Var* p) { N = 1; }
  virtual void operator()(UnaryOp* p) { N = 3; }
  virtual void operator()(BinaryOp* p)
  {
    if (p->op == '^') N = 2;
    else if (p->op == '*' || p->op == '/') N = 4;
    else N = 5;
  }

  virtual void operator()(TarskiConst* p) { N = 10; }
  virtual void operator()(InterpVar* p) { N = 10; }
  virtual void operator()(TarskiAtom* p) { N = 10; }
  virtual void operator()(ExtTarskiAtom* p) { N = 10; }
  virtual void operator()(QuantifierBlock* p) { N = 11; }
  virtual void operator()(NotOp* p) { N = 13; }
  virtual void operator()(BinaryLogOp* p)
  {
    if (p->logop == ANDOP) { N = 15; }
    else { N = 18; }
  }
};
inline int precIndex(AlgebraicRef a) { Precedence P; a->apply(P); return P.N; }
inline int precIndex(FormulaRef a) { Precedence P; a->apply(P); return P.N; }
inline char getAlgOp(AlgebraicRef a) { BinaryOp *p = dynamic_cast<BinaryOp*>(&(*a)); return p ? p->op : 0; }

bool parensNeededLeft(AlgebraicRef a, AlgebraicRef b);
bool parensNeededRight(AlgebraicRef a, AlgebraicRef b);
bool parensNeededLeft(FormulaRef a, FormulaRef b);
bool parensNeededRight(FormulaRef a, FormulaRef b);

/********************************************************************************
 * Prints formulas nicely ... without redundant parenthesese 
 ********************************************************************************/
class PrettyPrint : public TarskiApply
{
private:
  bool opBefore;
  
public:
  enum Format { native = 0, qepcad = 1, redlog = 2, mathematica = 3 };
  std::ostream *pout;
  Format oform;

  std::string optrans(Format f, char op);
  PrettyPrint(std::ostream &out, Format f = native) { pout = &out; oform = f; opBefore = false; }

  virtual void operator()(Num* p);
  virtual void operator()(Var* p);
  virtual void operator()(InterpVar* p);
  virtual void operator()(UnaryOp* p);
  virtual void operator()(BinaryOp* p);
  virtual void operator()(TarskiAtom* p);
  virtual void operator()(ExtTarskiAtom* p);
  virtual void operator()(BinaryLogOp* p);
  virtual void operator()(NotOp* p);
  virtual void operator()(TarskiConst *p);
  virtual void operator()(QuantifierBlock* p);
};
}//end namespace tarski
#endif
