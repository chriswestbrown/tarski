#include "qep-solver-comm.h"
#include "qep-solver.h"
#include "../minisat/core/Solver.h"
namespace tarski{

  SRef QepSolverComm::execute(SRef input, std::vector<SRef> &args) {

    Minisat::lbool l_true = Minisat::lbool((uint8_t)0);
    Minisat::lbool l_false = Minisat::lbool((uint8_t)1);
    Minisat::lbool l_undef = Minisat::lbool((uint8_t)2);

    int N = args.size();
    if (N < 1) {return new ErrObj("Qep-Solver requires 1 argument");}

    TarRef tarRef = args[N-1]->tar();
    if (tarRef.is_null()) {return new StrObj("Qep-Solver argument not a Tarski formula");}
    
    
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
    Minisat::QEPSolver * q = new Minisat::QEPSolver(tRef);
    /*
    q.printMapping();
    cout << "----------------------------------------------\n";
    q.printCNF(tRef);
    cout << "----------------------------------------------\n";
    */
    string err;
    bool res = q->solve(err);
    if (err.size() == 0) {
      return new StrObj((res == true) ? "SATISFIABLE" : "UNSAT");
    }
    else {
      return new ErrObj(err);
    }

  }

}
