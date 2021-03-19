#include "nucadcell.h"
#include "prophandler.h"
#include "collectivetermbase.h"

using namespace tarski;

NuCADCell::NuCADCell(SamplePointManagerRef SM, SamplePointId sid, bool samplePointIsUnfinished,
		     TermContextRef TC, NuCADCell* parent, char splitLabel, int splitLevel) :
  SM(SM), mysid_ONLY_USE_VIA_GETTER(sid), samplePointIsUnfinished(samplePointIsUnfinished),
  TC(TC), parent(parent), splitLabel(splitLabel), splitLevel(splitLevel)
{
  TB = new SimpleTermBase();
  CTB = new CollectiveTermBase(this);
  maxLevel = 0;
}



SamplePointId NuCADCell::getSamplePointId()
{
  if (samplePointIsUnfinished)
  {
    mysid_ONLY_USE_VIA_GETTER = getSamplePointManager()->completeSample(mysid_ONLY_USE_VIA_GETTER,getSplitLevel(),parent);
    samplePointIsUnfinished = false;
  }
  return mysid_ONLY_USE_VIA_GETTER;
}


    void NuCADCell::record( TermId conclusion, const string &reason )
    {
      record(conclusion,reason,std::vector<TermId>());
    }

    void NuCADCell::record( TermId conclusion, const string &reason, const std::vector<TermId>& froms )
    {
      if (TB->isIn(conclusion))
      {
	std::cerr << "NOTE: in recording, duplicate " << TC->get(conclusion)->toString(getPolyManager())
		  << std::endl; //	std::cerr << "error recording " << TC->get(conclusion)->toString(getPolyManager()) << std::endl;
	//throw TarskiException("NuCADCell.record ... conclusion already in term base");
	return;
      }
      TB->add(conclusion,reason,froms);
      for(auto itr = froms.begin(); itr != froms.end(); ++itr)
	if (!TB->isIn(*itr))
	  Q->enqueue(*itr);
    }

    void NuCADCell::record( TermId conclusion, const string &reason, const std::vector<TermRef>& froms )
    {
      if (TB->isIn(conclusion))
      {
	std::cerr << "NOTE: in recording, duplicate " << TC->get(conclusion)->toString(getPolyManager()) << std::endl;
	
	//throw TarskiException("NuCADCell.record ... conclusion already in term base");
	return;
      }
      
      std::vector<TermId> V;      
      for(int i = 0; i < froms.size(); i++)
      {
	if (!froms[i]->hasSectionVariable())
	  V.push_back(getTermContext()->add(froms[i]));
	else
	{
	  // record that TermId conclusion is "waiting on" the TermObj froms[i] is pointing to
	  waitingOn[froms[i].vpval()] = conclusion;
	  Q->enqueue(froms[i]);
	}
      }
      record(conclusion,reason,V);
    }


