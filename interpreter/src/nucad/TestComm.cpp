#include "TestComm.h"

using namespace tarski;

VarOrderRef TestComm::argToVarOrder(SRef obj, PolyManager* PMptr)
  {
    LisRef Lv = obj->lis();
    if (Lv.is_null())
      throw TarskiException("Object of non-list type cannot be a VarOrder!");
    VarOrderRef V = new VarOrderObj(PMptr);
    for(int i = 0; i < Lv->length(); ++i)
    {
      SymRef s = Lv->get(i)->sym();
      if (Lv.is_null())
	throw TarskiException("Object of non-sym type cannot be an element of a VarOrder!");
      V->push_back(s->val);
    }
    return V;
  }

  GCWord TestComm::argToRationalPoint(SRef obj)
  {
    LisRef La = obj->lis();
    if (La.is_null())
      throw TarskiException("Object of non-list type cannot be a rational point!");
    Word A = NIL;
    for(int i = La->length() - 1; i >= 0; --i)
    {
      NumRef x = La->get(i)->num();
      if (x.is_null())
	throw TarskiException("Object of non-num type cannot be a rational coordinate!");
      A = COMP(x->val,A);
    }
    return A;
  }

  SRef TestComm::execute(SRef input, vector<SRef> &args) 
  {
    SymRef sym = args[0]->sym();
    AlgRef alg = args[1]->alg();
    VarOrderRef V = argToVarOrder(args[2],getPolyManagerPtr());
    SamplePointManagerRef SM = new TestSamplePointManagerObj(V);
    //SamplePointManagerRef SM = new TreeSamplePointManagerObj(V);
    GCWord alpha = argToRationalPoint(args[3]);
    SamplePointId sid = SM->addRationalOver(0,alpha,0);
    TermContextRef TC = new SimpleTermContextObj();

    NuCADCellRef c = new NuCADCellMOD(SM,sid,false,TC,NULL);
    NuCADCellTypeRef ct = new NuCADCellTypeObj(c);    
    ct->refine(args);
    
    return new ExtObj(ct);
  }

