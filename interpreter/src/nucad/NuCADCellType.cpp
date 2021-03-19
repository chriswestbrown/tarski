#include "NuCADCellType.h"
#include "collectivetermbase.h"

namespace tarski {
TypeExtensionObj::AMsgManager<NuCADCellTypeObj> NuCADCellTypeObj::_manager = TypeExtensionObj::AMsgManager<NuCADCellTypeObj>();

TypeExtensionObj::LFH<NuCADCellTypeObj> NuCADCellTypeObj::A[] = {
  {"refine",&NuCADCellTypeObj::refine,"To appear"},
  {"refine-to-child",&NuCADCellTypeObj::refineToChild,"To appear"},
  {"tst",&NuCADCellTypeObj::tst,"To appear"},
  {"sign-at-sample",&NuCADCellTypeObj::signAtSample,"To appear"},
  {"check-base",&NuCADCellTypeObj::checkBase,"To appear"},
  {"dump",&NuCADCellTypeObj::dump,"To appear"},
  {0,0,"function does not exist"}
 };


  SRef NuCADCellTypeObj::refine(std::vector<SRef>& args)
  {
    SymRef sym = args[0]->sym();
    AlgRef alg = args[1]->alg();
    IntPolyRef H = alg->getVal();
    FactRef f = makeFactor(*(cell->getPolyManager()),H);
    TermRef initialTerm;
    if (f->numFactors() == 1 && f->getContent() == 1 && f->factorBegin()->second == 1)      
      initialTerm = new Property(prop::nameToProp(sym->getVal()), new PolyTerm(f->factorBegin()->first));
    else
      initialTerm = new Property(prop::nameToProp(sym->getVal()), new FactorTerm(f));
    int N = initialTerm->level(cell->getVarOrder());
    std::vector<TermRef> goals(std::vector<TermRef>{ initialTerm });

    cell->transferCellBoundsToGoals(N,goals);
    cell->refineHelp(goals);
    return new SObj();
  }

  SRef NuCADCellTypeObj::refineToChild(std::vector<SRef>& args)
  {
    SymRef sym = args[0]->sym();
    AlgRef alg = args[1]->alg();
    IntPolyRef H = alg->getVal();
    FactRef f = makeFactor(*(cell->getPolyManager()),H);
    TermRef initialTerm;
    if (f->numFactors() == 1 && f->getContent() == 1 && f->factorBegin()->second == 1)      
      initialTerm = new Property(prop::nameToProp(sym->getVal()), new PolyTerm(f->factorBegin()->first));
    else
      initialTerm = new Property(prop::nameToProp(sym->getVal()), new FactorTerm(f));
    int N = initialTerm->level(cell->getVarOrder());
    std::vector<TermRef> goals(std::vector<TermRef>{ initialTerm });

    NuCADCellTypeRef ccell = new NuCADCellTypeObj(cell->refineToChild(goals));
    return new ExtObj(ccell);
  }

  SRef NuCADCellTypeObj::tst(std::vector<SRef>& args)
  {
    SymRef sym = args[0]->sym();
    AlgRef alg = args[1]->alg();
    IntPolyRef H = alg->getVal();
    FactRef f = makeFactor(*(cell->getPolyManager()),H);
    TermRef initialTerm;
    if (f->numFactors() == 1 && f->getContent() == 1 && f->factorBegin()->second == 1)      
      initialTerm = new Property(prop::nameToProp(sym->getVal()), new PolyTerm(f->factorBegin()->first));
    else
      initialTerm = new Property(prop::nameToProp(sym->getVal()), new FactorTerm(f));
    int N = initialTerm->level(cell->getVarOrder());
    std::vector<TermRef> goals(std::vector<TermRef>{ initialTerm });

    NuCADCellRef child = cell->refineToChild(goals);
    std::vector<NuCADCellRef> V = cell->split(child);
    LisRef res = new LisObj();
    for(auto itr = V.begin(); itr != V.end(); ++itr)
      res->push_back(new ExtObj(new NuCADCellTypeObj(*itr)));
    return res;
  }

  SRef NuCADCellTypeObj::signAtSample(std::vector<SRef>& args)
  {
    AlgRef alg = args[0]->alg();
    IntPolyRef H = alg->getVal();
    FactRef f = makeFactor(*(cell->getPolyManager()),H);
    int s = cell->getSamplePointManager()->factorSignAt(cell->getSamplePointId(),f);
    return new NumObj(s);
  }

  SRef NuCADCellTypeObj::checkBase(std::vector<SRef>& args)
  {
    LisRef L = args[0]->lis();
    SymRef prop = L->get(0)->sym();
    AlgRef poly = L->get(1)->alg();
    FactRef f = makeFactor(*(cell->getPolyManager()),poly->getVal());
    if (f->numFactors() != 1) { throw TarskiException("check-base should be called on an irreducible polynomial"); }
    auto itr = f->factorBegin();
    if (itr->second != 1) { throw TarskiException("check-base should be called on an irreducible polynomial"); }
    IntPolyRef p = itr->first;
    TermRef T = new Property(prop::nameToProp(prop->getVal()), new PolyTerm(p));
    TermId tid = cell->getTermContext()->add(T);
    int res = cell->checkStatus(tid);
    return new BooObj(res);
  }

  SRef NuCADCellTypeObj::dump(std::vector<SRef>& args)
  {
    cell->getCollectiveTermBase()->dump(cell->getPolyManager(),cell->getTermContext());
    return new SObj();
  }
}