void NuCADCell::chooseSectionVariableValues(int currentLevel, TermRef pt, TermId nextid)
{
  TermQueueRef TQ = Q->get(currentLevel);

  if (pt->hasSectionVariable())
  {
    // Dequeue everything
    vector<PropertyRef> V;
    V.push_back(pt);
    while(!TQ->empty())
      V.push_back(dynamic_cast<Property*>(&(*(TQ->dequeue().first))));
    
    // Run through elts of V tracking the closest concrete sections below and above alpha
    // In other words, decide what "upper" and "lower" sections will be!
    SectionRef lower = Section::mkNegInfty(currentLevel);
    SectionRef upper = Section::mkPosInfty(currentLevel);
    for(auto itr = V.begin(); itr != V.end(); ++itr)
    {
      PropertyRef pt = *itr;
      std::cerr << "  ---  " << pt->toString(getPolyManager()) << std::endl;
      switch(pt->getProperty())
      {
      case prop::sector:
	break;
      case prop::leq: {
	SectionTermRef t0 = pt->getChild(0);
	SectionTermRef t1 = pt->getChild(1);
	if (t1->hasSectionVariable())
	{
	  if (SM->compareAtAlpha(getSamplePointId(),lower,t0->getSection()) < 0)
	    lower = t0->getSection();	      
	}
	else
	{
	  if (SM->compareAtAlpha(getSamplePointId(),t1->getSection(),upper) < 0)
	    upper = t1->getSection();	      
	}	    
      } break;
      default:
	if (pt->hasSectionVariable())
	  throw TarskiException("Error!  Don't know how to handle " + prop::name(pt->getProperty())
				+ " containing section variables.");
	//-- Otherwise we just ignore this!	  
	break;
      }
    }

    std::cout << "lower = " << lower->toStr(getPolyManager()) << " "
	      << "upper = " << upper->toStr(getPolyManager()) << std::endl;

    // Set cell bounds at this level!
    setLowerBoundAtLevel(currentLevel,lower);
    setUpperBoundAtLevel(currentLevel,upper);
    

    // Actually assign values to section variables and re-enqueue the resulting terms
    for(auto itr = V.begin(); itr != V.end(); ++itr)
    {
      PropertyRef pt = *itr;
      switch(pt->getProperty())
      {
      case prop::sector: {
	SectionTermRef t0 = pt->getChild(0);
	SectionTermRef t1 = pt->getChild(1);
	TermId tid = TC->add(new Property(prop::sector, new SectionTerm(lower), new SectionTerm(upper)));
	record(tid, "cell-bound");
	auto itr = waitingOn.find(pt.vpval());
	if (itr != waitingOn.end())
	{
	  TB->addDed(itr->second,tid);
	  waitingOn.erase(itr);
	}
      } break;
      case prop::leq: {
	SectionTermRef t0 = pt->getChild(0);
	SectionTermRef t1 = pt->getChild(1);
	TermId tid = 0;
	if (t1->hasSectionVariable())
	  tid = TC->add(new Property(prop::leq, t0, new SectionTerm(lower)));
	else
	  tid = TC->add(new Property(prop::leq, new SectionTerm(upper), t1));
	Q->enqueue(tid);
	auto itr = waitingOn.find(pt.vpval());
	if (itr != waitingOn.end())
	{
	  TB->addDed(itr->second,tid);
	  waitingOn.erase(itr);
	}	
      } break;
      default:
	if (pt->hasSectionVariable())
	  throw TarskiException("Error!  Don't know how to handle " + prop::name(pt->getProperty())
				+ " containing section variables.");
	Q->enqueue(pt); //-- just put it right back in the queue
	break;
      }
    }
  }
}

CollectiveTermBaseRef NuCADCell::getCollectiveTermBase() { return CTB; }

void NuCADCell::refineHelp(const std::vector<TermRef> &goals)
{
  std::vector<TermId> A(goals.size());
  for(int i = 0; i < goals.size(); ++i)
    A[i] = TC->add(goals[i]);
  refineHelp(A);
}

void NuCADCell::refineHelp(const std::vector<TermId> &goals)
{
    int nextSectionIdentifier = 0;
    VarOrderRef V = SM->getVarOrder();
    int N = 0;
    for(int i = 0; i < goals.size(); i++)
      N = std::max(N,TC->get(goals[i])->level(V));
    Q = new SimpleLeveledTermQueue(TC,TB,V,N);
    for(int i = 0; i < goals.size(); i++)
      Q->enqueue(goals[i]);

    PropHandlerRef PH = new TempPropHandler();
    int currentLevel = N;
    while(currentLevel >= 0)
    {
      TermQueueRef TQ = Q->get(currentLevel);
      if (TQ->empty()) { --currentLevel; continue; }
      auto next = TQ->dequeue();
      PropertyRef pt = dynamic_cast<Property*>(&(*(next.first)));
      TermId nextid = next.second;
      std::cout << "dequeued " << pt->toString(getPolyManager()) << std::endl;
      if (pt->hasSectionVariable())
	chooseSectionVariableValues(currentLevel, pt, nextid);
      else
	PH->handle(currentLevel, pt, nextid, this, nextSectionIdentifier);  
    }
    // TB->dump(getPolyManager(),TC);
    // for(int level = N; level > 0; --level)
    // {
    //   std::cout << getLowerBoundAtLevel(level)->toStr(getPolyManager()) << " < "
    // 		<< getPolyManager()->getName(getVarOrder()->get(level)) << " < "
    // 		<< getUpperBoundAtLevel(level)->toStr(getPolyManager()) << std::endl;
    // }
}

