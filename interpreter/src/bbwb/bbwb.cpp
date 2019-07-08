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
      TAtomRef atom = asa<TAtomObj>(tRef);
      if (A.is_null() && atom.is_null()) {
        throw TarskiException("ERROR - bbwb requires conjunction of atoms");
      }
      else if (!A.is_null() && A->size() == 0) {
        throw TarskiException("ERROR - EMTPY CONJUNCTION");
      }
      if (A.is_null()) {
	A = new TAndObj();
	A->AND(atom); }
      else {
	for (TAndObj::conjunct_iterator itr = A->conjuncts.begin(); itr != A->conjuncts.end(); ++itr) {
	  TAtomRef tf = asa<TAtomObj>(*itr);
	  if (tf.is_null()) throw TarskiException("ERROR - bbwb requires strict conjunction");
	}
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

     cerr << endl << "after deduceAll:" << endl;
     s.debugWriteSorted();
     cerr << endl << endl;
     s.prettyPrintResult();

    if (!s.isUnsat())
    {
       cerr << endl << "after call to simplify:" << endl;
      TAndRef Ap = s.simplify();
       cerr << "SIMPLIFIED: "; Ap->write(true); cerr << endl;
       s.prettyPrintSimplify(Ap);
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
    Level1 * p = new Level1();
    RawNormalizer R(*p);
    R(A);
    LisRef l;
    if (R.getRes()->getTFType() == TF_CONST) {
      if (R.getRes()->constValue() == FALSE) {
	l = new LisObj(new SymObj("UNSAT"), new TarObj(p->getUnsatAtom()));
      }
      else
	l = new LisObj(new SymObj("SAT"), new TarObj(new TConstObj(true)));
    }
    delete p;
    if (l.is_null()) {
      /*
	Solver Manager requires the formula to be a TAnd.
	So if R.getRes() is an atom, we need to wrap it in a 
	TAnd.
      */
      TAndRef res = R.getRes();
      if (res.is_null()) { res = new TAndObj(); res->AND(R.getRes()); }
      
      
      SolverManager s( SolverManager::BB |
		       SolverManager::WB |
		       SolverManager::SS,  res);
      l = s.genLisResult();
      if (o.getOpt(0)) s.prettyPrintResult();
    }
    return l;
  }

}//namespace tarski
