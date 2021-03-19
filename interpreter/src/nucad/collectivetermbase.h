#ifndef _NUCAD_COLLECTIVE_TERM_BASE_
#define _NUCAD_COLLECTIVE_TERM_BASE_

#include "nucadcell.h"

namespace tarski { 


/*
    CollectiveTermBase - the idea here is that instead of having each cell maintain
    a complete database of all terms known to hold, a term only stores a database of
    terms known to hold in it, but not already known to hold in its parent!  The tricky
    part is that not all properties are inherited. In particular, the sample point of 
    the parent may or maynot be the same as the sample point of the child.  

    The other issue is that cell-bound properties at or below the split-level are not 
    inherited.  This could be done more cleanly, I'm sure, but it's a bit difficult.  
    As the cell is refined, it needs to figure out its own cell-bounds, since they may
    change, and having other cell bounds from the parent pop up causes problems.

    So, any recursive search in the CollectiveTermBase has to track
    1. recursion level: recursion level 0 is the top-level call, and anything in it's
       TermBase proper is a term that holds in the cell.
    2. top recursion split level.  The top-level recursive call originates with some cell
       c.  The cell-bounds for c below its split level are inherited.  Those at or above
       its split level are not.
    3. minimum prior split level.  Understanding which properties-at-alpha are inherited
       is a little bit special.  If the top-level recursive call originated from c, and we
       are considering ancestor cell c', the sample of c and c' agree in the first l-1
       coordinates, where l is the minimum of the split levels of all cells prior to c' in 
       the chain of recursive calls. Thus, any "at-alpha" properties of level less than l
       are inherited.  Certain others of level equal to l are inherited as well:
   */
  class CollectiveTermBase : public TermBase
  {
  private:
    NuCADCell* owner;
  public:
    CollectiveTermBase(NuCADCell* owner) { this->owner = owner; }

    TermBaseRef getTermBase() { return owner->getTermBase(); }
    NuCADCellRef getParent() { return owner->getParent(); }
    TermContextRef getTermContext() { return owner->getTermContext(); }
    VarOrderRef getVarOrder() { return owner->getVarOrder(); }
    int getSplitLevel() { return owner->getSplitLevel(); }
    char getSplitLabel() { return owner->getSplitLabel(); }
    
    // recursionLevel = 0 is the top-level call (at this level minSplitLevel/minSplitLabel are meaningless)
    // minSplitLevel is the minimum level of all splits from the top recursion level to this one.
    //               if k is the minSplitLevel, then this parent and the top-level cell agree in
    //               coordinates 1,...,k-1
    // maxSplitLevel is obvious ... the point is that cell-bounds at or below the split level are not inherited
    TermBaseDatum* get(TermId id, int recursionLevel, int minSplitLevel, char minSplitLabel, int maxSplitLevel);

    TermBaseDatum* get(TermId id) { return get(id,0,0,'x',0); }    
    bool isIn(TermId id) { return get(id,0,0,'x',0) != NULL; }

    void add(TermId id, const std::string &reason);
    void add(TermId id, const std::string &reason, const std::vector<TermId> &froms);
    void add(TermId id, const TermBaseDatum& D);

    void remove(TermId id) // NOTE: this is a "shallow" remove, and that changes the sematics of a termbase!
    {
      getTermBase()->remove(id);
    }

    const std::string getReason(TermId id);
    void addDed(TermId id, TermId did);
    std::vector<TermId>::iterator dedfBegin(TermId id);
    std::vector<TermId>::iterator dedfEnd(TermId id);
    void fillWithTerms(std::vector<TermId> &V,
		       int recursionLevel, int minSplitLevel, char minSplitLabel, int maxSplitLevel);
    void fillWithTerms(std::vector<TermId> &V)
    {
      fillWithTerms(V,0,0,'x',0);
    }
    void fillWithTermsALL(std::vector<TermId> &V);
  };

}
#endif
