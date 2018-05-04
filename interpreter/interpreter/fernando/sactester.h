#ifndef _SAC_TESTER_H
#define _SAC_TESTER_H

#include "../shell/einterpreter.h"
#include "fern-poly-iter.h"
#include "../formula/formula.h"
#include "mono-explain.h"
#include "poly-explain.h"
#include "deduce-sign.h"
#include <queue>



namespace tarski {
  class SacTesterComm : public EICommand
  {
  public:
  SacTesterComm(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, std::vector<SRef> &args);
    std::string testArgs(std::vector<SRef> &args) { return ""; }
    std::string doc() { return "This command is used to test several data structures and methods I've made, including fern-poly-iters, mono-sign and poly-sign, and findIntervalsExplain and deduceSignExplain\n"; }
    std::string usage() { return "(sac-test <Formula>)"; }
    std::string name() { return "sac-test"; }
  };

}


#endif
