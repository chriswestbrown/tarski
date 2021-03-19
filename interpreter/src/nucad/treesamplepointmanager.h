#ifndef _TREE_SAMPLE_POINT_MANAGER_
#define _TREE_SAMPLE_POINT_MANAGER_
#include "samplePointManager.h"

namespace tarski {

  class SPNode;
  typedef GC_Hand<SPNode> SPNodeRef;

  class SPNode : public GC_Obj
  {
  public:
    SPNode(int level, SPNode* parent, RealAlgNumRef x);
    const int rational = 0, root = 1, primitive = 2, multiple = 3;
    virtual int getLevel();
    virtual SPNode* getParent();
    virtual int samplePointNumberOfExtensions(int level);
    virtual SPNodeRef addRealOver(RealAlgNumRef P, int k); //-- k is the level of the new coordinate P
    virtual int coordKind(int level);
    virtual SamplePointId getSamplePointId() { return sid; }
    virtual void setSamplePointId(SamplePointId sid) { this->sid = sid; }
    virtual std::string toStr();

    // this node represnts a sample point.  the function gets the first k coordinates of the sample
    // point as rational numbers and returns them as a list.  An exception is thrown if this fails
    virtual Word getRationalSampleUpTo(int k);
    
  private:    
    class ChildCompare
    {
    public:
      inline int comp(const std::pair<RealAlgNumRef,int> &a, const std::pair<RealAlgNumRef,int> &b )
      {
	if (a.second == b.second) return  a.first->compareTo(b.first);
	return a.second < b.second ? -1 : +1;
      }
      inline bool operator()(const std::pair<RealAlgNumRef,int> &a, const std::pair<RealAlgNumRef,int> &b )
      {
	return a.second < b.second || a.first->compareTo(b.first) < 0;
      }
    };
    std::map< std::pair<RealAlgNumRef,int>, SPNodeRef, ChildCompare  > child; //-- child[alpha,change-in-level] = SPNode*
    int level;
    SPNode* parent;
    RealAlgNumRef me;
    SamplePointId sid;
    
  protected:
    virtual int myKind();
  };

  
  

  class TreeSamplePointManagerObj : public SamplePointManagerObj
  {
  public:
    TreeSamplePointManagerObj(VarOrderRef varOrder);
  
    SamplePointId addRationalOver(SamplePointId s, GCWord P, int k);

    SamplePointId addRealOver(SamplePointId s, RealAlgNumRef P, int k);
  
    int polynomialSignAt(SamplePointId s, IntPolyRef p);

    bool isNullifiedAt(SamplePointId s, IntPolyRef p);

    std::pair<SectionRef,SectionRef> boundingSections(SamplePointId s, IntPolyRef p);

    int compareAtAlpha(SamplePointId id_alpha, SectionRef s1, SectionRef s2);

    int compareToAlpha(SamplePointId id_alpha, SectionRef s1);

    SamplePointId splitBelowSample(SamplePointId id_alpha, SectionRef s1, WithBoundsInterface* bip);

    SamplePointId splitAboveSample(SamplePointId id_alpha, SectionRef s1, WithBoundsInterface* bip);

    SamplePointId samplePointOnSection(SamplePointId id_alpha, SectionRef S, WithBoundsInterface* bip);

    SamplePointId completeSample(SamplePointId sid, int k, WithBoundsInterface* bip);
  
    void debugDump();

    std::string toStr(SamplePointId sid);

    inline SPNodeRef getNode(SamplePointId sid) { return sid < idToSamplePoint.size() ? idToSamplePoint[sid] : NULL; }

  private:
    SPNodeRef root;
    std::vector<SPNodeRef> idToSamplePoint;
    
  };

}

#endif

