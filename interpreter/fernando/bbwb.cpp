#include "bbwb.h"
#include "blackbox-solve.h"
#include "whitebox-solve.h"
#include "solver-manager.h"

namespace tarski {


  vector<string> BBWBComm::options = {"verbose"};

  TAndRef BBWBComm::preProcess(std::vector<SRef> &args, optionsHandler& o) {
      int N = args.size();
      if (N < 1) {throw TarskiException("bbwb requires 1 arguement");}
      TarRef tarRef = args[N-1]->tar();
      if (tarRef.is_null()) {throw TarskiException("bbwb argument not a Tarski formula");}
      TFormRef tRef = tarRef->getValue();
      TAndRef A = asa<TAndObj>(tRef);
      if (A.is_null()) {
        throw TarskiException("ERROR - NOT A CONJUNCTION");
      }
      else if (A->size() == 0) {
        throw TarskiException("ERROR - EMTPY CONJUNCTION");
      }
      o.loadOptions(args);
      return A;
    }

  
  SRef BBWBComm::execute(SRef input, std::vector<SRef> &args) {
    TAndRef A;
    optionsHandler o(options);
    try {
      A = preProcess(args, o);
    }
    catch (TarskiException t) {
      return new ErrObj(t.what());
    }

    vector<QuickSolver *> solvers;
    solvers.push_back(new WBSolver(A));
    solvers.push_back(new BBSolver(A));
    SolverManager s(solvers, A);
    s.deduceAll();
    LisRef l = s.genLisResult();
    if (o.getOpt(0)) s.prettyPrintResult();
    return l;
  }

}//namespace tarski
