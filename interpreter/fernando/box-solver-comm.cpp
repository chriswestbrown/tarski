#include "box-solver-comm.h"
#include "box-solver.h"
#include "../minisat/core/Solver.h"
#include <stdio.h>
#include <string>

namespace tarski {
  SRef BoxSolverComm::execute(SRef input, std::vector<SRef> &args) {

    int N = args.size();
    if (N < 1) {return new ErrObj("BoxSolver requires 1 argument");}

    TarRef tarRef = args[N-1]->tar();
    if (tarRef.is_null()) {return new StrObj("BoxSolver argument not a Tarski formula");}
    
    
    TFormRef tRef = tarRef->getValue();
    if (tRef.is_null()) {
      return new ErrObj("No formula given");
    }
    TAndRef andRef = asa<TAndObj>(tRef);
    if (andRef.is_null()) {
      TOrRef orRef = asa<TOrObj>(tRef);
      if (orRef.is_null()) {
        return new ErrObj("Formula trivially solvable");
      }
    }
    Minisat::BoxSolver b(tRef);

    //b.printMapping();
    //cout << "----------------------------------------------\n";
    //b.printCNF(tRef);
    //cout << "----------------------------------------------\n";

    string err;
    bool res = b.solve(err);
    if (err.size() == 0) {
      return new StrObj((res == true) ? "SATISFIABLE" : "UNSAT");
    }
    else {
      return new ErrObj(err);
    }


  }


}//end namespace
