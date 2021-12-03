#ifndef _QFR_COMM_H_
#define _QFR_COMM_H_

#include "qfr.h"
#include "../shell/einterpreter.h"
#include "../poly/variable.h"
#include "../poly/poly.h"
#include "../formula/formula.h"

namespace tarski {

TFormRef QformToRegularForm(TFormRef F, VarSet QVars)
{
  if (asa<TAndObj>(F))
  {
    TAndRef Fa = asa<TAndObj>(F);
    VarSet QVarsAppearing = /*Fa->QVars*/ QVars & Fa->getVars();
    if (QVarsAppearing.none()) 
      return Fa;
    else 
    {
      TQBObj *p = new TQBObj(QVarsAppearing,EXIST,Fa,Fa->getPolyManagerPtr());
      return p;
    }
  }
  else if (asa<TOrObj>(F))
  {
    TOrRef Fo = asa<TOrObj>(F);
    TOrRef p = new TOrObj();
    for(set<TFormRef>::iterator itr = Fo->disjuncts.begin(); itr != Fo->disjuncts.end(); ++itr)
      p->OR(QformToRegularForm(*itr,QVars));
    return p;
  }
  else
  { 
    VarSet QVarsAppearing = QVars & F->getVars();
    if (QVarsAppearing.none()) 
      return F;
    else 
    {
      TQBObj *p = new TQBObj(QVarsAppearing,EXIST,F,F->getPolyManagerPtr());
      return p;
    }
  }
}


  
  class CommQFR : public EICommand
  {
  public:
  CommQFR(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, vector<SRef> &args) 
    { 
      SRef res;
      {
	TFormRef T = args[0]->tar()->val;
	int searchType = 6;
	if (args.size() > 1 && !args[1]->num().is_null()) {
	  Word val = RNFLOR(args[1]->num()->getVal());
	  if (val < 1 || val > 6) { throw TarskiException("Optional argument to qfr must be number in {1,2,3,4,5}!"); }
	  searchType = val;
	}
	QFR qfr;
	try {
	  if (qfr.init(searchType,T,interp->PM)) { return new ErrObj("Bad input format for QFR."); }
	  qfr.rewrite();
	  TFormRef minpf = qfr.getBest();
	  TFormRef Tp = QformToRegularForm(minpf,qfr.getQuantifiedVariables());
	  res = new TarObj(Tp);
	  //qfr.printDerivation();
	}
	catch(TarskiException &e)
	{
	  res = new ErrObj(e.what());
	}
      } 
      return res;
    }
    string testArgs(vector<SRef> &args)
    {
      return require(args,_tar) == "" ? "" : require(args,_tar,_num);
    }
    string doc() 
    {
      return "(qfr F), where F is a Tarski Formula, returns\
a Tarski Formula resulting from applying qfr.";
    }
    string usage() { return "(qfr <tarski formula>)"; }
    string name() { return "qfr"; }
  };

}
#endif
