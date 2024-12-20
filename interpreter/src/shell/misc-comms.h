#ifndef _CWB_MISC_COMMS_
#define _CWB_MISC_COMMS_

#include "einterpreter.h"

using namespace std;

namespace tarski {


  class CommGetFreeVars : public EICommand
  {
  public:
    CommGetFreeVars(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, vector<SRef> &args);
    string testArgs(vector<SRef> &args) { return ""; }
    string doc() 
    {
      return "Returns a list of all top-level symbols in the interpreter.";
    }
    string usage() { return "(get-free-vars)"; }
    string name() { return "get-free-vars"; }
  };

  class CommSolutionDimension : public EICommand
  {
  public:
    CommSolutionDimension(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, vector<SRef> &args);
    string testArgs(vector<SRef> &args) {
      return requirepre(args,_tar);
    }
    string doc() 
    {
      return "Returns the dimension of the solution set for input formula.";
    }
    string usage() { return "(solution-dimension T)"; }
    string name() { return "solution-dimension"; }
  };

  
  class CommDiscriminant : public EICommand
  {
  public:
    CommDiscriminant(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, vector<SRef> &args);
    string testArgs(vector<SRef> &args) {
      return requirepre(args,_alg,_sym);
    }
    string doc() 
    {
      return "Returns the discriminant of polynomial A with respect to variable x.";
    }
    string usage() { return "(discriminant A x)"; }
    string name() { return "discriminant"; }
  };

  
  class CommSubDiscSeq : public EICommand
  {
  public:
    CommSubDiscSeq(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, vector<SRef> &args);
    string testArgs(vector<SRef> &args) {
      return requirepre(args,_alg,_sym);
    }
    string doc() 
    {
      return "Returns the sequence of principal sub-discriminant coefficients of polynomial A with respect to variable x.";
    }
    string usage() { return "(sub-disc-seq A x)"; }
    string name() { return "sub-disc-seq"; }
  };

  class CommNullifySys : public EICommand
  {
  public:
    CommNullifySys(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, vector<SRef> &args);
    
    string testArgs(vector<SRef> &args) {
      return requirepre(args,_alg,_lis);
    }
    string doc() 
    {
      return "Returns formula F that is true exactly when all coefficients of P as a polynomial in v are zero.\
Example: ";
    }
    string usage() { return "(nullify-sys P v)"; }
    string name() { return "nullify-sys"; }
  };

  class CommClear : public EICommand
  {
  public:
    CommClear(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, vector<SRef> &args);
    string testArgs(vector<SRef> &args) { return ""; }
    string doc() 
    {
      return "Clears non-constant denominators in an uninterpreted formula to create a Tarski formula.  An \
optional second argument is a symbol indicating what process is to be used: 'fair for the process that \
preserves fair-SATness, 'noguard for clearing without guards, and 'naive for producing the conjunction of \
the division-free atom with p /= 0 for each denominator polynomial p.  The default behavior when no second \
argument is present is 'fair.";
    }
    string usage() { return "(clear F) or (clear F ['fair|'noguard|'naive]"; }
    string name() { return "clear"; }
  };


  
}
#endif
