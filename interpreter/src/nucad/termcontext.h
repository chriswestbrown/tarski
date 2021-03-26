#ifndef _NUCAD_TERM_CONTEXT_
#define _NUCAD_TERM_CONTEXT_
#include "term.h"

namespace tarski {

/*
    The purpose of the TermContextObj is to give a unique canonical 
    copy of each term (or at present for each Property Term).  This
    means equivalence later on down the road can be reduced to identical 
    address.
    IMPORTANT: TermId of 0 is a "null id".
  */
  
  typedef unsigned int TermId;
  
  class TermContextObj;
  typedef GC_Hand<TermContextObj> TermContextRef;

  class TermContextObj : public GC_Obj
  {
  public:
    virtual TermId add(const TermRef &g) = 0;
    virtual TermRef get(TermId id) = 0;
    virtual void dump(PolyManager *pPM) = 0;
    virtual ~TermContextObj() { }
  };    


  class SimpleTermContextObj : public TermContextObj
  {
  public:
    SimpleTermContextObj()
    {
      termBase.push_back(NULL); // This ensures that no valid term gets Id zero
    }
    TermId add(const TermRef &g)
    {
      map<TermRef, TermId, TermRefCmp>::iterator i = term2id.find(g);
      if (i != term2id.end()) { return i->second; }
      int id = termBase.size();
      termBase.push_back(g);
      term2id[g] = id;
      return id;
    }
    TermRef get(TermId id) { return termBase[id]; }
    void dump(PolyManager *pPM)
    {
      for(int i = 1; i < termBase.size(); ++i)
	std::cout << "term " << i << " = " << termBase[i]->toString(pPM) << std::endl;
    }
    
  private:
    vector<TermRef> termBase;
    class TermRefCmp {
    public:
      bool operator()(const TermRef &g1, const TermRef &g2) const { return g1->comp(*g2) < 0; }
    };
    map<TermRef,TermId,TermRefCmp> term2id;
  };

}
#endif