SectionRef NuCADCell::getLowerBoundAtLevel(int k)
{
  auto itr = lowerBoundAtLevel.find(k);
  if (itr != lowerBoundAtLevel.end())
    return itr->second;
  if (!getParent().is_null())
    return getParent()->getLowerBoundAtLevel(k);
  else
    return Section::mkNegInfty(k);
}

void NuCADCell::setLowerBoundAtLevel(int k, SectionRef T)
{
  lowerBoundAtLevel[k] = T;
}

SectionRef NuCADCell::getUpperBoundAtLevel(int k)
{
  auto itr = upperBoundAtLevel.find(k);
  if (itr != upperBoundAtLevel.end())
    return itr->second;
  if (!getParent().is_null())
    return getParent()->getUpperBoundAtLevel(k);
  else
    return Section::mkPosInfty(k);
}

void NuCADCell::setUpperBoundAtLevel(int k, SectionRef T)
{
  upperBoundAtLevel[k] = T;
}

bool NuCADCell::isSectionAtLevel(int k)
{
  SectionRef sl = getLowerBoundAtLevel(k);
  SectionRef su = getUpperBoundAtLevel(k);
  if (sl->getKind() != Section::indexedRoot || su->getKind() != Section::indexedRoot)
    return false;
  return sl->compare(su) == 0;
}


int NuCADCell::getMaxLevel()
{
    int N = 0;
    for(auto itr = lowerBoundAtLevel.begin(); itr != lowerBoundAtLevel.end(); ++itr)
      N = std::max(N,itr->first);
    for(auto itr = upperBoundAtLevel.begin(); itr != upperBoundAtLevel.end(); ++itr)
      N = std::max(N,itr->first);
    if (!getParent().is_null())
      N = std::max(N,getParent()->getMaxLevel());
    return N;
} 
std::string NuCADCell::cellDescrip()
{
  int N = getMaxLevel();
  std::ostringstream sout;
  for(int level = N; level > 0; --level)
  {
    if (isSectionAtLevel(level))
      sout << getPolyManager()->getName(getVarOrder()->get(level)) << " = "
	   << getLowerBoundAtLevel(level)->toStr(getPolyManager()) << std::endl;
    else
      sout << getLowerBoundAtLevel(level)->toStr(getPolyManager()) << " < "
	   << getPolyManager()->getName(getVarOrder()->get(level)) << " < "
	   << getUpperBoundAtLevel(level)->toStr(getPolyManager()) << std::endl;
  }
  // For debugging!
  sout << "Sample point is - " << (samplePointIsUnfinished ? "unfinished" : "finished") << std::endl;
    sout << getSamplePointManager()->toStr(getSamplePointId());
  return sout.str();
}

void NuCADCell::transferCellBoundsToGoals(int N, std::vector<TermRef> &goals)
{
  for(int i = N; i > 0; --i)
  {
    TermRef st = new Property(prop::sector,
			      new SectionTerm(getLowerBoundAtLevel(i)),
			      new SectionTerm(getUpperBoundAtLevel(i)));
    TermId stId = getTermContext()->add(st);
    getTermBase()->remove(stId);
    goals.push_back(st);
  }
}

int NuCADCell::signAtSamplePoint(IntPolyRef p)
{
  return getSamplePointManager()->polynomialSignAt(getSamplePointId(),p);
}

int NuCADCell::checkStatus(TermId tid)
{
  TermBaseRef TB = getCollectiveTermBase();
  return TB->isIn(tid);
}


