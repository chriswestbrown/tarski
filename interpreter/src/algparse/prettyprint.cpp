#include "prettyprint.h"
#include "../tarski.h"

namespace tarski {
  
bool parensNeededLeft(AlgebraicRef a, AlgebraicRef b)
{
  int i1 = precIndex(a);
  int i2 = precIndex(b);
  if (i1 != i2) return i1 > i2;
  char op1 =  getAlgOp(a);
  char op2 =  getAlgOp(b);
  return op1 == '^'; // because all other binary ops are left-associative anyway
  
}

bool parensNeededRight(AlgebraicRef a, AlgebraicRef b)
{
  int i1 = precIndex(a);
  int i2 = precIndex(b);
  if (i1 != i2) return i1 < i2;
  char op1 =  getAlgOp(a);
  char op2 =  getAlgOp(b);
  return op1 == '/' || op1 == '-';
}

bool parensNeededLeft(FormulaRef a, FormulaRef b)
{
  int i1 = precIndex(a);
  int i2 = precIndex(b);
  if (i1 != i2) return i1 > i2;
  return false; // because all binary logical ops are left-associative anyway  
}

bool parensNeededRight(FormulaRef a, FormulaRef b)
{
  int i1 = precIndex(a);
  int i2 = precIndex(b);
  if (i1 != i2) return i1 < i2;
  BinaryLogOp *p = asa<BinaryLogOp>(a);
  if (p && (p->logop == ANDOP || p->logop == OROP)) return false;
  return true; // because all binary logical ops are left-associative anyway
}


std::string PrettyPrint::optrans(Format f, char op) 
{ 
  std::string s; 
  if ((f == qepcad || f == mathematica) && op == '*') 
    s += ' '; 
  else s += op; 
  return s; 
}

void PrettyPrint::operator()(Num* p)
{ 
  (*pout) << p->value;  
  opBefore = false;
}

void PrettyPrint::operator()(Var* p)
{
  (*pout) << p->value;  
  opBefore = false;
}

void PrettyPrint::operator()(InterpVar* p)
{
  (*pout) << "$" << p->var;  
  opBefore = false;
}

void PrettyPrint::operator()(UnaryOp* p)
{ 
  bool surround = opBefore;
  if (surround) { (*pout) << "("; }

  // NOTE: I had to wrap every unary - in ()'s because 3 + -4 was causing problems. now: 3+(-4)
  //       
  if (precIndex(p) < precIndex(p->arg))
  { (*pout) << "(-("; opBefore = false; p->arg->apply(*this); (*pout) << "))"; opBefore = false; }
  else
  { (*pout) << "(-";   opBefore = true; p->arg->apply(*this); (*pout) << ")"; }    
 
  if (surround) { (*pout) << ")"; }
}

void PrettyPrint::operator()(BinaryOp* p)
{ 
  bool p1n = parensNeededLeft(p->arg1,p);
  bool p2n = parensNeededRight(p,p->arg2);

  if (oform == qepcad && p->op == '/' && (!asa<Num>(p->arg1) || !asa<Num>(p->arg2)))
  {
    if (!asa<Num>(p->arg2)) 
    { 
      std::string msg = "Only constants are allowed in denominators!";
      throw TarskiException(msg);
    }
    else { 
      if (p1n) { (*pout) << "("; } opBefore = false; p->arg1->apply(*this); if (p1n) { (*pout) << ")"; }
      (*pout) << optrans(oform,'*');
      (*pout) << "1/"; 
      opBefore = false;
      p->arg2->apply(*this);
    }
  }
  else if (oform == qepcad && p->op == '/')
  {
    (*pout) << "("; p->arg1->apply(*this); (*pout) << "/"; p->arg2->apply(*this); (*pout) << ")";
  }
  else
  {
    if (p1n) { (*pout) << "("; opBefore = false; p->arg1->apply(*this); (*pout) << ")"; }
    else     { p->arg1->apply(*this); }
    (*pout) << optrans(oform,p->op); 
    if (p2n) { (*pout) << "("; opBefore = false; p->arg2->apply(*this); (*pout) << ")"; }
    else     { opBefore = true; p->arg2->apply(*this); }
  }
}


static const char *redlogRelopStrings[8] = { "ERR", "<", "=", "<=", ">", "<>", ">=", "??" };
static const char *redlogLogopStrings[17] = { "ERR","ERR","ERR","ERR","ERR","ERR","ERR","ERR","ERR","ERR","ERR",
				  "and", "or", "not", "impl", "repl", "equiv" };
static const char *mathematicaRelopStrings[8] = { "ERR", "<", "==", "<=", ">", "!=", ">=", "??" };
static const char *mathematicaLogopStrings[17] = { "ERR","ERR","ERR","ERR","ERR","ERR","ERR","ERR","ERR","ERR","ERR",
				  "&&", "||", "!", "\\[Implies]", "ERR", "\\[Equivalent]" };


void PrettyPrint::operator()(TarskiAtom* p)    { 
  p->LHS->apply(*this); 
  (*pout) << ' ' 
	  << (oform == redlog ? redlogRelopStrings[p->relop] : 
	      (oform == mathematica ? mathematicaRelopStrings[p->relop] : relopStrings[p->relop]) )
	  << ' '; p->RHS->apply(*this); }
void PrettyPrint::operator()(ExtTarskiAtom* p) { 
  p->LHS->apply(*this); 
  (*pout) << ' ' 
	  << (oform == redlog ? redlogRelopStrings[p->relop] :
	      (oform == mathematica ? mathematicaRelopStrings[p->relop] : relopStrings[p->relop]) )
	  << ' '; 
  (*pout) << "_root_"; p->rootIndex->apply(*this); std::cout << " "; 
  p->RHS->apply(*this); }
void PrettyPrint::operator()(BinaryLogOp* p)   { 
  std::string LP = ((oform == redlog || oform == mathematica) ? "(" : "[");
  std::string RP = ((oform == redlog || oform == mathematica) ? ")" : "]");
  bool p1n = parensNeededLeft(p->LHS,p)  || oform == qepcad;
  bool p2n = parensNeededRight(p,p->RHS) || oform == qepcad;
  if (p1n) (*pout) << LP; p->LHS->apply(*this); if (p1n) (*pout) << RP;
  (*pout) << ' ' << 
    (oform == redlog ? redlogLogopStrings[p->logop] : 
     (oform == mathematica ? mathematicaLogopStrings[p->logop] : logopStrings[p->logop])) 
	  << ' ';
  if (p2n) (*pout) << LP; p->RHS->apply(*this); if (p2n) (*pout) << RP; 
}
void PrettyPrint::operator()(NotOp* p)         {
  if (oform == redlog)
    if (precIndex(p) < precIndex(p->arg)) { (*pout) << "not("; p->arg->apply(*this); (*pout) << ")"; }
    else { (*pout) << "not "; p->arg->apply(*this); }
  else if (oform == mathematica)
    if (precIndex(p) < precIndex(p->arg)) { (*pout) << "!("; p->arg->apply(*this); (*pout) << ")"; }
    else { (*pout) << "! "; p->arg->apply(*this); }
  else
    if (precIndex(p) < precIndex(p->arg)) { (*pout) << "~["; p->arg->apply(*this); (*pout) << "]"; }
    else { (*pout) << "~"; p->arg->apply(*this); }
}
void PrettyPrint::operator()(TarskiConst *p)
{
  if (oform == qepcad) { (*pout) << (p->value == TRUE ? "0 = 0" : "0 /= 0"); }
  else (*pout) << (p->value == TRUE ? "true" : "false");
}
void PrettyPrint::operator()(QuantifierBlock* p)
{
  if (oform == native)
  {
    (*pout) << (p->kind == UNIVER ? "all" : "ex") << ' ';
    std::vector<AlgebraicRef>::iterator i = p->vars.begin();
    (*i)->apply(*this);
    for(++i; i != p->vars.end(); ++i) { (*pout) << ","; (*i)->apply(*this); }
    (*pout) << "[";
    p->arg->apply(*this);
    (*pout) << "]";
  }
  else if (oform == qepcad)
  {
    std::string qs = p->kind == UNIVER ? "A" : "E";
    for(std::vector<AlgebraicRef>::iterator i = p->vars.begin(); i != p->vars.end(); ++i)
    { (*pout) << "(" << qs << ' ';  (*i)->apply(*this);  (*pout) << ")"; }
    (*pout) << "[";
    p->arg->apply(*this);
    (*pout) << "]";
  }
  else if (oform == redlog)
  {
    (*pout) << (p->kind == UNIVER ? "all" : "ex") << '(';
    if (p->vars.size() > 1) (*pout) << "{";
    std::vector<AlgebraicRef>::iterator i = p->vars.begin();
    (*i)->apply(*this);
    for(++i; i != p->vars.end(); ++i) { (*pout) << ","; (*i)->apply(*this); }
    if (p->vars.size() > 1) (*pout) << "}";
    (*pout) << ",";
    p->arg->apply(*this);
    (*pout) << ')';
  }
  else if (oform == mathematica)
  {
    (*pout) << (p->kind == UNIVER ? "ForAll" : "Exists") << '[';
    if (p->vars.size() > 1) (*pout) << "{";
    std::vector<AlgebraicRef>::iterator i = p->vars.begin();
    (*i)->apply(*this);
    for(++i; i != p->vars.end(); ++i) { (*pout) << ","; (*i)->apply(*this); }
    if (p->vars.size() > 1) (*pout) << "}";
    (*pout) << ",";
    p->arg->apply(*this);
    (*pout) << ']';
  }
}

}//end namespace tarski
