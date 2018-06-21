#ifndef _NUCAD_PROPERTY_
#define _NUCAD_PROPERTY_

#include "../poly/poly.h"
#include "samplePointManager.h"

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
    narrowed , // went through the narrowing process
    
    // checks
    nz_alpha  ,    // non-zero at alpha
    nnull_alpha , // non-null at alpha
    null_alpha ,  // null at alpha
    constant  ,    // is a constant
    level1   ,  // is a level 1 poly
    
    // non-irreducibles
    si_s, // sign invariant for possibly reducibles
    oi_s, // order invariant for possibly reducibles
    ni_s, // null invariant for possibly reducibles
    
    // error
    err 
  };

  inline bool isCheck(Tag p) { return nz_alpha <= p && p <= level1; }
  inline bool isNirGoal(Tag p) { return si_s <= p && p <= ni_s; }
  bool check(Tag p, GoalContextRef GC, IntPolyRef P);
  string name(Tag p);
  Tag nameToProp(const string& str);
}

}
#endif