//----------------------------- NuCADCellMOD --------------------------------//
void NuCADCellMOD::refineHelp(const std::vector<TermId> &goals)
{
  int nextSectionIdentifier = 0;
  VarOrderRef V = getVarOrder();
  TermContextRef TC = getTermContext();
  TermBaseRef TB = getCollectiveTermBase();
  int N = 0;
  for(int i = 0; i < goals.size(); i++)
    N = std::max(N,TC->get(goals[i])->level(V));
  Q = new SimpleLeveledTermQueue(TC,TB,V,N);
  for(int i = 0; i < goals.size(); i++)
    Q->enqueue(goals[i]);

  PropHandlerRef PH = new TempPropHandlerMOD();
  int currentLevel = N;
  bool boundsChosen = false;
  TermQueueRef TQ = Q->get(currentLevel);
  while(currentLevel >= 0)
  {
    if (TQ->empty()) { --currentLevel; boundsChosen = false; if (currentLevel >= 0) TQ = Q->get(currentLevel); continue; }
    auto next = TQ->dequeue();
    PropertyRef pt = dynamic_cast<Property*>(&(*(next.first)));
    TermId nextid = next.second;
    std::cout << "dequeued " << pt->toString(getPolyManager()) << std::endl;
    if (!boundsChosen && !pt->hasFactor())
    {
      chooseBounds(currentLevel, pt, nextid);
      boundsChosen = true;
    }
    PH->handle(currentLevel, pt, nextid, this, nextSectionIdentifier);  
  }
  // TC->dump(getPolyManager());
  TB->dump(getPolyManager(),TC);
}

// Input: k - a level
//        target
//        source - no term in source is in target
// SideEffect: all terms in source of level < k are added to target,
//             and any term of higher level that can be proved by
//             terms from target (possibly after addition) are added.
void NuCADCell::foo(int newSplitLevel, char newSplitLabel, CollectiveTermBaseRef target, TermBaseRef source)
{
  std::vector<TermId> contents;
  source->fillWithTerms(contents);
  source->topologicalSort(contents);
  for(int i = 0; i < contents.size(); ++i)
  {
    TermId tid = contents[i];
    TermRef T = getTermContext()->get(tid);
    TermBaseDatum* tbdp = source->get(tid);
    prop::Tag tprop = T->getProperty();

    //-- Check out collectivetermbase.cpp for the genisis of this!
    int putativeTermLevel = T->level(getVarOrder());
    bool above = (newSplitLabel == 'S' || newSplitLabel == 'U');
    bool inherit = inheritAtLevel(tprop,putativeTermLevel,newSplitLevel,above);

    bool isCellBound = tbdp->getReason() == "cell-bound";
    
    std::cout << "checking " << T->toString(getPolyManager()) << " ... ";
    if (isCellBound && putativeTermLevel >= newSplitLevel)
    {
      std::cout << "*NOT* inherited! (cell-bound)" << endl;
    }
    else if ((isCheck(tprop) && inherit) || (isCellBound && putativeTermLevel < newSplitLevel))
    {
      target->add(tid,*(tbdp));
      std::cout << "inherited!" << endl;
    }
    else if (!isCheck(tprop) && !isCellBound)
    {
      bool tst = true;
      for(auto itr = tbdp->dedfBegin(); itr != tbdp->dedfEnd() && tst; ++itr)
	tst = tst && target->isIn(*itr);
      if (tst)
	target->add(tid,*(tbdp));
      std::cout << "prereqs are inherited!" << endl;
    }
    else
    {
      std::cout << "*NOT* inherited!" << endl;
    }
  }
}

NuCADCellRef NuCADCell::refineToChild(const std::vector<TermRef> &goals)
{
  // Find N - the presumed potential split level
  std::vector<TermRef> goalsp(goals);
  VarOrderRef V = getVarOrder();
  int N = 0;
  for(auto itr = goalsp.begin(); itr != goalsp.end(); ++itr)
    N = std::max(N,(*itr)->level(V));

  NuCADCellRef child = mkChild('X',N);
  
  // Fix this to work on the TermId vector rather than TermRef
  child->transferCellBoundsToGoals(N,goalsp);
  std::vector<TermId> goalsById;
  for(auto itr = goalsp.begin(); itr != goalsp.end(); ++itr)
    goalsById.push_back(getTermContext()->add(*itr));
  child->refineHelp(goalsById);

  // Now set to true split level
  int N_true = N;
  while(N_true > 0 && getLowerBoundAtLevel(N_true)->compare(child->getLowerBoundAtLevel(N_true)) == 0
	&& getUpperBoundAtLevel(N_true)->compare(child->getUpperBoundAtLevel(N_true)) == 0)
    N_true--;
  child->splitLevel = N_true;
  
  return child;
}

