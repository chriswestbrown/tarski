#ifndef _WB_TESTER_H
#define _WB_TESTER_H


#include "../shell/einterpreter.h"
#include "wbsat.h"
#include "../formula/formula.h"
#include "bbwb.h"

namespace tarski {
  class WBTesterComm : public BBWBComm
  {
  public:
  WBTesterComm(NewEInterpreter* ptr) : BBWBComm(ptr) { }
    SRef execute(SRef input, std::vector<SRef> &args);
    std::string testArgs(std::vector<SRef> &args) { return ""; }
    std::string doc() { return "This command is used to do WhiteBox with explanations exclusively on a given conjunction of inequalities"; }
    std::string usage() { return "(wbsat <Formula>)"; }
    std::string name() { return "wbsat"; }
  };

}

#endif

