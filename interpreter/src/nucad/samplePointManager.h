#ifndef _SAMPLE_POINT_MANAGER_
#define _SAMPLE_POINT_MANAGER_
#include "../onecell/varorder.h"
#include "../realroots/realroots.h"
#include "../poly/poly.h"
#include "section.h"

namespace tarski {

class SamplePointManagerObj;
typedef GC_Hand<SamplePointManagerObj> SamplePointManagerRef;

typedef int SamplePointId; // note: 0 is the SamplePointId of the origin

class SamplePointManagerObj : public GC_Obj
{
  VarOrderRef varOrder;
public:
  SamplePointManagerObj(VarOrderRef varOrder) { this->varOrder = varOrder; }
  virtual ~SamplePointManagerObj() { }

  VarOrderRef getVarOrder() const { return varOrder; }
  PolyManager* getPolyManager() const { return varOrder->getPolyManager(); }

  // register's point (s_1,...,s_{k},P_1,...,P_n)
  virtual SamplePointId addRationalOver(SamplePointId s, GCWord P, int k) = 0;

  virtual int polynomialSignAt(SamplePointId s, IntPolyRef p) = 0;

  virtual bool isNullifiedAt(SamplePointId s, IntPolyRef p) = 0;

  // p must be irreducible and the canonical reference
  virtual void roots(SamplePointId s, IntPolyRef p) = 0;
  
  virtual void debugDump() = 0;

  //-- sample points for sections

  // returns -1,0,1 as s1 and s2 compare at alpha.  Note: s1 and s2 *must* have the same level
  virtual int compareAtAlpha(SamplePointId id_alpha, SectionRef s1, SectionRef s2) = 0;

  // returns -1,0,1 as s1 compares to alpha.  Specifically: let k be the level of S1,
  // we are comparing S1(alpha_1,...,alpha_{k-1}) to alpha_k.  So -1 means S1 is "below" alpha,
  // 0 means the section runs through alpha, and +1 means S1 is "above" alpha
  virtual int compareToAlpha(SamplePointId id_alpha, SectionRef s1) = 0;
};


class TestSamplePointManagerObj : public SamplePointManagerObj
{
public:
  TestSamplePointManagerObj(VarOrderRef varOrder);
  
  // register's point (s_1,...,s_{k},P_1,...,P_n)
  SamplePointId addRationalOver(SamplePointId s, GCWord P, int k);
  
  IntPolyRef partialEval(SamplePointId s, IntPolyRef p, int kp);  
  
  int polynomialSignAt(SamplePointId s, IntPolyRef p);

  bool isNullifiedAt(SamplePointId s, IntPolyRef p);

  void debugDump();

  void roots(SamplePointId s, IntPolyRef p);

  int compareAtAlpha(SamplePointId id_alpha, SectionRef s1, SectionRef s2);

  int compareToAlpha(SamplePointId id_alpha, SectionRef s1);
  
private: // This is just for early test purposes!
  bool testIdentical(SamplePointId s, std::vector<RealAlgNumRef>& newP);
  std::vector< std::vector<RealAlgNumRef> > store;  

  std::vector< std::map<SectionRef,RealAlgNumRef> > sectionValues;

  // This is always assuming the map key is a cannonical poly
  std::vector< std::map<IntPolyRef,std::vector<RealRootIUPRef> > > rootsMap;


  void registerSectionValue(SamplePointId alpha, SectionRef S1, RealAlgNumRef beta);
  RealAlgNumRef fetchSectionValueOverAlpha(SamplePointId alpha, SectionRef S1);

  // NOTE: p must be cannonical
  std::vector<RealRootIUPRef>& fetchRootsOverAlpha(SamplePointId s, IntPolyRef p);
  

};

}

#endif

