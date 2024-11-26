#include "nontarski.h"
#include "../tarski.h"
#include "bexpr.h"
#include "cwbpt.h"
#include "treemanip.h"
#include "../poly/factor.h"
#include "../formula/formmanip.h"
#include "../formula/normalize.h"

namespace tarski {

  class ToBExp : public Traverse
  {
  public:
    BExpRef res;
    virtual void operator()(Num* p)    { res = mknum(stringToInteger(p->value)); }
    virtual void operator()(Var* p)    { res = mkvar(p->value); }
    virtual void operator()(UnaryOp* p){
      p->arg->apply(*this);
      BExpRef x = mkfun(opn,res);
      res = x;
    }
    virtual void operator()(BinaryOp* p) {
      p->arg1->apply(*this);
      BExpRef LHS = res;
      p->arg2->apply(*this);
      BExpRef RHS = res;
      switch(p->op) {
      case '^': {
	if (!RHS->isint()) { throw BExpException("Only integer literals allowed in exponents!"); }
	int k = RHS->num()->val();
	res = mkexp(LHS,k);
	break; }// CHRIS CHECK!
      case '-': res = mkfun(opa,LHS,mkfun(opn,RHS)); break; 
      case '+': res = mkfun(opa,LHS,RHS); break; 
      case '*': res = mkfun(opm,LHS,RHS); break;
      case '/': res = mkfun(opdiv,LHS,RHS); break;
      default: throw BExpException(std::string("Unkown op '") + p->op + "' in ToBExp!");
      }
    }
    virtual void operator()(TarskiAtom* p)    {
      p->LHS->apply(*this);
      BExpRef LHS = res;
      p->RHS->apply(*this);
      BExpRef RHS = res;
      res = mkfun(p->relop,LHS,RHS);
    }
    virtual void operator()(ExtTarskiAtom* p) {
      throw BExpException("Indexed root expressions not yet implemented in ToBExp!");
      p->LHS->apply(*this); p->rootIndex->apply(*this); p->RHS->apply(*this);
    }
    virtual void operator()(BinaryLogOp* p)   {
      p->LHS->apply(*this);
      BExpRef LHS = res;
      p->RHS->apply(*this);
      BExpRef RHS = res;
      res = mkfun(p->logop,LHS,RHS);
    }
    virtual void operator()(NotOp* p)         {
      p->arg->apply(*this);
      BExpRef arg = res;
      res = mkfun(NOTOP,arg);
    }  
    virtual void operator()(QuantifierBlock *p);
  };
  void ToBExp::operator()(QuantifierBlock *p) 
  {
    vector<BExpRef> tmp;
    for(unsigned int i = 0; i < p->vars.size(); ++i) {
      p->vars[i]->apply(*this);
      tmp.push_back(res);
    }
    p->arg->apply(*this);
    tmp.push_back(res);
    res = mkfun(p->kind,tmp);
  }

  BExpRef toBExp(TarskiRef T, PolyManager* pM) {
    ToBExp TBE;
    T->apply(TBE);
    BExpRef F = TBE.res;
    return F;
  }
  


  class HasNonConstDenom : public Traverse {
  public:
    class NCDFound : public std::exception { };
    void operator()(BinaryOp* p) {
      if (p->op == '/' && !asa<Num>(p->arg2)) {
	throw NCDFound();
      }
      p->arg1->apply(*this); p->arg2->apply(*this);
    }
  };

  bool hasNonConstDenom(TarskiRef T, PolyManager* pM) {
    HasNonConstDenom HNCD;
    try { T->apply(HNCD); return false; }
    catch(HasNonConstDenom::NCDFound &e) {
      return true;
    }
  }

}