//-- split at level k
NuCADCellRef NuCADCell::siblingFromSplitWRTSample(SamplePointId sid, NuCADCellRef child, int k,
						  SectionRef S, bool splitBelow)
{
  if (splitBelow)
  {
    auto sL = S;
    SamplePointId nsidL = getSamplePointManager()->splitBelowSample(sid,sL,NULL);
    NuCADCellRef lchild = mkChild(nsidL,true,'L',k);
    for(int i = 1; i < k; i++)
    {      
      auto itrL = child->lowerBoundAtLevel.find(i);
      if (itrL != child->lowerBoundAtLevel.end()) { lchild->setLowerBoundAtLevel(i,itrL->second); }
      auto itrU = child->upperBoundAtLevel.find(1);
      if (itrU != child->upperBoundAtLevel.end()) { lchild->setUpperBoundAtLevel(1,itrU->second); }
    }
    lchild->setUpperBoundAtLevel(k,child->getLowerBoundAtLevel(k));
    lchild->setLowerBoundAtLevel(k,sL);
    TermId bid = getTermContext()->add(new Property( prop::sector,
						     new SectionTerm(lchild->getLowerBoundAtLevel(k)),
						     new SectionTerm(lchild->getUpperBoundAtLevel(k))));
    lchild->record(bid,"cell-bound");
    //-- lchild inherits from its upper sibling "child"
    lchild->foo(k,'L',lchild->getCollectiveTermBase(),child->getTermBase());
    return lchild;
  }
  else
  {
    auto sR = S;
    SamplePointId nsidR = getSamplePointManager()->splitAboveSample(sid,sR,NULL);
    NuCADCellRef rchild = mkChild(nsidR,true,'U',k);
    for(int i = 1; i < k; i++)
    {      
      auto itrL = child->lowerBoundAtLevel.find(i);
      if (itrL != child->lowerBoundAtLevel.end()) { rchild->setLowerBoundAtLevel(i,itrL->second); }
      auto itrU = child->upperBoundAtLevel.find(1);
      if (itrU != child->upperBoundAtLevel.end()) { rchild->setUpperBoundAtLevel(1,itrU->second); }
    }
    rchild->setLowerBoundAtLevel(k,child->getUpperBoundAtLevel(k));
    rchild->setUpperBoundAtLevel(k,sR);
    TermId bidr = getTermContext()->add(new Property( prop::sector,
						      new SectionTerm(rchild->getLowerBoundAtLevel(k)),
						      new SectionTerm(rchild->getUpperBoundAtLevel(k))));
    rchild->record(bidr,"cell-bound");
    //-- rchild inherits from its upper sibling "child"
    rchild->foo(k,'R',rchild->getCollectiveTermBase(),child->getTermBase());
    return rchild;
  }
}

NuCADCellRef NuCADCell::siblingFromBoundary(SamplePointId sid, NuCADCellRef child, int k,
					    SectionRef S, SamplePointId sidnew, bool splitBelow)
{
  char blabel = (splitBelow ? 'I' : 'S');
  NuCADCellRef bchild = mkChild(sidnew,true,blabel,k);
  for(int i = 1; i < k; i++)
  {      
    auto itrL = child->lowerBoundAtLevel.find(i);
    if (itrL != child->lowerBoundAtLevel.end()) { bchild->setLowerBoundAtLevel(i,itrL->second); }
    auto itrU = child->upperBoundAtLevel.find(1);
    if (itrU != child->upperBoundAtLevel.end()) { bchild->setUpperBoundAtLevel(1,itrU->second); }
  }
  bchild->setUpperBoundAtLevel(k,S);
  bchild->setLowerBoundAtLevel(k,S);
  SectionTermRef T_S = new SectionTerm(S);
  TermId bid = getTermContext()->add(new Property( prop::sector,T_S,T_S));
  bchild->record(bid,"cell-bound");
  //-- lchild inherits from its upper sibling "child"
  bchild->foo(k,(blabel == 'I' ? 'L' : 'U'),bchild->getCollectiveTermBase(),child->getTermBase());
  return bchild;
}


