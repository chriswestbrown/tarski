#ifndef _WB_TESTER_H
#define _WB_TESTER_H

#include "whitebox-solve.h"
#include "bbwb.h"
#include "solver-manager.h"
#include "whitebox-solve.h"
#include "../shell/einterpreter.h"
#include "../formula/formula.h"


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

