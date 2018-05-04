#ifndef _WB_TESTER_H
#define _WB_TESTER_H


#include "../shell/einterpreter.h"
#include "wbsat.h"
#include "../formula/formula.h"


namespace tarski {
  class WBTesterComm : public EICommand
  {
  public:
  WBTesterComm(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, std::vector<SRef> &args);
    std::string testArgs(std::vector<SRef> &args) { return ""; }
    std::string doc() { return "This command is used to do WhiteBox with explanations exclusively on a given conjunction of inequalities"; }
    std::string usage() { return "(wbsat-test <Formula>)"; }
    std::string name() { return "wbsat-test"; }
  };

}

#endif

