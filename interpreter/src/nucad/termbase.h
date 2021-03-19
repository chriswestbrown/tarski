#ifndef _NUCAD_TERM_BASE_
#define _NUCAD_TERM_BASE_
#include "termcontext.h"
#include <stack>

namespace tarski {

  class TermBase;
  typedef GC_Hand<TermBase> TermBaseRef;

  class TermBaseDatum
  {
  public:
    TermBaseDatum() : reason("uninit") { }
    TermBaseDatum(const string &r) : reason(r) { }
    TermBaseDatum(const string &r, const std::vector<TermId> &dedf)
      : reason(r), deduceFrom(dedf) { }
  TermBaseDatum(const TermBaseDatum& T) : reason(T.reason), deduceFrom(T.deduceFrom) { }
    std::string getReason() const { return reason; }
    std::vector<TermId>::iterator dedfBegin() { return deduceFrom.begin(); }
    std::vector<TermId>::iterator dedfEnd() { return deduceFrom.end(); }
    void addDed(TermId tid) { deduceFrom.push_back(tid); }
  private:
    std::string reason;
    std::vector<TermId> deduceFrom;
  };
  
  class TermBase : public GC_Obj
  {
  public:
    virtual bool isIn(TermId id) = 0;
    virtual void add(TermId id, const std::string &reason) = 0;
    virtual void add(TermId id, const std::string &reason, const std::vector<TermId> &froms) = 0;
    virtual void add(TermId id, const TermBaseDatum& D) = 0;
    virtual void remove(TermId id) = 0;
    virtual const std::string getReason(TermId id) = 0;
    virtual void addDed(TermId id, TermId did) = 0;
    virtual std::vector<TermId>::iterator dedfBegin(TermId id) = 0;
    virtual std::vector<TermId>::iterator dedfEnd(TermId id) = 0;
    virtual void dump(PolyManager* pPM, TermContextRef TC);
    virtual TermBaseDatum* get(TermId id) = 0;

    // Input: inTerms - the terms from the TermBase
    // Side Effect: the terms in inTerms are reordered such that 
    // if id1 comes before id2, then id2 is not in the "deduceFrom" list for term id1.
    // In other words, if you're looking at term id1 in the list, any terms from the
    // "proof" for id1 come later in inTerms.
    // NOTE: The reason the caller provides inTerms isi that TermBase does not have a
    //       a uniform way to iterate over the terms in the TermBase (which I may fix later!)
    virtual void topologicalSort(std::vector<TermId> &inTerms);

    virtual void fillWithTerms(std::vector<TermId> &V) = 0;
  };

  class SimpleTermBase : public TermBase
  {
  public:
    bool isIn(TermId id)
    {
      return table.count(id) > 0;
    }
    void add(TermId id, const std::string &reason)
    {
      table[id] = TermBaseDatum(reason);
    }
    void add(TermId id, const std::string &reason, const std::vector<TermId> & froms)
    {
      table[id] = TermBaseDatum(reason,froms);
    }
    void add(TermId id, const TermBaseDatum& D)
    {
      table.emplace(id,D);
    }
    void remove(TermId id)
    {
      auto itr = table.find(id);
      if (itr != table.end())
	table.erase(itr);
    }
    const std::string getReason(TermId id)
    {
      return table[id].getReason();
    }
    void addDed(TermId id, TermId did)
    {
      table[id].addDed(did);
    }
    virtual std::vector<TermId>::iterator dedfBegin(TermId id)
    {
      return table[id].dedfBegin();
    }
    virtual std::vector<TermId>::iterator dedfEnd(TermId id)
    {
      return table[id].dedfEnd();
    }
    void fillWithTerms(std::vector<TermId> &V)
    {
      for(auto itr = table.begin(); itr != table.end(); ++itr)
	V.push_back(itr->first);
    }
    TermBaseDatum* get(TermId id)
    {
      auto itr = table.find(id);
      return itr != table.end() ? &(itr->second) : NULL;
    }
  private:
    std::unordered_map<TermId,TermBaseDatum> table;
  };
  
}
#endif
