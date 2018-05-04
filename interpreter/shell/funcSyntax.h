#ifndef _FUNC_SYNTAX_
#define _FUNC_SYNTAX_

#include "einterpreter.h"

namespace tarski {

// Array gives strings for each of the relational operators
static const char *mathematicaRelopStrings[8] = { "ERR", "<", "==", "<=", ">", "!=", ">=", "??" };
inline std::string mathematicaRelopString(int r) { return mathematicaRelopStrings[r]; }

static const char *mapleRelopStrings[8] = { "ERR", "<", "=", "<=", ">", "<>", ">=", "??" };
inline std::string mapleRelopString(int r) { return mapleRelopStrings[r]; }


void writeMaple(TFormRef T, std::ostream& out);
void writeMathematica(TFormRef T, std::ostream& out);
void writeSyNRAC(TFormRef T, std::ostream& out);
void writeREDLOG(TFormRef T, std::ostream& out);

class CommSyntax : public EICommand
{
 public:
 CommSyntax(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, std::vector<SRef> &args)
  {
    SymRef target = args[0]->sym();
    TarRef formula = args[1]->tar();
    AlgRef polynom = args[1]->alg();
    std::ostringstream ostr;
    if (target->getVal() == "maple")
    {
      if (formula.is_null())
	polynom->getVal()->writeMAPLE(*(interp->getPolyManagerPtr()),ostr);
      else
	writeMaple(formula->getValue(),ostr);
    }
    else if (target->getVal() == "synrac")
    {
      if (formula.is_null())
	polynom->getVal()->writeMAPLE(*(interp->getPolyManagerPtr()),ostr);
      else
	writeSyNRAC(formula->getValue(),ostr);
    }
    else if (target->getVal() == "redlog")
    {
      if (formula.is_null())
	polynom->getVal()->writeMAPLE(*(interp->getPolyManagerPtr()),ostr);
      else
	writeREDLOG(formula->getValue(),ostr);
    }
    else if (target->getVal() == "mathematica")
    {
      if (formula.is_null())
	polynom->getVal()->writeMATHEMATICA(*(interp->getPolyManagerPtr()),ostr);
      else
	writeMathematica(formula->getValue(),ostr);
    }
    else
      return new ErrObj("Unkown syntax target \"" + target->getVal() + "\" in function syntax.");
    return new StrObj(ostr.str());
  }
  std::string testArgs(std::vector<SRef> &args)
  {
    std::string s = require(args,_sym,_tar);
    return s == "" ? s : require(args,_sym,_alg);
  }
  std::string doc() { return "Convert the syntactic representation of a formula or polynomial.  The first argument is a symbol defining the target syntax, which right now is: 1. 'maple for the syntax used by the Maple RegularChains library.  E.g. (syntax 'maple [x y - 1 < 0 /\\ x > 0]) produces \"x*y-1<0 &and x> 0\". 2. 'mathematica for the syntax used by Mathematica."; }
  std::string usage() { return "(syntax target formula) or (syntax target polynomial)"; }
  std::string name() { return "syntax"; }
};

class WriteMaple : public TFPolyFun
{
  std::ostream* pout;    
 public:
  WriteMaple(std::ostream &out) { pout = &out; }
  void action(TConstObj* p) { (*pout) << (p->constValue() == TRUE ? "true" : "false");  }
  void action(TAtomObj* p)
  {
    p->getFactors()->writeMAPLE(*(p->getPolyManagerPtr()),*pout);
    (*pout) << " " << mapleRelopString(p->getRelop()) << " 0";
  }
  void action(TExtAtomObj* p)
  {
    throw TarskiException("Error in function 'syntax': tranlation of extended Tarski formulas to Maple not yet implemented!");
  }
  void action(TAndObj* p) 
  {
    for(std::set<TFormRef>::iterator itr = p->conjuncts.begin(); itr != p->conjuncts.end(); ++itr)
    {
      if (itr != p->conjuncts.begin())
	(*pout) << "&and";
      (*pout) << "(";
      actOn(*itr);
      (*pout) << ")";
    }
  }
  void action(TOrObj* p)
  { 
    for(std::set<TFormRef>::iterator itr = p->disjuncts.begin(); itr != p->disjuncts.end(); ++itr)
    {
      if (itr != p->disjuncts.begin())
	(*pout) << "&or";
      (*pout) << "(";
      actOn(*itr);
      (*pout) << ")";
    }
  }
  void action(TQBObj* p)
  {
    std::ostream &out = *pout;
    PolyManager *pM = p->getPolyManagerPtr();
    int N = p->numBlocks();
    for(int k = N-1; k >= 0; k--)
    {
      if (p->blockType(k) == EXIST) out << "&E"; else out << "&A";	
      out << "([";
      VarSet V = p->blockVars(k);
      VarSet::iterator itr = V.begin();
      out << pM->getName(*itr);
      for(++itr; itr != V.end(); ++itr) out << "," << pM->getName(*itr);
      out << "]) , ";
    }
    
    out << "(";
    this->actOn(p->getFormulaPart());
    out << ")";
  }
};

class PrecLevel : public TFPolyFun
{  
 public:
  int k;
  void action(TConstObj* p) { k = 0; }
  void action(TAtomObj* p)  { k = 0; }
  void action(TExtAtomObj* p) { k = 0; }
  void action(TAndObj* p) { k = 1; }
  void action(TOrObj* p) { k = 2; }
  void action(TQBObj* p) { k = 0; }
};
int getPrecLevel(TFormRef p) { PrecLevel P; P.actOn(p); return P.k; }

class WriteMathematica : public TFPolyFun
{
  std::ostream* pout;    
 public:
  WriteMathematica(std::ostream &out) { pout = &out; }
  void action(TConstObj* p) { (*pout) << (p->constValue() == TRUE ? "True" : "False");  }
  void action(TAtomObj* p)
  {
    p->getFactors()->writeMATHEMATICA(*(p->getPolyManagerPtr()),*pout);
    (*pout) << " " << mathematicaRelopString(p->getRelop()) << " 0";
  }
  void action(TExtAtomObj* p)
  {
    throw TarskiException("Error in function 'syntax': tranlation of extended Tarski formulas to Maple not yet implemented!");
  }
  void action(TAndObj* p) 
  {
    for(std::set<TFormRef>::iterator itr = p->conjuncts.begin(); itr != p->conjuncts.end(); ++itr)
    {
      if (itr != p->conjuncts.begin())
	(*pout) << " && ";
      if (getPrecLevel(p) < getPrecLevel(*itr)) { (*pout) << "("; actOn(*itr); (*pout) << ")"; }
      else { actOn(*itr); }
    }
  }
  void action(TOrObj* p)
  { 
    for(std::set<TFormRef>::iterator itr = p->disjuncts.begin(); itr != p->disjuncts.end(); ++itr)
    {
      if (itr != p->disjuncts.begin())
	(*pout) << " || ";
      if (getPrecLevel(p) < getPrecLevel(*itr)) { (*pout) << "("; actOn(*itr); (*pout) << ")"; }
      else { actOn(*itr); }
    }
  }
  void action(TQBObj* p)
  {
    std::ostream &out = *pout;
    PolyManager *pM = p->getPolyManagerPtr();
    int N = p->numBlocks();
    for(int k = N-1; k >= 0; k--)
    {
      if (p->blockType(k) == EXIST) out << "Exists"; else out << "ForAll";	
      out << "[{";
      VarSet V = p->blockVars(k);
      VarSet::iterator itr = V.begin();
      out << pM->getName(*itr);
      for(++itr; itr != V.end(); ++itr) out << "," << pM->getName(*itr);
      out << "} , ";
    }
    
    this->actOn(p->getFormulaPart());

    for(int i = 0; i < N; i++)
      out << "]";    
  }
};


class WriteSyNRAC : public TFPolyFun
{
  std::ostream* pout;    
 public:
  WriteSyNRAC(std::ostream &out) { pout = &out; }
  void action(TConstObj* p) { (*pout) << (p->constValue() == TRUE ? "true" : "false");  }
  void action(TAtomObj* p)
  {
    p->getFactors()->writeMAPLE(*(p->getPolyManagerPtr()),*pout);
    (*pout) << " " << mapleRelopString(p->getRelop()) << " 0";
  }
  void action(TExtAtomObj* p)
  {
    throw TarskiException("Error in function 'syntax': tranlation of extended Tarski formulas to SyNRAC not yet implemented!");
  }
  void action(TAndObj* p) 
  {
    (*pout) << "And(";
    for(std::set<TFormRef>::iterator itr = p->conjuncts.begin(); itr != p->conjuncts.end(); ++itr)
    {
      if (itr != p->conjuncts.begin())
	(*pout) << ", ";
      actOn(*itr);
    }
    (*pout) << ")";
  }
  void action(TOrObj* p)
  { 
    (*pout) << "Or(";
    for(std::set<TFormRef>::iterator itr = p->disjuncts.begin(); itr != p->disjuncts.end(); ++itr)
    {
      if (itr != p->disjuncts.begin())
	(*pout) << ", ";
      actOn(*itr);
    }
    (*pout) << ")";
  }
  void action(TQBObj* p)
  {
    std::ostream &out = *pout;
    PolyManager *pM = p->getPolyManagerPtr();
    int N = p->numBlocks();
    for(int k = N-1; k >= 0; k--)
    {
      if (p->blockType(k) == EXIST) out << "Ex"; else out << "All";	
      out << "([";
      VarSet V = p->blockVars(k);
      VarSet::iterator itr = V.begin();
      out << pM->getName(*itr);
      for(++itr; itr != V.end(); ++itr) out << "," << pM->getName(*itr);
      out << "] , ";
    }
    this->actOn(p->getFormulaPart());
    out << ")";
  }
};


class WriteREDLOG : public TFPolyFun
{
  std::ostream* pout;    
 public:
  WriteREDLOG(std::ostream &out) { pout = &out; }
  void action(TConstObj* p) { (*pout) << (p->constValue() == TRUE ? "true" : "false");  }
  void action(TAtomObj* p)
  {
    p->getFactors()->writeMAPLE(*(p->getPolyManagerPtr()),*pout);
    (*pout) << " " << mapleRelopString(p->getRelop()) << " 0"; // REDLOG uses the same relops as Maple!
  }
  void action(TExtAtomObj* p)
  {
    throw TarskiException("Error in function 'syntax': tranlation of extended Tarski formulas to REDLOG not yet implemented!");
  }
  void action(TAndObj* p) 
  {
    (*pout) << "and(";
    for(std::set<TFormRef>::iterator itr = p->conjuncts.begin(); itr != p->conjuncts.end(); ++itr)
    {
      if (itr != p->conjuncts.begin())
	(*pout) << ", ";
      actOn(*itr);
    }
    (*pout) << ")";
  }
  void action(TOrObj* p)
  { 
    (*pout) << "or(";
    for(std::set<TFormRef>::iterator itr = p->disjuncts.begin(); itr != p->disjuncts.end(); ++itr)
    {
      if (itr != p->disjuncts.begin())
	(*pout) << ", ";
      actOn(*itr);
    }
    (*pout) << ")";
  }
  void action(TQBObj* p)
  {
    std::ostream &out = *pout;
    PolyManager *pM = p->getPolyManagerPtr();
    int N = p->numBlocks();
    for(int k = N-1; k >= 0; k--)
    {
      if (p->blockType(k) == EXIST) out << "ex"; else out << "all";	
      out << "({";
      VarSet V = p->blockVars(k);
      VarSet::iterator itr = V.begin();
      out << pM->getName(*itr);
      for(++itr; itr != V.end(); ++itr) out << "," << pM->getName(*itr);
      out << "} , ";
    }
    this->actOn(p->getFormulaPart());
    out << ")";
  }
};


void writeMaple(TFormRef T, std::ostream& out)
{
  WriteMaple WM(out);
  WM.actOn(T);
}

void writeMathematica(TFormRef T, std::ostream& out)
{
  WriteMathematica WM(out);
  WM.actOn(T);
}

void writeSyNRAC(TFormRef T, std::ostream& out)
{
  WriteSyNRAC WM(out);
  WM.actOn(T);
}

void writeREDLOG(TFormRef T, std::ostream& out)
{
  WriteREDLOG WM(out);
  WM.actOn(T);
}
}//end namespace tarski
#endif

