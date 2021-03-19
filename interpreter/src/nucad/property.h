#ifndef _NUCAD_PROPERTY_
#define _NUCAD_PROPERTY_

#include "../poly/poly.h"

namespace tarski { 

class GoalContextObj;
typedef GC_Hand<GoalContextObj> GoalContextRef;


namespace prop
{
  enum Tag {
    // standard
    si = 0,        // sign invariant
    oi ,        // order invariant
    ni ,        // null invariant
    ana_delin , // analytic delineable
    sector,     // sector
    section,     // section
    leq, // <=
    lt, // < 
    narrowed , // went through the narrowing process
    
    // checks (polynomial checks)
    nz_alpha  ,    // non-zero at alpha
    nnull_alpha , // non-null at alpha
    null_alpha ,  // null at alpha
    constant  ,    // is a constant
    level1   ,  // is a level 1 poly
    // checks (single section checks)
    above_alpha , // (above_alpha s), where s is a section
    on_alpha , // (on_alpha s), where s is a section
    below_alpha , // (below_alpha s), where s is a section
    // checks (double section checks)
    leq_alpha, // (<=_alpha S1 S2)
    lt_alpha, // (<_alpha S1 S2)
    
    // non-irreducibles
    si_s, // sign invariant for possibly reducibles
    oi_s, // order invariant for possibly reducibles
    ni_s, // null invariant for possibly reducibles
    
    // error
    err 
  };

  bool isPolynomialCheck(Tag p);

  inline bool isSingleSectionCheck(Tag p) { return above_alpha <= p && p <= below_alpha; }
  inline bool isDoubleSectionCheck(Tag p) { return leq_alpha <= p && p <= lt_alpha; }

  inline bool isCheck(Tag p) { return nz_alpha <= p && p <= lt_alpha; }
  inline bool isFactor(Tag p) { return si_s <= p && p <= ni_s; }
  string name(Tag p);
  Tag nameToProp(const string& str);

  //-- these address the inheritability of an "at-alpha" property of level k for a sample
  //-- point that has the same coordinates of level < k, and a level-k coordinate below (resp. above)
  //-- alpha at level k
  inline bool inheritAtLevel(Tag p, int putativeLevel, int minSplitLevel, bool above)
  {
    if (!isCheck(p)) return true;
    if (above) {
      switch(p) {
	//-- these act at a level below the polynomial's level 
      case nnull_alpha: case null_alpha: case leq_alpha: case lt_alpha:
      case below_alpha: //-- this is inherited at the putative level because "above" splits preserve below-ness
	return putativeLevel <= minSplitLevel; break;
      case level1: //-- this is independent of sample point
       return true; break; 
      default: //-- everything else is only inherited if its level is lower
	return putativeLevel < minSplitLevel; break;
      }
    }
    else { // below
      switch(p) {
	//-- these act at a level below the polynomial's level
      case nnull_alpha: case null_alpha: case leq_alpha: case lt_alpha:
      case above_alpha: //-- this is inherited at the putative level because "below" splits preserve above-ness
	return putativeLevel <= minSplitLevel; break;
      case level1: //-- this is independent of sample point
       return true; break; 
      default: //-- everything else is only inherited if its level is lower
	return putativeLevel < minSplitLevel; break;
      }
    }
  }
}

}
#endif
