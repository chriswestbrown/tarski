#ifndef BOX_SOLVE_COMM_H
#define BOX_SOLVE_COMM_H

#include "../shell/einterpreter.h"


namespace tarski {

  class BoxSolverComm : public EICommand
  {
  public:
  BoxSolverComm(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, std::vector<SRef> &args);
    std::string testArgs(std::vector<SRef> &args) { return ""; }
    std::string doc() { return "This command is testing the SMT solver with bb/wb. It requires level 1 normalization to work properly"; }
    std::string usage() { return "(box-solve (normalize 'level 1<Formula>))"; }
    std::string name() { return "box-solve"; }

  };


}//end namespace

#endif
