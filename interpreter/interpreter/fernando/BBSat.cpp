#include "BBSat.h"
#include "deduction.h"



namespace tarski {
  

  SRef BBSatComm::execute(SRef input, std::vector<SRef> &args)
  {
    int N = args.size();
    if (N < 1) {return new ErrObj("bbsat requires 1 arguement");}

    TarRef tarRef = args[N-1]->tar();
    if (tarRef.is_null()) {return new StrObj("bbsat argument not a Tarski formula");}


    bool stats = false;

    for(int i = 0; i < N-1; i++)
      {
        SymRef opt = args[i]->sym();
        if (!opt.is_null() && opt->val == "stat" )
          stats = true;
        else
          return new ErrObj("smt-load optional argument '" + args[i]->toStr() +"' not understood.");
      }

    TFormRef tRef = tarRef->getValue();
    TAndRef F = asa<TAndObj>(tRef);
    PolyManager * PM = F->getPolyManagerPtr();
    std::vector<Deduction *> results = bbsat(F);
    if (results.size() == 0) {
      LisRef res = new LisObj();
      SymRef r1 = new SymObj("UNSAT");
      res->push_back(r1);
      return res;
    }
    else if (results[0]->isUnsat()) {
      LisRef res = new LisObj();
      SymRef r1 = new SymObj("UNSAT");
      res->push_back(r1);
      dedToLis(results[0], PM, res);
      return res;
    }
    else {
      LisRef res = new LisObj();
      SymRef r1 = new SymObj("SAT");
      res->push_back(r1);
      for (int i = 0; i < results.size(); i++) {
        dedToLis(results[i], PM, res);
      }
      return res;

    }

  }

  void BBSatComm::dedToLis(Deduction * bbded, PolyManager * PM, LisRef res) {

    TAndRef tandref = new TAndObj();

    const std::vector<TAtomRef> atoms = bbded->getDeps();
    for (std::vector<TAtomRef>::const_iterator aIter = atoms.begin(); aIter != atoms.end(); ++aIter) {
      tandref->AND(*aIter);
    }

    res->push_back(new TarObj(tandref));

  }

}//end namespace
