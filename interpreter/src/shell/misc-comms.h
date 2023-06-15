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

  
}

#endif
