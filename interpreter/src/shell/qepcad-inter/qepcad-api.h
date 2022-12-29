#ifndef _QEPCAD_API_SESSION_
#define _QEPCAD_API_SESSION_

#include "einterpreter.h"
#include "../../../qesource/source/qepcad.h"

namespace tarski {

  /*
    input: A string that is a valid qepcad input script
    formType: char T for Tarski, E for Extended G for Geometric
  */
  SRef qepcadAPICall(std::string &input, char formType='T');

  
  class QepcadAPICallback {
  public:
    virtual SRef operator()(QepcadCls &Q);
  };
  SRef qepcadAPICall(std::string &input, QepcadAPICallback &f);
  

    
  class CommQepcadAPICall : public EICommand
  {
  public:
    CommQepcadAPICall(NewEInterpreter* ptr) : EICommand(ptr) { }

    SRef execute(SRef input, /*
			       input: A string that is a valid qepcad input script
			       formType: char T for Tarski, E for Extended G for Geometric
			     */
		 vector<SRef> &args);

    string testArgs(vector<SRef> &args)
    {
      return require(args,_tar) == "" ? "" :  require(args,_tar,_sym);
    }

    string doc() 
    {
      return "(qepcad-api-call F) -or- (qepcad-api-call F S), where F is a Tarski formula and S is either 'T or 'E. Returns the formula that results from calling QEPCADB via an API call rather than by forking a QEPCADB process.  By default the result is in the language of Extended Tarski Formulas (which can be stated explicitly with the 'E option), but with the 'T option a standard tarski formula is produced.";
    }

    string usage() { return "(qepcad-api-call <tarski formula> ['T | 'E])"; }

    string name() { return "qepcad-api-call"; }
  };


}

#endif
