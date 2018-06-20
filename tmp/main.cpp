#include "algparse.h"
#include "utils.h"
#include "getvars.h"
#include "prettyprint.h"
#include "printGross.h"
#include <fstream>
#include <sstream>

using namespace tarski;

// Non-destructive
class ThrowAwayQuantifiers : public FormulaMassage
{
  virtual void operator()(QuantifierBlock *p) { p->arg->apply(*this); }
  virtual void operator()(BinaryLogOp *p)  { 
    FormulaRef L, R;
    p->LHS->apply(*this); L = res;
    p->RHS->apply(*this); R = res;
    BinaryLogOp *q = new BinaryLogOp(L,p->logop,R);
    res = q;
  }
  virtual void operator()(NotOp *p) { 
    FormulaRef A;
    p->arg->apply(*this); A = res;
    res = new NotOp(A);
  }  
};


/* 
   Quantifier Flatten ... nested quantifiers of the same type are collected together
*/
class QFFlatten : public FormulaMassage
{
public:
  virtual void operator()(QuantifierBlock *p)
  {
    QFFlatten P; p->arg->apply(P);
    QuantifierBlock *q = dynamic_cast<QuantifierBlock*>(&(*(P.res)));
    if (!q || q->kind != p->kind) 
      res = p;
    else
    {
      QuantifierBlock *r = new QuantifierBlock(*p);
      for(unsigned int i = 0; i < q->vars.size(); ++i)
	r->vars.push_back(q->vars[i]);
      r->arg = q->arg;
      res = r;
    }
  }
};
FormulaRef qfflatten(FormulaRef f) { QFFlatten P; f->apply(P); return P.res; }

class BreakTopLevelOrs : public FormulaMassage
{
public:
  std::vector<FormulaRef> S;
  virtual void operator()(BinaryLogOp *p)
  {
    if (p->logop != OROP) { res = p; return; }
    BreakTopLevelOrs SL, SR;
    p->LHS->apply(SL);
    p->RHS->apply(SR);
    if (SL.S.size() == 0)
      S.push_back(SL.res);
    else
      for(unsigned int i = 0; i < SL.S.size(); ++i)
	S.push_back(SL.S[i]);
    if (SR.S.size() == 0)
      S.push_back(SR.res);
    else
      for(unsigned int i = 0; i < SR.S.size(); ++i)
	S.push_back(SR.S[i]);
  }
};


class StatsForVar : public Traverse
{
public:
  std::string var;
  int maxd, maxtd, numt; // max degree, max total degree of terms containing var, number of terms containing var
  int td; // total degree of any term, even if var is not involved!

  StatsForVar(const std::string &v) : var(v) { td = maxd = maxtd = numt = 0; }
  void print()
  {
    std::cout << var << ": maxd = " << maxd << ", maxtd = " << maxtd << ", numt = " << numt << ", td = " << td << std::endl;
  }
  
  virtual void operator()(BinaryOp* p)      
  { 
    StatsForVar X1(var), X2(var);
    p->arg1->apply(X1); 
    p->arg2->apply(X2); 
    if (p->op == '+' || p->op == '-') 
    { 
      maxd += max(X1.maxd,X2.maxd); 
      maxtd += max(X1.maxtd,X2.maxtd); 
      td += max(X1.td,X2.td);
      numt += X1.numt + X2.numt;
    }
    else if (p->op == '*' || p->op == '/')
    {
      maxd += X1.maxd+X2.maxd;
      td += X1.td + X2.td;
      maxtd += max(X1.td + X2.maxtd, X1.maxtd + X2.td);
      // to deal with numt properly I need to also track total number of terms!
      if (X1.numt > 0 && X2.numt > 0) numt += X1.numt * X2.numt;
      else if (X1.numt > 0) numt += X1.numt;
      else if (X2.numt > 0) numt += X2.numt;
    }
    else if (p->op == '^')
    {
      Num *np = asa<Num>(p->arg2);
      if (np == 0) { std::cerr << "Error! exponent not constant!" << std::endl; exit(1); }
      int k = atoi(np->value.c_str());
      td += X1.td*k;
      maxd += X1.maxd*k;
      maxtd += X1.maxtd*k;
      numt += X1.numt < 2 ? X1.numt : X1.numt*k; // NOTE: I'd need to track more data to do this right!
    }
  }

  virtual void operator()(Var* p)
  {
    td += 1;
    if (var != p->value) return;
    maxd += 1;
    maxtd += 1;
    numt += 1;
  }

  virtual void operator()(TarskiAtom *p)
  { 
    StatsForVar X1(var), X2(var);
    p->LHS->apply(X1); 
    p->RHS->apply(X2); 
    maxd  = max(maxd,max(X1.maxd,X2.maxd));
    maxtd = max(maxtd,max(X1.maxtd,X2.maxtd));
    numt += X1.numt + X2.numt; 
    td = max(td,max(X1.td,X2.td));
  }
};


bool operator<(const StatsForVar &a, const StatsForVar &b) {
  return 
    a.maxd < b.maxd || a.maxd == b.maxd && 
    (a.maxtd < b.maxtd || a.maxtd == b.maxtd &&
     (a.numt < b.numt || a.numt == b.numt && a.var < b.var));
}

void printQEPCAD(std::set<StatsForVar> &FV, std::set<StatsForVar> &BV, FormulaRef F, std::string H, std::ostream &out);


