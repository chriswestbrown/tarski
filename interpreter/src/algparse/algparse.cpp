#include "algparse.h"
#include "utils.h"
#include "getvars.h"
#include "prettyprint.h"
#include "printGross.h"
#include <fstream>
#include <sstream>

namespace tarski {
class SExpObj;
typedef GC_Hand<SExpObj> SExpRef;

const char* srep[] = { "","","","+","-"," ","-" };
class SExpObj : public GC_Obj
{
public:
  enum SEType { ERR=0, NUM, VAR, PLUS, MINUS, MULT, UMINUS };  

private:
  std::string aval;
  SEType mykind;
  SExpRef c1, c2;

public:
  bool isOne() { return mykind == NUM && aval == "1"; }
  static SExpRef mkNum(std::string val) { SExpRef p = new SExpObj(); p->aval = val; p->mykind = NUM; return p; }
  static SExpRef mkVar(std::string val) { SExpRef p = new SExpObj(); p->aval = val; p->mykind = VAR; return p; }
  static SExpRef mkUMinus(SExpRef a) { SExpRef p = new SExpObj(); p->mykind = UMINUS; p->c1 = a; return p; }
  static SExpRef mkBinOp(SEType k, SExpRef a, SExpRef b) 
  { 
    if (k == MULT && a->isOne()) { return b; }
    if (k == MULT && b->isOne()) { return a; }
    SExpRef p = new SExpObj(); 
    p->mykind = k; 
    p->c1 = a; 
    p->c2 = b; 
    return p; 
  }
  SEType kind() { return mykind; }
  std::string rep()
  {
    switch(kind())
    {
    case NUM: return aval; break;
    case VAR: return aval; break;
    default: return "";
    }
  }
  SExpRef child(int i) { return i == 0 ? c1 : c2; }
  void write(std::ostream& out)
  {
     switch(kind())
    {
    case NUM: out << aval; break;
    case VAR: out << aval; break;
    case UMINUS: out << "-("; c1->write(out); out << ")"; break;
    case PLUS: case MINUS: case MULT:
      out << "("; c1->write(out); out << ")"; 
      out << srep[kind()];
      out << "("; c2->write(out); out << ")"; 
      break;
    default: break;
    }
  }
};

class MRes
{
public:
  SExpRef f, e;
  MRes() { }
  MRes(SExpRef a, SExpRef b) { f = a; e = b; }
};

class ClearDenoms : public TarskiApply
{
public:
  MRes res;
  std::ostringstream sout;

public:

  virtual void operator()(TarskiConst* p)
  {
    sout << (p->value == TRUE ? "true" : "false");
  }
  
  virtual void operator()(TarskiAtom* p)
  {
    AlgebraicRef ts = new BinaryOp('-',p->LHS,p->RHS);
    ts->apply(*this);
    
    if (p->relop == EQOP || p->relop == NEOP)
    {
      res.e->write(sout);
      sout << relopStrings[p->relop] << " 0";
    }
    else
    {
      sout << '('; res.f->write(sout); sout << ')';
      sout << ' ';
      sout << '('; res.e->write(sout); sout << ')';
      sout << relopStrings[p->relop] << " 0";
    }
  }

  virtual void operator()(ExtTarskiAtom* p)
  {
    p->LHS->write(sout);
    sout << " " << relopStrings[p->relop];
    sout << " _root_";
    p->rootIndex->write(sout);
    sout << " ";
    p->RHS->apply(*this);
    //    res.e->write(sout);
    p->RHS->write(sout);
  }

  
  virtual void operator()(BinaryLogOp* p)
  {
    sout << '['; p->LHS->apply(*this); sout << ']';
    sout << " " << logopStrings[p->logop] << " ";
    sout << '['; p->RHS->apply(*this); sout << ']';
  }
  
  virtual void operator()(NotOp* p)
  {
    sout << "~["; p->arg->apply(*this); sout << ']';
  }

  virtual void operator()(QuantifierBlock* p)
  {
    sout << (p->kind == EXIST ? "ex" : "all") << ' ';
    sout << dynamic_cast<Var*>(&*(p->vars[0]))->value;
    for(unsigned int i = 1; i < p->vars.size(); ++i) { sout << ','; p->vars[i]->print(); }
    sout << "["; 
    if (!p->arg.is_null()) { p->arg->apply(*this); }
    sout << "]";
  }
  
  
  virtual void operator()(Num* p) { res.f = SExpObj::mkNum("1"); res.e = SExpObj::mkNum(p->value); }
  virtual void operator()(Var* p) { res.f = SExpObj::mkNum("1"); res.e = SExpObj::mkVar(p->value); }
  virtual void operator()(UnaryOp* p) 
  { 
    p->arg->apply(*this); MRes A = res; res.f = A.f; res.e = SExpObj::mkUMinus(A.e); 
  }
  virtual void operator()(BinaryOp* p)
  {
    p->arg1->apply(*this); MRes A = res; 
    p->arg2->apply(*this); MRes B = res; 
    char op = p->op;
    switch(op) {
    case '+': case '-': {
      res.f = SExpObj::mkBinOp(SExpObj::MULT,A.f,B.f);
      res.e = SExpObj::mkBinOp((op == '+' ? SExpObj::PLUS : SExpObj::MINUS),
			       SExpObj::mkBinOp(SExpObj::MULT,B.f,A.e),
			       SExpObj::mkBinOp(SExpObj::MULT,A.f,B.e));
      break;
    }
    case '*': {
      res.f = SExpObj::mkBinOp(SExpObj::MULT,A.f,B.f);
      res.e = SExpObj::mkBinOp(SExpObj::MULT,A.e,B.e);
      break;
    }
    case '/': {
      // res.f = SExpObj::mkBinOp(SExpObj::MULT,
      // 			       SExpObj::mkBinOp(SExpObj::MULT,A.f,B.f),
      // 			       SExpObj::mkBinOp(SExpObj::MULT,B.e,B.e)  );
      // res.e = SExpObj::mkBinOp(SExpObj::MULT,
      // 			       SExpObj::mkBinOp(SExpObj::MULT,A.e,B.e),
      // 			       SExpObj::mkBinOp(SExpObj::MULT,B.f,B.f)  );
      res.f = SExpObj::mkBinOp(SExpObj::MULT,A.f,B.e);
      res.e = SExpObj::mkBinOp(SExpObj::MULT,A.e,B.f);
      break;
    }
    case '^': {
      int e = atoi(B.e->rep().c_str());
      if (e == 0) { res.f = A.f; res.e = SExpObj::mkNum("1"); }
      else {
	MRes col;
	col.f = A.f;
	col.e = A.e;
	for(int i = 1; i < e; i++) {
	  col.f = SExpObj::mkBinOp(SExpObj::MULT,col.f,A.f);
	  col.e = SExpObj::mkBinOp(SExpObj::MULT,col.e,A.e);
	}
	res = col;
      }
      break;
    }
    default: {
      throw TarskiException(string("Unknown binary operator '") + op + "'!");
      break;
    }
    }
  }
};

TarskiRef clearDenominators(TarskiRef T)
{
  ClearDenoms CD;
  CD.sout << '[';
  T->apply(CD);
  CD.sout << ']';

   std::cerr << CD.sout.str() << std::endl;
    
  std::istringstream sin(CD.sout.str());
  LexContext LCs(sin);
  TarskiRef Ts;
  ::algparse(&LCs,Ts);    

  return Ts;
}

}//end namespace tarski
