#ifndef QEP_SOLVER_COMM_H
#define QEP_SOLVER_COMM_H

#include "../shell/einterpreter.h"

namespace tarski {

  class QepSolverComm : public EICommand {
  public:
    QepSolverComm(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, std::vector<SRef> &args);
    std::string testArgs(std::vector<SRef> &args) { return ""; }
    std::string doc() { return "This command is testing the SMT solver without bb/wb"; }
    std::string usage() { return "(qep-solve <Formula>)"; }
    std::string name() { return "qep-solve"; }

  };


}//end namespace


#endif
