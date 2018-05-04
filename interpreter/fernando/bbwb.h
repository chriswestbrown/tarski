#ifndef _BBWB_H
#define _BBWB_H

#include "../shell/einterpreter.h"
#include "../formula/monomialinequality.h"
#include "../formula/formula.h"
#include "boxer.h"

namespace tarski {
  class BBWBComm: public EICommand {
  private:

  public:

  BBWBComm(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, std::vector<SRef> &args);
    std::string testArgs(std::vector<SRef> &args) {return "";}
    std::string doc() { return "This command evaluates a formula and calls BlackBox/WhiteBox on it repetetively. If this works, Fernando will be very happy. Note that it requires level 1 normalization before usage." ; }
    std::string usage() { return "(bbwb (normalize 'level 1 <Formula>))" ;}
    std::string name() { return "bbwb"; }
  };



}//end namespace tarski








#endif
