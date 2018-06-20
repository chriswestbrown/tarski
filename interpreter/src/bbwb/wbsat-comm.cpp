#include "wbsat-comm.h"

namespace tarski {
  
  SRef WBTesterComm::execute(SRef input, std::vector<SRef> &args) {

    TAndRef A;
    optionsHandler o(options);
    try {
      A = preProcess(args, o);
    }
    catch (TarskiException t) {
      return new ErrObj(t.what());
    }
    SolverManager s({ new WBSolver(A) }, A);
    LisRef l = s.genLisResult();
    if (o.getOpt(0)) s.prettyPrintResult();
    return l;
  }

}



