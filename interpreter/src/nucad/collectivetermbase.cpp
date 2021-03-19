#include "collectivetermbase.h"
#include "property.h"

using namespace tarski;

TermBaseDatum* CollectiveTermBase::get(TermId id, int recursionLevel, int minSplitLevel, char minSplitLabel,
				       int maxSplitLevel)
{
  TermBaseDatum* p = getTermBase()->get(id);
  if (p != NULL && recursionLevel == 0 || p == NULL && getParent().is_null()) return p;

  if (p != NULL)   // at this point p is non-NULL and the recursionLevel > 0
  {
    TermRef T = getTermContext()->get(id);
    prop::Tag p_T = T->getProperty();
    int putativeTermLevel = T->level(getVarOrder());
    bool above = (minSplitLabel == 'S' || minSplitLabel == 'U');
    bool inherit = inheritAtLevel(p_T,putativeTermLevel,minSplitLevel,above);
    inherit = inherit && (p->getReason() != "cell-bound" || putativeTermLevel > maxSplitLevel);
    return inherit ? p : NULL;
  }
  
  // Determine new minSplitLevel/Label to use for recursive call
  // NOTE:  if the old split level is the same as the new, we still need the new split label to
  //        operate correctly.  That's because the "new split" actually happened earlier in the NuCAD construction
  int newMinSplitLevel = recursionLevel == 0 ? getSplitLevel() : std::min(minSplitLevel,getSplitLevel());
  char newMinSplitLabel = newMinSplitLevel == getSplitLevel() ? getSplitLabel() : minSplitLabel;
  int newMaxSplitLevel = recursionLevel == 0 ? getSplitLevel() : std::max(maxSplitLevel,getSplitLevel());
  if (p == NULL)
    return getParent()->getCollectiveTermBase()->get(id,recursionLevel+1,newMinSplitLevel,newMinSplitLabel,
						     newMaxSplitLevel);
}

void CollectiveTermBase::fillWithTerms(std::vector<TermId> &V,
				       int recursionLevel, int minSplitLevel, char minSplitLabel,
				       int maxSplitLevel)
{
  auto TC = getTermContext();
  auto X = getVarOrder();
  {
    std::vector<TermId> Vp;
    getTermBase()->fillWithTerms(Vp);
    for(auto itr = Vp.begin(); itr != Vp.end(); ++itr)
    {
      bool inherit = true;
      if (recursionLevel != 0)
      {
	TermBaseDatum* p = getTermBase()->get(*itr);
	TermRef T = TC->get(*itr);
	prop::Tag p_T = T->getProperty();
	int putativeTermLevel = T->level(X);
	bool above = (minSplitLabel == 'S' || minSplitLabel == 'U');
	inherit = inheritAtLevel(p_T,putativeTermLevel,minSplitLevel,above);
	inherit = inherit && (p->getReason() != "cell-bound" || putativeTermLevel > maxSplitLevel);
      }
      if (inherit)
	V.push_back(*itr);
    }
  }
  
  if (!getParent().is_null())
  {
    int newMinSplitLevel = recursionLevel == 0 ? getSplitLevel() : std::min(minSplitLevel,getSplitLevel());
    char newMinSplitLabel = newMinSplitLevel == getSplitLevel() ? getSplitLabel() : minSplitLabel;
    int newMaxSplitLevel = recursionLevel == 0 ? getSplitLevel() : std::max(maxSplitLevel,getSplitLevel());
    getParent()->getCollectiveTermBase()->fillWithTerms(V,recursionLevel+1,newMinSplitLevel,newMinSplitLabel,
							newMaxSplitLevel);      
  }
}

void CollectiveTermBase::fillWithTermsALL(std::vector<TermId> &V)
{
  getTermBase()->fillWithTerms(V);
      if (!getParent().is_null())
	getParent()->getCollectiveTermBase()->fillWithTermsALL(V);      
}




void CollectiveTermBase::add(TermId id, const std::string &reason)  
{
  if (!isIn(id))
    getTermBase()->add(id,reason);
}

void CollectiveTermBase::add(TermId id, const std::string &reason, const std::vector<TermId> &froms)
{
  if (!isIn(id))
    getTermBase()->add(id,reason,froms);      
}

void CollectiveTermBase::add(TermId id, const TermBaseDatum& D)
{
  if (!isIn(id))
    getTermBase()->add(id,D);
}


const std::string CollectiveTermBase::getReason(TermId id)
{
  TermBaseDatum* p = get(id);
  if (p == NULL) throw TarskiException("TermId id not in term base!");
  return p->getReason();
}

void CollectiveTermBase::addDed(TermId id, TermId did)
{
  TermBaseDatum* p = get(id);
  if (p == NULL) throw TarskiException("TermId id not in term base!");
  p->addDed(did);
}


std::vector<TermId>::iterator CollectiveTermBase::dedfBegin(TermId id)
{
  TermBaseDatum* p = get(id);
  if (p == NULL) throw TarskiException("TermId id not in term base!");
  return p->dedfBegin();
}

std::vector<TermId>::iterator CollectiveTermBase::dedfEnd(TermId id)
{
  TermBaseDatum* p = get(id);
  if (p == NULL) throw TarskiException("TermId id not in term base!");
  return p->dedfEnd();
}