std::vector<NuCADCellRef> NuCADCell::split(NuCADCellRef child)
{
  std::vector<NuCADCellRef> resL, resR;
  int k = child->getSplitLevel();
  for(int i = 1; i <= k; ++i)
  {
    std::vector<NuCADCellRef> K = split(child,i);
    bool found = false;
    for(auto itr = K.begin(); itr != K.end(); ++itr)
      if (found)
	resR.push_back(*itr);
      else if (child.identical(*itr))
	found = true;
      else
	resL.push_back(*itr);
  }
  resL.push_back(child);
  for(int j = resR.size() - 1; j >= 0; --j)
    resL.push_back(resR[j]);
  return resL;
}
  
std::vector<NuCADCellRef> NuCADCell::split(NuCADCellRef child, int k)
{
  //  int k = child->getSplitLevel();
  VarOrderRef V = getVarOrder();
  SamplePointId sid = getSamplePointId();

  if (k == 0)
    return std::vector<NuCADCellRef>{ };
  else if (this->isSectionAtLevel(k))
  {
    throw TarskiException("In NuCADCell::split - This case should never happen!");
  }
  else if (child->isSectionAtLevel(k)) // child is section, parent is sector
  {
    NuCADCellRef lchild = siblingFromSplitWRTSample(sid,child,k,getLowerBoundAtLevel(k),true);
    NuCADCellRef rchild = siblingFromSplitWRTSample(sid,child,k,getUpperBoundAtLevel(k),false);
    return std::vector<NuCADCellRef>{ lchild, child, rchild };
  }
  else // child is sector
  {
    SectionRef S_lower = getLowerBoundAtLevel(k);
    SectionRef S_upper = getUpperBoundAtLevel(k);
    SectionRef Sc_lower = child->getLowerBoundAtLevel(k);
    SectionRef Sc_upper = child->getUpperBoundAtLevel(k);
    bool hasLower = S_lower->compare(Sc_lower) != 0;
    bool hasUpper = S_upper->compare(Sc_upper) != 0;

    NuCADCellRef lchild, rchild, ichild, schild;

    if (hasLower) {
      SamplePointId sidInf = getSamplePointManager()->samplePointOnSection(sid,Sc_lower,NULL);
      SamplePointId sidL = getSamplePointManager()->splitBelowSample(sidInf,S_lower,this);
      ichild = siblingFromBoundary(sid,child,k,Sc_lower,sidInf,true);
      lchild = siblingFromSplitWRTSample(sidL,child,k,S_lower,true); }

    if (hasUpper) {
      SamplePointId sidSup = getSamplePointManager()->samplePointOnSection(sid,Sc_upper,NULL);
      SamplePointId sidU = getSamplePointManager()->splitAboveSample(sidSup,S_upper,this);
      schild = siblingFromBoundary(sid,child,k,Sc_upper,sidSup,false);
      rchild = siblingFromSplitWRTSample(sidU,child,k,S_upper,false); }

    if (hasLower && hasUpper) return std::vector<NuCADCellRef>{ lchild, ichild, child, schild, rchild };
    else if (hasLower && !hasUpper) return std::vector<NuCADCellRef>{ lchild, ichild, child };
    else if (!hasLower && hasUpper) return std::vector<NuCADCellRef>{ child, schild, rchild };
    else return std::vector<NuCADCellRef>{ child };
  }
  
  return std::vector<NuCADCellRef>{ NULL };
}



