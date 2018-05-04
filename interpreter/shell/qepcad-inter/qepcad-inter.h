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

}//end namespace tarski

#endif
