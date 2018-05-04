#include "wbsat-tester.h"

namespace tarski {
  SRef WBTesterComm::execute(SRef input, std::vector<SRef> &args) {
    int N = args.size();
    if (N < 1) {return new ErrObj("wbsat-test requires one argument."); }

    TarRef tarRef = args[N-1]->tar();
    if (tarRef.is_null()) {return new StrObj("wbsat-test argument not a Tarski formula");}

    TFormRef F = tarRef->getValue();
    PolyManager &PM = *(F->getPolyManagerPtr());
    TAndRef C = asa<TAndObj>(F);
    if (C.is_null()) {
      return new StrObj("ERROR - NOT A CONJUNCTION");
    }
    else if (C->size() == 0) {
      return new StrObj("ERROR - EMPTY CONJUNCTION");
    }

    WBManager * wb = new WBManager(C);
    Result res =  wb->deduceAll();
    std::cout << std::endl << std::endl;
    std::cout << "##################################################" << std::endl;
    if (wb->isUnsat()) {
      std::cout << "UNSAT\n";
      std::cout << "Explanation: "; res.write(); std::cout << std::endl << std::endl;
      std::cout << "Proof: \n";
      wb->writeProof();
      std::cout << std::endl;
      std::cout << "All Deductions: \n";
      wb->writeAll();
    }
    else {
      std::cout << "WBSAT\n";
      std::cout << std::endl;
      std::cout << "All Deducitons: \n";
      wb->writeAll();
    }
    std::cout << "##################################################" << std::endl;

    delete wb;
    return tarRef;
  }

}



