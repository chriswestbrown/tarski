#ifndef _TEST_SAMPLE_POINT_MANAGER_
#define _TEST_SAMPLE_POINT_MANAGER_
#include "samplePointManager.h"

namespace tarski {
  class TestSamplePointManagerObj : public SamplePointManagerObj
  {
  public:
    TestSamplePointManagerObj(VarOrderRef varOrder);
  
    // register's point (s_1,...,s_{k},P_1,...,P_n)
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
  
  private: // This is just for early test purposes!
    bool testIdentical(SamplePointId s, std::vector<RealAlgNumRef>& newP);
    std::vector< std::vector<RealAlgNumRef> > store;  

    std::vector< std::map<SectionRef,RealAlgNumRef> > sectionValues;

    // This is always assuming the map key is a cannonical poly
    std::vector< std::map<IntPolyRef,std::vector<RealAlgNumRef> > > rootsMap;

    // This is always assuming the map key is a cannonical poly
    std::unordered_map<void*,char> signForCanonicalIntPoly; // NOTE: -1,0,+1 with other values reserved


    //-- PRIVATE METHODS -----------------------------------------------------------//
    IntPolyRef partialEval(SamplePointId s, IntPolyRef p, int kp);  
    void registerSectionValue(SamplePointId alpha, SectionRef S1, RealAlgNumRef beta);
    RealAlgNumRef fetchSectionValueOverAlpha(SamplePointId alpha, SectionRef S1);

    // NOTE: p must be cannonical
    std::vector<RealAlgNumRef>& fetchRootsOverAlpha(SamplePointId s, IntPolyRef p);
  
    RealAlgNumRef getCoordinate(SamplePointId s, int level);
  };

}

#endif

