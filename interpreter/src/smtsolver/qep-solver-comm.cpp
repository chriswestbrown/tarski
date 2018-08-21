#include "qep-solver-comm.h"
#include "qep-solver.h"
#include "../../../minisat/core/Solver.h"
#include <chrono>

namespace tarski{
  SRef QepSolverComm::execute(SRef input, std::vector<SRef> &args) {

    using namespace std::chrono;
    bool time = false;
    int N = args.size();
    if (N < 1) {return new ErrObj("Qep-Solver requires 1 argument");}

    if (N == 2) {
      SymRef opt = args[0]->sym();
      if (opt.is_null()) return new ErrObj("Not a symbol for arg");
      if (opt->val == "time") {
        time = true;
      }
      else return new ErrObj("Argument not understood");
    }

    TarRef tarRef = args[N-1]->tar();
    if (tarRef.is_null()) {return new StrObj("Qep-Solver argument not a Tarski formula");}
    
    
    TFormRef tRef = tarRef->getValue();
    if (tRef.is_null()) {
      return new ErrObj("No formula given");
    }

    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    QEPSolver q(tRef);

    /*
    q.printMapping();
    cout << "----------------------------------------------\n";
    q.printCNF(tRef);
    cout << "----------------------------------------------\n";
    */
    string err;
    bool res = q.solve(err);
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    if (err.size() == 0) {
      if (time) {
        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
        std::ostringstream strs;
        strs << std::fixed << std::setprecision(16) << time_span.count();
        std::string str = strs.str();
        return new StrObj(str);
      }
      return new StrObj((res == true) ? "SATISFIABLE" : "UNSAT");
    }
    else {
      return new ErrObj(err);
    }

  }

}
