#ifndef _TEST_COMM_
#define _TEST_COMM_
#include "property.h"
#include "goal.h"
#include "../poly/factor.h"
#include <algorithm>
//#include "rule.h"

namespace tarski {

/* class RuleGenStar */
/* { */
/*  public: */
/*   virtual apply(FactRef F, vector< pair<prop::Tag,IntPolyRef> > & */

/* }; */


class TestComm : public EICommand
{
public:
  TestComm(NewEInterpreter* ptr) : EICommand(ptr) { }

  VarOrderRef argToVarOrder(SRef obj, PolyManager* PMptr)
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

  GCWord argToRationalPoint(SRef obj)
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

  SRef execute(SRef input, vector<SRef> &args) 
  {
    SymRef sym = args[0]->sym();
    AlgRef alg = args[1]->alg();
    VarOrderRef V = argToVarOrder(args[2],getPolyManagerPtr());
    SamplePointManagerRef SM = new TestSamplePointManagerObj(V);
    GCWord alpha = argToRationalPoint(args[3]);
    SamplePointId sid = SM->addRationalOver(0,alpha,0);
    SM->debugDump(); 

    IntPolyRef H = alg->getVal();
    IntPolyRef p = getPolyManagerPtr()->getCannonicalCopy(H);
    int polySign = SM->polynomialSignAt(sid,p);
    std::cout << "polySign = " << polySign << std::endl;

    SM->roots(sid,p);
    
    GoalContextRef G = new GoalContextObj(SM,sid);

    
    prop::Tag prop = prop::nameToProp(sym->getVal());
    FactRef F = makeFactor(*(getPolyManagerPtr()),p,1);
    cerr << "property: " << prop::name(prop) << endl;

    GoalId gid;
    if (prop::isCheck(prop))
    {
      gid = G->addCheck(prop,p);
      int t_c = G->getStatus(gid);
      cerr << "check returned " << t_c << endl;
    }
    else if (prop::isNirGoal(prop))
    {
      gid = G->addNirGoal(prop,F); 
      int t_c = G->getStatus(gid);
      cerr << "NirGoal status is " << t_c << endl;
    }
    else
    {
      gid = G->addIrrGoal(prop,p);
      int t_c = G->getStatus(gid);
      cerr << "IrrGoal status is " << t_c << endl;
    }

    cerr << "gid = " << gid << endl;
    cerr << "goal: " << G->toString(gid) << endl;

    /* RuleRef r = generateRule(G,gid); */
    /* cerr << "rule: " << (r.is_null() ? "<none>" : r->toString()) << endl; */

    return new SObj();
  }
  string testArgs(vector<SRef> &args) { return ""; }
  string doc() { return "TO APPEAR"; }
  string usage() { return "TO APPEAR"; }
  string name() { return "test-comm"; }
};

}

#endif

// (test-comm 'level1 [ x - 1 ])
