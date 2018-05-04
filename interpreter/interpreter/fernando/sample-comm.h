#ifndef _SAMPLE_COMM_H_
#define _SAMPLE_COMM_H_

#include "../shell/einterpreter.h"

namespace tarski {
  class SampleComm : public EICommand
  {
  public:
  SampleComm(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, std::vector<SRef> &args);
    std::string testArgs(std::vector<SRef> &args) { return ""; }
    std::string doc() { return "This is a smple command.  It just returns a std::string."; }
    std::string usage() { return "(sample-command)"; }
    std::string name() { return "sample-command"; }
  };
}
#endif