void printQEPCAD(std::set<StatsForVar> &FV, std::set<StatsForVar> &BV, FormulaRef F, std::string H,std::ostream &out)
{
  out << "[" << H << "]" << std::endl;
  out << "(";
  if (FV.size() != 0)
  {
    std::set<StatsForVar>::reverse_iterator i = FV.rbegin();
    out << i->var;
    for(++i; i != FV.rend(); ++i)
      out << "," << i->var;
    for( std::set<StatsForVar>::reverse_iterator j = BV.rbegin(); j != BV.rend(); ++j)
      out << "," << j->var;    
  }
  else
  {
    std::set<StatsForVar>::reverse_iterator i = BV.rbegin();
    out << i->var;
    for(++i; i != BV.rend(); ++i)
      out << "," << i->var;
  }
  out << ")" << std::endl;
  out << FV.size() << std::endl;
  
  for(std::set<StatsForVar>::reverse_iterator i = BV.rbegin(); i != BV.rend(); ++i)
    out << "(E " << i->var << ")";
  
  PrettyPrint PP(out,PrettyPrint::qepcad);
  out << "[";
  F->apply(PP);
  out << "]." << std::endl;
  out << "finish" << std::endl;
}



uint64 GC_Obj::next_tag=0;
uint64 GC_Obj::num_deleted=0;
/***************************************************************/
const std::string helpString="\
frewrite: Formula Rewrite\n\
Reads in a formula and writes it in different formats.\n\
-e : echos in interpreter format\n\
-q : prints in QEPCAD format ... breaking up top-level ORs\n\
-g : prints in format for Christian Gross's maple preprocessor.\n\
-r : prints in redlog format\n\
-m : prints in mathematica format\n";
/***************************************************************/
int main(int nargs, char **argv)
{ 
  bool gross = false, qepcad = false, echo = false, 
    redlog = false, mathematica = false, help = false;

  if (nargs == 1)
    echo = true;
  else
  {
    for(int i = 1; i < nargs; ++i)
    {
      std::string f = argv[i];
      gross |= f == "-g";
      qepcad |= f == "-q";
      redlog |= f == "-r";
      mathematica |= f == "-m";
      echo |= f == "-e";
      help |= f == "-h";
    }
  }
  if (help)
  {
    std::cout << helpString;
    return 0;
  }

  // Parse input
  LexContext LC(std::cin);
  TarskiRef T;
  algparse(&LC,T);    
  if (T.is_null())
  {
    std::cerr << "Parse error!" << std::endl;
    exit(1);
  }

  //TEST
  {
    TarskiRef Ts = clearDenominators(T);
    if (Ts.is_null())
    {
      std::cerr << "Parse error after clearing denominators!" << std::endl;
      exit(1);
    }
    PrettyPrint PP(std::cout);
    Ts->apply(PP);
    std::cout << std::endl;
  }
  

  if (echo)
  {
    std::cout << std::endl;
    PrettyPrint PP(std::cout);
    T->apply(PP);
    std::cout << std::endl;
  }
  
  if (redlog)
  {
    std::cout << std::endl;
    PrettyPrint PP(std::cout,PrettyPrint::redlog);
    T->apply(PP);
    std::cout << std::endl;
  }

  if (mathematica)
  {
    std::cout << std::endl;
    PrettyPrint PP(std::cout,PrettyPrint::mathematica);
    T->apply(PP);
    std::cout << std::endl;
  }
  
  if (gross)
  {
    std::cout << std::endl;
    PrintGross PG(std::cout);
    T->apply(PG);
    std::cout << std::endl;
  }

  if (qepcad)
  {
    // Get disctinct variable names
    DistinctVarNames DVN;
    T->apply(DVN);
    std::set<std::string> &freeVars  = DVN.GV.freeVars;
    std::set<std::string> &boundVars = DVN.GV.boundVars;
    
    // Remove the quantifiers
    ThrowAwayQuantifiers TAQ;
    T->apply(TAQ);
    T = &(*(TAQ.res));
    
    // Break the top-level ors
    BreakTopLevelOrs B;
    T->apply(B);
    if (B.S.size() == 0) B.S.push_back(asa<FormulaObj>(T));
    
    // Output pieces to files!
    for(unsigned int i = 0; i < B.S.size(); ++i)
    {
      // Find variable std::set for this piece
      GetVars GVi;
      B.S[i]->apply(GVi);
      std::set<std::string> freeVarsi = intersect(GVi.vars,freeVars);
      std::set<std::string> boundVarsi = intersect(GVi.vars,boundVars);
      if (freeVarsi.size() == 0 && boundVarsi.size() == 0)
	continue;
      
      // Get bound variable "stats" for order heuristic
      std::set<StatsForVar> BV;
      for(std::set<std::string>::iterator j = boundVarsi.begin(); j != boundVarsi.end(); ++j)
      {
	StatsForVar SFV(*j);
	B.S[i]->apply(SFV);
	BV.insert(SFV);
      }
      
      // Get free variable "stats" for order heuristic
      std::set<StatsForVar> FV;
      for(std::set<std::string>::iterator j = freeVarsi.begin(); j != freeVarsi.end(); ++j)
      {
	StatsForVar SFV(*j);
	B.S[i]->apply(SFV);
	FV.insert(SFV);
      }
      
      printQEPCAD(FV,BV,B.S[i],"",std::cout);
    }
    std::cout << std::endl << std::endl;
  }

  return 0;
}
