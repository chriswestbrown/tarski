#ifndef _QEPCAD_INTER_
#define _QEPCAD_INTER_

#include <fstream>
#include <cstdlib>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include "../../poly/caserver/unnamedpipe.h"
#include "../../tarskisysdep.h"
#include "../../formula/writeForQE.h"
#include "../einterpreter.h"
#include "qepcad-session.h"

namespace tarski {

  // qepcad-qe
class CommQepcadQE : public EICommand
{
public:
  CommQepcadQE(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  { 
    TFormRef F = args[0]->tar()->val;
    TFormRef A = args.size() == 2 ? args[1]->tar()->val : TFormRef(new TConstObj(TRUE));
    TFormRef res;
    QepcadConnection qconn;
    try {
      res = qconn.basicQepcadCall(F,A);
      return new TarObj(res);
    } catch(TarskiException &e) { return new ErrObj(e.what()); }
  }

  string testArgs(vector<SRef> &args)
  {
    string s1 = require(args,_tar,_tar);
    return s1 == "" ? s1 : require(args,_tar);
  }
  string doc() 
  {
    return "(qepcad-qe F) returns the result of calling QEPCADB on input formula F.  With the optional second argument A, where A is a tarski formula in the free variables of F, A is passed to QEPCADB as \"assumptions\".";
  }
  string usage() { return "(qepcad-qe <input-formula> <assumptions>) or (qepcad-qe <input-formula>)"; }
  string name() { return "qepcad-qe"; }
};


// qepcad-sat
class CommQepcadSat : public EICommand
{
public:
  CommQepcadSat(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  {
    SRef output = new ErrObj("Failure in qepcad-sat!");
      
    TFormRef F = args[0]->tar()->val;
    // TODO!! check that F is quantifier free
    TAndRef Fp = new TAndObj();
    Fp->AND(F);
    bool conjunctionCase = isConjunctionOfAtoms(Fp);
    if (conjunctionCase)
    {
      TFormRef res;
      QepcadConnection qconn;
      try {
	res = qconn.basicQepcadCall(exclose(F),true);
	if (res->constValue() == TRUE) {
	  std::string w = qconn.samplePointPrettyPrint(getPolyManagerPtr());
	  output = new LisObj(new SymObj("SAT"),new StrObj(w));
	}
	else if (res->constValue() == FALSE) {
	  std::vector<TFormRef> M;
	  for(TAndObj::conjunct_iterator i = Fp->begin(); i != Fp->end(); ++i)
	    M.push_back(*i);
	  TAndRef core = new TAndObj();
	  vector<int> T = qconn.getUnsatCore();
	  for(int i = 0; i < T.size(); i++)
	    core->AND(M[T[i]]);	  
	  output = new LisObj(new SymObj("UNSAT"),new TarObj(core));
	}
	else { throw TarskiException("Expected either TRUE or FALSE returned from QepcadB!"); }
      } catch(TarskiException &e) { return new ErrObj(e.what()); }
    }
    else // not a conjunction!
    {
      TFormRef res;
      QepcadConnection qconn;
      try {
	res = qconn.basicQepcadCall(exclose(F),false);
	if (res->constValue() == TRUE) {
	  std::string w = qconn.samplePointPrettyPrint(getPolyManagerPtr());
	  output = new LisObj(new SymObj("SAT"),new StrObj(w));
	}
	else if (res->constValue() == FALSE) {
	  output = new LisObj(new SymObj("UNSAT"), new LisObj());
	}
	else { throw TarskiException("Expected either TRUE or FALSE returned from QepcadB!"); }
      } catch(TarskiException &e) { return new ErrObj(e.what()); }
    }
    return output;
  }

  string testArgs(vector<SRef> &args)
  {
    return require(args,_tar);
  }
  string doc() 
  {
    return "(qepcad-sat F) returns the result of calling QEPCADB on the existential closure of unquantified input formula F.  Specifically, a pair (r,s) is returned, where r is either the symbol SAT or UNSAT - according to whether the formula is SAT or UNSAT.  If r is SAT, the second component, s, is a string describing a solution point.  If r is UNSAT, s is an UNSAT core if F is a conjunction, and '() otherwise. ";
  }
  string usage() { return "(qepcad-sat <input-formula>)"; }
  string name() { return "qepcad-sat"; }
};

}//end namespace tarski

#endif
