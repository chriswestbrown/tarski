#ifndef _SAMPLE_POINT_MANAGER_
#define _SAMPLE_POINT_MANAGER_
#include "../onecell/varorder.h"
#include "../realroots/realroots.h"
#include "../poly/poly.h"
#include "section.h"
#include "termbase.h"

namespace tarski {

  /*
    SamplePointManager
    The basic idea is to let all outside code deal with a sample point simply 
    as an index (SamplePointId).  Each index refers to a point in an essentially
    infinite dimensional space - with all coordinates after the last explicitly
    represented coordinate being zero.

    Invariants:
    - If sample point SP has its last explicitly represented coordinate at level k,
      then the level k coordinate is not zero.
   */

  class WithBoundsInterface
  {
  public:
    virtual SectionRef getLowerBoundAtLevel(int k) = 0;
    virtual SectionRef getUpperBoundAtLevel(int k) = 0;
    virtual int getMaxLevel() = 0;
  };

  
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

  // registers point (s_1,...,s_{k},P_1,...,P_n)
  virtual SamplePointId addRationalOver(SamplePointId s, GCWord P, int k) = 0;

  // register's point (s_1,...,s_{k},P_1,...,P_n)
  virtual SamplePointId addRealOver(SamplePointId s, RealAlgNumRef P, int k) = 0;

  // p must be irreducible and the canonical reference
  virtual int polynomialSignAt(SamplePointId s, IntPolyRef p) = 0;

  virtual int factorSignAt(SamplePointId s, FactRef f);

  virtual bool isNullifiedAt(SamplePointId s, IntPolyRef p) = 0;

  // p must be irreducible and the canonical reference
  virtual std::pair<SectionRef,SectionRef> boundingSections(SamplePointId s, IntPolyRef p) = 0;
 
  virtual void debugDump() = 0;
  virtual std::string toStr(SamplePointId sid) = 0;
  
  //-- sample points for sections ----------------------------------------------//

  // returns -1,0,1 as s1 and s2 compare at alpha.  Note: s1 and s2 *must* have the same level
  virtual int compareAtAlpha(SamplePointId id_alpha, SectionRef s1, SectionRef s2) = 0;

  // returns -1,0,1 as s1 compares to alpha.  Specifically: let k be the level of S1,
  // we are comparing S1(alpha_1,...,alpha_{k-1}) to alpha_k.  So -1 means S1 is "below" alpha,
  // 0 means the section runs through alpha, and +1 means S1 is "above" alpha
  virtual int compareToAlpha(SamplePointId id_alpha, SectionRef s1) = 0;

  // Input:  sample point a and section s1.  let k be the level of s1 
  //         over (a1,...,a_{k-1}) the section value must be less than a_k
  // Output: sample point b s.t. (a1,...,a_{k-1}) = (b1,...,b_{k-1}) and
  //         s1(a1,...,a_{k-1}) < b_k < a_k, and b_k rational
  virtual SamplePointId splitBelowSample(SamplePointId id_alpha, SectionRef s1, WithBoundsInterface* bip) = 0;

  virtual SamplePointId splitAboveSample(SamplePointId id_alpha, SectionRef s1, WithBoundsInterface* bip) = 0;

  // Input : sample point alpha and section S, s.t. S is known to be defined over alpha (let k be S's level)
  // Output: sample point b s.t. (a1,...,a_{k-1}) = (b1,...,b_{k-1}), b_k = S(b_1,...,b_{k-1}), and
  //         b_{k+1},...,b_n is within the bounds given by bip.
  virtual SamplePointId samplePointOnSection(SamplePointId id_alpha, SectionRef S, WithBoundsInterface* bip) = 0;

  // sid is a sample point that is only correct up to level k.  completeSample returns a new sample
  // that agrees with sid on levels 1,...,k, but is chosen from bip for all higher levels defined by bip.
  virtual SamplePointId completeSample(SamplePointId sid, int k, WithBoundsInterface* bip) = 0;
  
  
  //-- terms -------------------------------------------------------------------//

  // Input:  sample point sid and term T
  // Output: if T is a "check", i.e. it is a property that is local to the sample point only
  //         or not related to any particular point or region in space (e.g. level), this will
  //         check the property, returning TRUE or FALSE.  If the input property is not of this
  //         type, UNDET is returned.
  int check(SamplePointId sid, TermRef T);
};



}

#endif