void NuCADCellMOD::chooseBounds(int currentLevel, TermRef pt, TermId nextid)
{
  std::cerr << "In chooseBounds!" << std::endl;
  TermQueueRef TQ = Q->get(currentLevel);

  TermId bid;
  
  //-- Get all si, oi and sector terms and collect the upper/lower/pass-through sections for each
  vector<PropertyRef> V;
  V.push_back(pt);
  while(!TQ->empty())
    V.push_back(dynamic_cast<Property*>(&(*(TQ->dequeue().first))));

  std::vector<SectionRef> lower, upper, through;
  for(auto itr = V.begin(); itr != V.end(); ++itr)
  {
    PropertyRef pt = *itr;
    switch(pt->getProperty())
    {
    case prop::si: case prop::oi: {
      IntPolyRef p = pt->getChild(0)->getPoly();
      if (getSamplePointManager()->isNullifiedAt(getSamplePointId(),p)) break;
      auto res = getSamplePointManager()->boundingSections(getSamplePointId(),p);
      if (res.second.is_null())
	through.push_back(res.first);
      else
      {
	lower.push_back(res.first);
	upper.push_back(res.second);
      }
    }break;
    default:
      break;
    }
  }
  
  //-- CASE 1: No sections pass through alpha.  Choose upper & lower sector bounds for this level
  if (through.size() == 0)
  {
    int changeCountLower = 0;
    SectionRef lowerBound = getLowerBoundAtLevel(currentLevel);
    for(auto itr = lower.begin(); itr != lower.end(); ++itr)
      if (getSamplePointManager()->compareAtAlpha(getSamplePointId(),(*itr),lowerBound) > 0 && ++changeCountLower)
	lowerBound = *itr;	  
    int changeCountUpper = 0;
    SectionRef upperBound = getUpperBoundAtLevel(currentLevel);
    for(auto itr = upper.begin(); itr != upper.end(); ++itr)
      if (getSamplePointManager()->compareAtAlpha(getSamplePointId(),(*itr),upperBound) < 0 && ++changeCountUpper)
	upperBound = *itr;

    SectionTermRef lowsterm = new SectionTerm(lowerBound);
    SectionTermRef upsterm = new SectionTerm(upperBound);
    bid = getTermContext()->add(new Property(prop::sector, lowsterm, upsterm ));
    std::vector<TermRef> reqs;
    if (changeCountLower > 0) reqs.push_back(new Property(prop::ana_delin, new PolyTerm(lowerBound->getPoly())));
    if (changeCountUpper > 0) reqs.push_back(new Property(prop::ana_delin, new PolyTerm(upperBound->getPoly())));
    reqs.push_back(new Property(prop::leq, lowsterm, upsterm));
    record(bid,"cell-bound",reqs);	

    if (changeCountLower > 0)
      setLowerBoundAtLevel(currentLevel,lowerBound);
    if (changeCountUpper > 0)
      setUpperBoundAtLevel(currentLevel,upperBound);
  }
  
  //-- CASE 2: Some sections pass through alpha.  Choose defining section.
  else
  {
    // Choose the defining section using BPC ... could be better options, of course.
    SectionRef s = through[0];
    BasicPolyCompare BPC(getVarOrder());
    for(int i = 0; i < through.size(); i++)
      if (BPC.lt(through[i]->getPoly(),s->getPoly()))
	s = through[i];

    bid = getTermContext()->add( new Property(prop::sector,
						     new SectionTerm(s),
						     new SectionTerm(s)
						     ));
    record(bid,"cell-bound", std::vector<TermRef>{ new Property(prop::ana_delin, new PolyTerm(s->getPoly())) });
    setLowerBoundAtLevel(currentLevel,s);
    setUpperBoundAtLevel(currentLevel,s);
  }  

  //-- Return Queue to its original state
  for(int i = 1; i < V.size(); i++)
    TQ->enqueue(V[i]);

  std::cerr << "Leaving chooseBounds! Chose ... "
	    << getTermContext()->get(bid)->toString(getPolyManager())
	    << std::endl;
}

