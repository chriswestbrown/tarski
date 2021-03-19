#ifndef _NUCAD_CELL_
#define _NUCAD_CELL_

#include "property.h"
#include "goal.h"
#include "termbase.h"
#include "termqueue.h"
#include "../poly/factor.h"
#include <algorithm>


namespace tarski { 

  class NuCADCell;
  typedef GC_Hand<NuCADCell> NuCADCellRef;

  class CollectiveTermBase;
  typedef GC_Hand<CollectiveTermBase> CollectiveTermBaseRef;
  
  class NuCADCell : public GC_Obj, public WithBoundsInterface
  {
  public:
    NuCADCell(SamplePointManagerRef SM, SamplePointId sid, bool samplePointIsUnfinished,
	      TermContextRef TC,
	      NuCADCell* parent, char splitLabel = 'C', int splitLevel = 0 );

    void record( TermId conclusion, const string &reason );
    void record( TermId conclusion, const string &reason, const std::vector<TermId>& froms );
    void record( TermId conclusion, const string &reason, const std::vector<TermRef>& froms );
    
    // When the level 'currentLevel' queue contains only terms with section variables,
    // this function gets called.  pt/nextid is the term that has just been dequeued
    // from that level ... there may or may not be more terms in the queue at that level.
    void chooseSectionVariableValues(int currentLevel, TermRef pt, TermId nextid);

    virtual void refineHelp(const std::vector<TermId> &goals);
    virtual void refineHelp(const std::vector<TermRef> &goals);
    virtual NuCADCellRef refineToChild(const std::vector<TermRef> &goals);

    virtual std::vector<NuCADCellRef> split(NuCADCellRef child);
    virtual std::vector<NuCADCellRef> split(NuCADCellRef child, int k);
    
    
    SamplePointManagerRef getSamplePointManager() { return SM; }
    VarOrderRef getVarOrder() { return SM->getVarOrder(); }
    PolyManager* getPolyManager() { SM->getPolyManager(); }

    SamplePointId getSamplePointId();
    bool isSamplePointUnfinished() { return samplePointIsUnfinished; }
    TermContextRef getTermContext() { return TC; }
    TermBaseRef getTermBase() { return TB; }
    CollectiveTermBaseRef getCollectiveTermBase();
    
    LeveledTermQueueRef getQ() { return Q; }
    
    NuCADCellRef getParent() { return parent; }
    SectionRef getLowerBoundAtLevel(int k);
    void setLowerBoundAtLevel(int k, SectionRef T);
    SectionRef getUpperBoundAtLevel(int k);
    void setUpperBoundAtLevel(int k, SectionRef T);
    bool isSectionAtLevel(int k);
    void foo(int newSplitLevel, char newSplitLabel, CollectiveTermBaseRef target, TermBaseRef source);

    std::string cellDescrip();
    int getMaxLevel();

    virtual NuCADCell* mkChild(char newSplitLabel = 'C', int newSplitLevel = 0)
    {
      return new NuCADCell(getSamplePointManager(),getSamplePointId(), false,
			   getTermContext(),
			   this,newSplitLabel,newSplitLevel);
    }
    virtual NuCADCell* mkChild(SamplePointId nsid, bool samplePointIsUnfinished,
			       char newSplitLabel, int newSplitLevel)
    {
      return new NuCADCell(getSamplePointManager(),nsid, false, getTermContext(),
			   this,newSplitLabel,newSplitLevel);
    }

    //-- remove all the sector terms defining the cell-bounds
    //-- (of level N down) from the term-base and add them as goals.
    void transferCellBoundsToGoals(int N, std::vector<TermRef> &goals);

    //-- returns true if term is in termbase
    int checkStatus(TermId tid);

    //-- returns -1/0/+1 according to the sign of p at the cell's sample point
    int signAtSamplePoint(IntPolyRef p);

    int getSplitLevel() const { return splitLevel; }
    char getSplitLabel() const { return splitLabel; }

    //-- This is really only for internal use, and should probably be private or protected
    NuCADCellRef siblingFromSplitWRTSample(SamplePointId sid, NuCADCellRef child, int k,
					   SectionRef S, bool splitBelow);

    //-- This is really only for internal use, and should probably be private or protected
    NuCADCellRef siblingFromBoundary(SamplePointId sid, NuCADCellRef child, int k,
				     SectionRef S, SamplePointId sidnew, bool splitBelow);

  private:
    SamplePointManagerRef SM;

    //-- Note: Sample point semantics are that a cell may be constructed with a full sample
    //--       point or only a partial sample point.  The partial sample point should be
    //--       complete up to and including the split level, but unspecified above.  When
    //--       the full sample point is first needed, the getSamplePointId method
    SamplePointId mysid_ONLY_USE_VIA_GETTER;
    bool samplePointIsUnfinished;


    TermContextRef TC;
    NuCADCell* parent; // this is a back reference!
    char splitLabel; // LIXSU Lower Infimum X Supremum Upper 
    int splitLevel;
    TermBaseRef TB;
    std::map<void*, TermId> waitingOn;
    CollectiveTermBaseRef CTB;
    
    // bounds
    int maxLevel;
    std::unordered_map<int,SectionRef> lowerBoundAtLevel;
    std::unordered_map<int,SectionRef> upperBoundAtLevel;

  public:
    LeveledTermQueueRef Q;

  };

  

  // This variant of NuCADCell chooses the cell bounds immediately after
  // all the factors have been dealt with.  This should obviate the need
  // for section variables, and make it easier to figure out how to make
  // use of equational constraints
  class NuCADCellMOD : public NuCADCell
  {
  public:
  NuCADCellMOD(SamplePointManagerRef SM, SamplePointId sid, bool samplePointIsUnfinished,
		 TermContextRef TC,
		 NuCADCell* parent, char splitLabel = 'C', int splitLevel = 0) :
      NuCADCell(SM,sid,samplePointIsUnfinished,TC,parent,splitLabel,splitLevel) { }

    void refineHelp(const std::vector<TermId> &goals);
    //    NuCADCellRef refineToChild(const std::vector<TermRef> &goals);
    void chooseBounds(int currentLevel, TermRef pt, TermId nextid);
    virtual NuCADCellMOD* mkChild(char newSplitLabel = 'C', int newSplitLevel = 0)
    {
      return new NuCADCellMOD(getSamplePointManager(),getSamplePointId(), isSamplePointUnfinished(), getTermContext(),
			      this,newSplitLabel,newSplitLevel);
    }
    virtual NuCADCellMOD* mkChild(SamplePointId nsid, bool samplePointIsUnfinished,
				  char newSplitLabel, int newSplitLevel)
    {
      return new NuCADCellMOD(getSamplePointManager(),nsid, samplePointIsUnfinished, getTermContext(),
			      this,newSplitLabel,newSplitLevel);
    }
  };
  
}

#endif
