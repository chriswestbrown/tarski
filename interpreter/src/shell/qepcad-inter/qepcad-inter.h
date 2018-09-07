#ifndef _QEPCAD_INTER_
#define _QEPCAD_INTER_

#include <fstream>
#include <cstdlib>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <chrono>
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
    using namespace std::chrono;
    bool time = false;
    SRef output = new ErrObj("Failure in qepcad-sat!");
    int N = args.size();
    TFormRef F = args[N-1]->tar()->val;
    if (N == 2) {
      SymRef opt = args[0]->sym();
      if (opt.is_null()) return new ErrObj("Not a symbol for arg");
      if (opt->val == "time") {
        time = true;
      }
      else return new ErrObj("Symbol not understood");
    }
    // TODO!! check that F is quantifier free
    TAndRef Fp = new TAndObj();
    Fp->AND(F);
    bool conjunctionCase = isConjunctionOfAtoms(Fp);
    if (conjunctionCase)
      {
        TFormRef res;
        QepcadConnection qconn;
        try {
          high_resolution_clock::time_point t1 = high_resolution_clock::now();
          Normalizer* p = new Level1();
          RawNormalizer R(*p);
          R(F);
          TFormRef F2 = R.getRes();
          if (F2->getTFType() != TF_CONST)
            res = qconn.basicQepcadCall(exclose(F2),true);
          high_resolution_clock::time_point t2 = high_resolution_clock::now();
          if (time) {
            duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
            std::ostringstream strs;
            strs << std::fixed << std::setprecision(16) << time_span.count();
            std::string str = strs.str();
            LisRef l = new LisObj();
            if (F2->getTFType() == TF_CONST) {
              if (F2->constValue() == TRUE)
                l->push_back(new StrObj("SATISFIABLE"));
              else
                l->push_back(new StrObj("UNSAT"));
            }
            else if (res->constValue() == TRUE)
              l->push_back(new StrObj("SATISFIABLE"));
            else
              l->push_back(new StrObj("UNSAT"));
            l->push_back(new StrObj(str));
            return l;
          }
          if (F2->getTFType() == TF_CONST) {
            return (F2->constValue() == TRUE)
              ? new SymObj("SATISFIABLE BY L1 NORMALIZATION")
              : new SymObj("UNSAT BY L1 NORMALIZATION");
          }
          else if (res->constValue() == TRUE) {
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
        } catch(TarskiException &e) {
          return new ErrObj(e.what());
        }
      }
    else // not a conjunction!
      {
        TFormRef res;
        QepcadConnection qconn;
        try {
          high_resolution_clock::time_point t1 = high_resolution_clock::now();
          res = qconn.basicQepcadCall(exclose(F),false);
          high_resolution_clock::time_point t2 = high_resolution_clock::now();
          if (time) {
            duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
            std::ostringstream strs;
            strs << std::fixed << std::setprecision(16) << time_span.count();
            std::string str = strs.str();
            LisRef l = new LisObj();
            if (res->constValue() == TRUE)
              l->push_back(new StrObj("SATISFIABLE"));
            else
              l->push_back(new StrObj("UNSAT"));
            l->push_back(new StrObj(str));
            return l;
          }
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
    //return require(args,_tar);
    return "";
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
