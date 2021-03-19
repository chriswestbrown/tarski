#ifndef _NUCAD_TERMQUEUE_
#define _NUCAD_TERMQUEUE_
#include "termbase.h"
#include <utility>
#include <queue>
#include <set>

namespace tarski {

  /*
    A TermQueue is used to store Property Terms.  When a
    Property Term is enqueued, it means that we will ultimately
    have to prove that the property holds.
    TermQueue should operate such that anything already 
    enqueue'd or already in the term base should be 
    ignored on enqueue.  Dequeue order is important!  In particular ...

    For the current algorithm to function well, we want 
    1. all factor properties to come first, then ...
    2. all si, oi and sector properties, then ...
    3. all property terms that have "section variables", then ...
    4. everything else.

    The rationale is that only factor, si, oi and sector terms
    lead to new section variables.  Moreover, nothing else leads
    to new terms of those types at the current level.  Thus, when
    all those are gone we know that we have all the section variables
    we're going to have at the current level, and so we can get rid of
    them.
   */
  class TermQueue;
  typedef GC_Hand<TermQueue> TermQueueRef;

  class TermQueue : public GC_Obj
  {
  public:
    virtual void enqueue(TermId tid) = 0;
    virtual void enqueue(TermRef T) = 0;
    virtual std::pair<TermRef,TermId> dequeue() = 0;
    virtual std::pair<TermRef,TermId> peek() const = 0;
    virtual bool empty() const = 0;    
    virtual ~TermQueue() { }
  };
  
  class SimpleTermQueue : public TermQueue
  {
  public:
    SimpleTermQueue(TermContextRef TC, TermBaseRef TB) : TC(TC), TB(TB) { }
    void enqueue(TermId tid)
    {
      if (TB->isIn(tid) || properTermsInQueue.find(tid) != properTermsInQueue.end())
      {
	std::cerr << "Not enqueuing term " << tid << std::endl;
	return;
      }
      else
      {
	Q.push(std::pair<TermRef,TermId>(TC->get(tid),tid));
	properTermsInQueue.insert(tid);
      }
    }
    void enqueue(TermRef T)
    {
      if (T->hasSectionVariable())
	Q.push(std::pair<TermRef,TermId>(T,0));
      else
	enqueue(TC->add(T));
    }
    std::pair<TermRef,TermId> dequeue()
    {
      auto res = Q.top(); Q.pop();
      if (res.second != 0)
	properTermsInQueue.erase(res.second);
      return res;
    }
    std::pair<TermRef,TermId> peek() const
    {
      return Q.top();
    }
    bool empty() const
    {
      return Q.empty();
    }
  private:
    class SpecialOrder {
    public:
      int tier(TermRef T)
      {
	if (T->hasFactor()) { return 4; }
	prop::Tag p = T->getProperty();
	if (T->hasSectionVariable()) return 2;
	if (p == prop::si || p == prop::oi || p == prop::sector) return 3;
	return 1;
      }
      bool operator()(const std::pair<TermRef,TermId> &a, std::pair<TermRef,TermId> &b)
      {
	if (a.first->kind() != Term::property || b.first->kind() != Term::property)
	  throw TarskiException("TermQueue's can only contain property terms!");
	int ta = tier(a.first), tb = tier(b.first);
	if (ta != tb) return ta < tb;
	if (a.second != 0)
	  return a.second < b.second;
	return a.first.vpval() < b.first.vpval();
      }
    };
    TermContextRef TC;
    TermBaseRef TB;
    std::priority_queue<std::pair<TermRef,TermId>, std::vector<std::pair<TermRef,TermId> >, SpecialOrder > Q;
    std::set<TermId> properTermsInQueue;
  };  


  class SimpleTermQueueOLD : public TermQueue
  {
  public:
    SimpleTermQueueOLD(TermContextRef TC, TermBaseRef TB) : TC(TC), TB(TB) { }
    void enqueue(TermId tid)
    {
      if (TB->isIn(tid)) return; else woVar.push(tid);
    }
    void enqueue(TermRef T)
    {
      if (T->hasSectionVariable())
	wiVar.push(T);
      else
      {
	TermId tid = TC->add(T);
	enqueue(tid);
      }
    }
    std::pair<TermRef,TermId> dequeue()
    {
      if (woVar.empty())
      {
	auto res = std::pair<TermRef,TermId>(wiVar.front(),0);
	wiVar.pop();
	return res;
      }
      else
      {
	TermId tid = woVar.front(); woVar.pop();
	while(!woVar.empty() && (woVar.front() == tid || TB->isIn(woVar.front())))
	  woVar.pop();
	return std::pair<TermRef,TermId>(TC->get(tid),tid);
      }
    }
    std::pair<TermRef,TermId> peek() const
    {
      if (woVar.empty())
	return std::pair<TermRef,TermId>(wiVar.front(),0);
      else
      {
	TermId tid = woVar.front();
	return std::pair<TermRef,TermId>(TC->get(tid),tid);
      }
    }
    bool empty() const
    {
      return wiVar.empty() && woVar.empty();
    }
  private:
    TermContextRef TC;
    TermBaseRef TB;
    std::queue<TermId> woVar;
    std::queue<TermRef> wiVar;
  };  




  class LeveledTermQueue; typedef GC_Hand<LeveledTermQueue> LeveledTermQueueRef;
  class LeveledTermQueue : public GC_Obj
  {
  public:
    virtual void enqueue(TermId tid) = 0;
    virtual void enqueue(const std::vector<TermId> &V) = 0;
    virtual void enqueue(TermRef T) = 0;
    virtual void enqueue(const std::vector<TermRef> &V) = 0;
    virtual TermQueueRef get(int level) = 0;
    virtual ~LeveledTermQueue() { }
  };

  class SimpleLeveledTermQueue : public LeveledTermQueue
  {
  private:
    TermContextRef TC;
    TermBaseRef TB;
    VarOrderRef V;
    vector<TermQueueRef> Q;
  public:
    SimpleLeveledTermQueue(TermContextRef TC, TermBaseRef TB, VarOrderRef V, int N) :
      TC(TC), TB(TB), V(V), Q(N+1)
    {
      for(unsigned int i = 0; i <= N; i++)
	Q[i] = new SimpleTermQueue(TC, TB);
    }
    void enqueue(TermId tid) {
      std::cerr << "enqueueing ... [" << tid << "] " << TC->get(tid)->toString(V->getPolyManager()) << std::endl;
      get(TC->get(tid)->level(V))->enqueue(tid);
    }
    void enqueue(const std::vector<TermId> &V)
    {
      for(auto itr = V.begin(); itr != V.end(); ++itr)
	enqueue(*itr);
    }
    void enqueue(TermRef T) {
      if (T->hasSectionVariable())
	std::cerr << "enqueueing ... [*] " << T->toString(V->getPolyManager()) << std::endl;
      else
	std::cerr << "enqueueing ... [" << TC->add(T) << "] " << T->toString(V->getPolyManager()) << std::endl;
      get(T->level(V))->enqueue(T);
    }     
    void enqueue(const std::vector<TermRef> &V)
    {
      for(auto itr = V.begin(); itr != V.end(); ++itr)
	enqueue(*itr);
    }
    TermQueueRef get(int level) { return Q[level]; }
  };
  
}
#endif

