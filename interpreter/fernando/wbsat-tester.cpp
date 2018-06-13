#include "wbsat-tester.h"

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
    
    WBManager * wb = new WBManager(A);
    LisRef l = wb->genLisResult();
    if (o.getOpt(0)) wb->prettyPrintResult();
    delete wb;
    return l;
  }

}



