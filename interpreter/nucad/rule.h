#ifndef _RULE_
#define _RULE_

#include "goal.h"

class RuleObj;
typedef GC_Hand<RuleObj> RuleRef;

class ActionObj;
typedef GC_Hand<ActionObj> ActionRef;
class ActionObj : public GC_Obj { public:
  virtual void act(GoalContextRef G, RuleRef r) = 0;
};

class RuleObj : public GC_Obj
{
public:
    
  RuleObj(GoalId res, const vector<GoalId> reqs, GoalContextRef G) : res(res), reqs(reqs), G(G) { }
  
  GoalId getResult() const { return res; }

  string toString() const
  {
    ostringstream ostr;
    for(int i = 0; i < reqs.size(); i++)
      ostr << (i > 0 ? ", " : "") << G->toString(reqs[i]);
    ostr << " ==> " << G->toString(res);
    return ostr.str();
  }
  prop::Tag getResProp() const { return G->getProp(res);   }

  // The "action" is the code to be executed when the rule is applied.
  // This only applies to rules with a "result" that is prop::narrowed.
  void setAction(ActionRef a) { action = a; }
  void execAction() { if (!action.is_null()) action->act(G,this); }
  
private:
  GoalId res;
  vector<GoalId> reqs;
  int leftmostUnknown;
  GoalContextRef G;
  ActionRef action;
};


class RuleGenerator
{
public:
  virtual RuleRef apply(GoalContextRef G, GoalId gid) = 0;
};

class McCallumNaive : public RuleGenerator
{ public:
  RuleRef apply(GoalContextRef G, GoalId gid)
  {
    /* if (G->getProp(gid) != prop::ana_delin) { throw TarskiException("McCallumNaive applied to non ana-delin goal!"); } */
    /* IntPolyRef p = G->getPoly(gid); */
    /* GoalId gid1 = G->addCheck(prop::nnull-alpha,p); */
    /* if (G->getStatus(gid1) != TRUE) return NULL; */
    /* vector<GoalId> reqs{gid1}; */

    /* int k = G->getVarOrder()->level(p); */
    /* VarSet x = G->GetVarOrder()->get(k); */
    /* FactRef disc = G->getPolyManager()->discriminantFactors(p,x); */
    /* reqs.push_back(G->addNirGoal(prop::Tag oi_s, disc)); */
    /* FactRef ldcf = G->getPolyManager()->ldcfFactors(p,x); */
    /* reqs.push_back(G->addNirGoal(prop::Tag si_s, ldcf)); */
    
  }
};


// (oi p) ==> (si p)
class DedSiFromOi : public RuleGenerator
{ public:
  RuleRef apply(GoalContextRef G, GoalId gid)
  {
    if (G->getProp(gid) != prop::si) { throw TarskiException("dedSiFromOi(G,gid) called with non si goal!"); }
    return new RuleObj(gid,{G->addIrrGoal(prop::oi,G->getPoly(gid))},G);
  }
};

// (narrowed p) ==> (oi p)
class DedOiFromNarrowed : public RuleGenerator
{ public:
  RuleRef apply(GoalContextRef G, GoalId gid)
  {
    if (G->getProp(gid) != prop::oi) { throw TarskiException("DedOiFromNarrowed called with non oi goal!"); }
    return new RuleObj(gid,{G->addIrrGoal(prop::narrowed,G->getPoly(gid))},G);
  }
};

// (ana-delin p) ==> (narrowed p)
class DedNarrowedFromAnaDelin : public RuleGenerator
{ public:
  class NarrowAction : public ActionObj { public:
    void act(GoalContextRef G, RuleRef r) { cerr << "----> Narrow w.r.t. " << r->toString() << endl; }
  };
  RuleRef apply(GoalContextRef G, GoalId gid)
  {
    if (G->getProp(gid) != prop::narrowed) { throw TarskiException("DedNarrowedFromAnaDelin called with non narrowed goal!"); }
    RuleRef r = new RuleObj(gid,{G->addIrrGoal(prop::ana_delin,G->getPoly(gid))},G);
    r->setAction(new NarrowAction());
    return r;
  }
};



// (level1 p) ==> (ana-delin p)
class DedADFromLevel1 : public RuleGenerator
{ public:
  RuleRef apply(GoalContextRef G, GoalId gid)
  {
    if (G->getProp(gid) != prop::ana_delin) { throw TarskiException("DedADFromLevel1 called with goal other than ana_delin!"); }
    IntPolyRef p = G->getPoly(gid);
    GoalId gid1 = G->addCheck(prop::level1,p);
    if (G->getStatus(gid1) != TRUE)
      return NULL;
    else
      return new RuleObj(gid,{gid1},G);
  }
};

// (constant p) ==> (oi p)
class DedOiFromConstant : public RuleGenerator
{ public:
  RuleRef apply(GoalContextRef G, GoalId gid)
  {
    if (G->getProp(gid) != prop::oi) { throw TarskiException("DedOiFromConstant called with goal other than oi!"); }
    IntPolyRef p = G->getPoly(gid);
    GoalId gid1 = G->addCheck(prop::constant,p);
    if (G->getStatus(gid1) != TRUE)
      return NULL;
    else
      return new RuleObj(gid,{gid1},G);
  }
};

// product preserving properties
class ProductRuleGenerator : public RuleGenerator
{
  prop::Tag  prop_s, prop;
public:
  ProductRuleGenerator(prop::Tag prop_s, prop::Tag prop) : prop_s(prop_s), prop(prop) { }
  RuleRef apply(GoalContextRef G, GoalId gid)
  {
    if (G->getProp(gid) != prop_s) { throw TarskiException("ProductRuleGenerator applied with non-* goal!"); }
    FactRef F = G->getFact(gid);
    vector<GoalId> reqs;
    for(FactObj::factorIterator itr = F->factorBegin(); itr != F->factorEnd(); ++itr)
      reqs.push_back(G->addIrrGoal(prop,itr->first));
    return new RuleObj(gid,reqs,G);
  }
};



static map<prop::Tag, vector<RuleGenerator*> > ruleGenerator{
  {prop::si,        { new DedSiFromOi()} },
  {prop::oi,        { new DedOiFromConstant(), new DedOiFromNarrowed()} },
  {prop::ana_delin, { new DedADFromLevel1()} },
  {prop::narrowed,  { new DedNarrowedFromAnaDelin()} },
    
  {prop::si_s,      { new ProductRuleGenerator(prop::si_s,prop::si)} },
  {prop::oi_s,      { new ProductRuleGenerator(prop::oi_s,prop::oi)} },
  {prop::ni_s,      { new ProductRuleGenerator(prop::ni_s,prop::ni)} }
};

RuleRef generateRule(GoalContextRef G, GoalId gid)
{
  auto itr = ruleGenerator.find(G->getProp(gid));
  if (itr == ruleGenerator.end()) return NULL;
  vector<RuleGenerator*>& V = itr->second;
  RuleRef res = NULL;
  for(int i = 0; i < V.size() && res.is_null(); ++i)
    res = V[i] ->apply(G,gid);
  return res;
}

#endif

