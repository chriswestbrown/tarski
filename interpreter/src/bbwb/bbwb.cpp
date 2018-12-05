#include "bbwb.h"
#include "solver-manager.h"
#include "whitebox-solve.h"
#include "blackbox-solve.h"
#include "normalize.h"


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
      for (TAndObj::conjunct_iterator itr = A->conjuncts.begin(); itr != A->conjuncts.end(); ++itr) {
        TAtomRef tf = asa<TAtomObj>(*itr);
        if (tf.is_null()) throw TarskiException("ERROR - bbwb requires strict conjunction");
      }
      o.loadOptions(args);
      return A;
    }


  LisRef foobar(TAndRef A)
  {
    //L1 Normalization
    Normalizer * p = new Level1();
    RawNormalizer R(*p);
    R(A);
    delete p;
    if (R.getRes()->getTFType() == TF_CONST) {
      if (R.getRes()->constValue() == FALSE)
        throw TarskiException("UNSAT BY L1 NORMALIZATION");
      else
        throw TarskiException("SAT BY L1 NORMALIZATION");
    }

    SolverManager s( SolverManager::BB |
                     SolverManager::WB |
                     SolverManager::SS,  R.getRes());

    s.deduceAll();

    // cerr << endl << "after deduceAll:" << endl;
    // s.debugWriteSorted();
    // cerr << endl << endl;
    // s.prettyPrintResult();

    if (!s.isUnsat())
    {
      // cerr << endl << "after call to simplify:" << endl;
      TAndRef Ap = s.simplify();
      // cerr << "SIMPLIFIED: "; Ap->write(true); cerr << endl;
      // s.prettyPrintSimplify(Ap);
    }
    return new LisObj();
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
    
    if(false){ //-- Dr Brown debug
      LisRef l = foobar(A);
      return l;
    }
    
    //L1 Normalization
    Normalizer * p = new Level1();
    RawNormalizer R(*p);
    R(A);
    delete p;
    if (R.getRes()->getTFType() == TF_CONST) {
      if (R.getRes()->constValue() == FALSE)
        return new SymObj("UNSAT BY L1 NORMALIZATION");
      else
        return new SymObj("SAT BY L1 NORMALIZATION");
    }

    SolverManager s( SolverManager::BB |
                     SolverManager::WB |
                     SolverManager::SS,  R.getRes());
    LisRef l = s.genLisResult();
    if (o.getOpt(0)) s.prettyPrintResult();
    return l;
  }

}//namespace tarski
