#include "bbwb.h"

namespace tarski {
  SRef BBWBComm::execute(SRef input, std::vector<SRef> &args) {
    int N = args.size();
    if (N < 1) {return new ErrObj("bbwb requires 1 arguement");}

    TarRef tarRef = args[N-1]->tar();
    if (tarRef.is_null()) {return new StrObj("bbwb argument not a Tarski formula");}

    TFormRef tRef = tarRef->getValue();
    TAndRef A = asa<TAndObj>(tRef);
    if (A.is_null()) {
      return new StrObj("ERROR - NOT A CONJUNCTION");
    }
    else if (A->size() == 0) {
      return new StrObj("ERROR - EMTPY CONJUNCTION");
    }

    PolyManager * PM = A->getPolyManagerPtr();

    Boxer * b = new Boxer(A);
    Result res = b->deduceAll();

    std::cout << "##################################################" << std::endl;
    if (b->isUnsat()) {
      std::cout << "UNSAT\n";
      std::cout << "Explanation: "; res.write(); std::cout << std::endl << std::endl;
      std::cout << "Proof: \n";
      b->writeProof();
      std::cout << std::endl;
      std::cout << "All Deductions: \n";
      b->writeAll();
    }
    else {
      std::cout << "BBWBSAT\n";
      std::cout << std::endl;
      std::cout << "All Deductions: \n";
      b->writeAll();
    }
    std::cout << "##################################################" << std::endl;
    delete b;
    return tarRef;


  }

}//namespace tarski
