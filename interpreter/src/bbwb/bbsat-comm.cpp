#include "bbsat-comm.h"
#include "deduction.h"
#include "solver-manager.h"
#include "blackbox-solve.h"



namespace tarski {
  

  SRef BBSatComm::execute(SRef input, std::vector<SRef> &args)
  {
    
    TAndRef A;
    optionsHandler o(options);
    try {
      A = preProcess(args, o);
    }
    catch (TarskiException t) {
      return new ErrObj(t.what());
    }
    SolverManager s( SolverManager::BB, A);
    LisRef l = s.genLisResult();
    if (o.getOpt(0)) s.prettyPrintResult();
    return l;
  }


}//end namespace
