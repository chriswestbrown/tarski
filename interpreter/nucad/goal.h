#ifndef _NUCAD_GOAL_
#define _NUCAD_GOAL_
#include "property.h"

/******************************************************
 * The goal context encapsulates the information needed
 * to interpret and process a goal.
 ******************************************************/
class GoalContextObj;
typedef GC_Hand<GoalContextObj> GoalContextRef;

class Goal;
typedef GC_Hand<Goal> GoalRef;

class Goal : public GC_Obj
{
private:
  prop::Tag prop;
  int status;
public:
  Goal(prop::Tag t) : prop(t), status(UNDET) { }
  prop::Tag getProp() const { return prop; }
  virtual IntPolyRef getPoly() const { return NULL; }
  virtual FactRef getFact() const { return NULL; }
  int getStatus() const { return status; }
  void setStatus(int val) { status = val; }
  virtual bool operator<(const Goal &g) const { return false; }
  virtual ~Goal() { }
  virtual string toString(PolyManager* pPM) const { return "dummy"; }
};

class NirGoal : public Goal
{
private:
  FactRef F;
public:
  NirGoal(prop::Tag prop, FactRef F) : Goal(prop), F(F) { }
  bool operator<(const Goal &g) const
  {
    if (this->getProp() != g.getProp()) { return this->getProp() < g.getProp(); }
    const NirGoal* ptr = dynamic_cast<const NirGoal*>(&g);
    return this->F->cmp(ptr->F);    
  }
  string toString(PolyManager* pPM) const { return "(" + name(getProp()) + " " + F->toString() + ")"; }
  FactRef getFact() const { return F; }
};

class IrrGoal : public Goal
{
private:
  IntPolyRef p; // this must be canonical!
public:
  IrrGoal(prop::Tag prop, IntPolyRef p) : Goal(prop), p(p) { }
  bool operator<(const Goal &g) const {
    if (this->getProp() != g.getProp()) { return this->getProp() < g.getProp(); }
    const IrrGoal* ptr = dynamic_cast<const IrrGoal*>(&g);
    return IntPolyObj::ipcmp(p,ptr->p);
  }
  string toString(PolyManager* pPM) const { return "(" + name(getProp()) + " " + pPM->polyToStr(p) + ")"; }
  IntPolyRef getPoly() const { return p; }
};

typedef int GoalId;

class GoalContextObj : public GC_Obj
{
private:
  SamplePointManagerRef SM;
  SamplePointId sid;

public:
  GoalContextObj(SamplePointManagerRef SM, SamplePointId sid) : SM(SM), sid(sid) { }
  VarOrderRef getVarOrder() const { return SM->getVarOrder(); }
  SamplePointManagerRef getSamplePointManager() const { return SM; }
  PolyManager* getPolyManager() const { return SM->getPolyManager(); }
  SamplePointId getSamplePointId() const { return sid; }

private:
  vector<GoalRef> goalBase;
  class GoalRefCmp { public: bool operator()(const GoalRef &g1, const GoalRef &g2) { return *g1 < *g2; } };
  map<GoalRef,GoalId,GoalRefCmp> goal2id;

  GoalId add(const GoalRef &g)
  {
    map<GoalRef,int, GoalRefCmp>::iterator i = goal2id.find(g);
    if (i != goal2id.end()) { return i->second; }
    int id = goalBase.size();
    goalBase.push_back(g);
    goal2id[g] = id;
    return id;
  }
  
public:

  string toString(GoalId gid)
  {
    return goalBase[gid]->toString(getVarOrder()->getPolyManager());
  }  
  int getStatus(GoalId gid) const { return goalBase[gid]->getStatus(); }
  prop::Tag getProp(GoalId gid) const { return goalBase[gid]->getProp(); }
  IntPolyRef getPoly(GoalId gid) const { return goalBase[gid]->getPoly(); }
  FactRef getFact(GoalId gid) const { return goalBase[gid]->getFact(); }
  
  // adds check (prop p)
  GoalId addCheck(prop::Tag prop, IntPolyRef p)
  {
    GoalRef g = new IrrGoal(prop,p);
    GoalId id = add(g);
    goalBase[id]->setStatus(prop::check(prop, this, p));
    return id;
  }

  // adds goal* (prop F)
  int addNirGoal(prop::Tag prop, FactRef F)
  {
    GoalRef g = new NirGoal(prop,F);
    return add(g);
  }

  // adds goal (prop p)
  int addIrrGoal(prop::Tag prop, IntPolyRef p)
  {
    GoalRef g = new IrrGoal(prop,p);
    return add(g);
  }
  
};


#endif
