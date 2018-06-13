#ifndef _BBWB_H
#define _BBWB_H

#include "../shell/einterpreter.h"
#include "../formula/monomialinequality.h"
#include "../formula/formula.h"
#include "boxer.h"


namespace tarski {

  class optionsHandler {

  protected:
    std::vector<string> optNames;
    std::vector<bool> opts;

  public:
  optionsHandler(vector<string> vs)
    : optNames(vs), opts(vs.size(), false) {}

    
    bool getOpt(int idx) {
      if (idx < 0 || idx >= opts.size()) throw TarskiException("Options index out of bounds");
      return opts[idx];
    }
    
    void loadOptions(std::vector<SRef> &args) {
      int N = args.size();
      for (int i = 0; i < N-1; i++) {
	SymRef opt = args[i]->sym();
	if (!opt.is_null()) {
	  bool noMatch = true;
	  for (int i = 0; i < optNames.size(); i++) {
	    if (opt->val == optNames[i]) {
	      opts[i] = true;
	      noMatch = false;
	      break;
	    }
	  }
	  if (noMatch) throw TarskiException("optional argument '" +
					     args[i]->toStr() +"' not understood.");

	}
	else throw TarskiException("optional argument '" +
				   args[i]->toStr() +"' not understood.");
      }
    }
  };

  
  class BBWBComm: public EICommand {

  protected:
    static vector<string> options;
    virtual TAndRef preProcess(std::vector<SRef> &args, optionsHandler& o);
    
  public:
    
  BBWBComm(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, std::vector<SRef> &args);
    std::string testArgs(std::vector<SRef> &args) {return "";}
    std::string doc() { return "This command evaluates a formula and calls BlackBox/WhiteBox on it repetetively. Note that it requires level 1 normalization before usage. Optionally, give the 'verbose option in order to see more detailed output as well as a proof." ; }
    std::string usage() { return "(bbwb (normalize 'level 1 <Formula>))" ;}
    std::string name() { return "bbwb"; }
  };



}//end namespace tarski








#endif
