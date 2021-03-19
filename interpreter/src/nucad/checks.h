#ifndef _NUCAD_CHECKS_
#define _NUCAD_CHECKS_

#include "property.h"
#include "samplePointManager.h"

namespace tarski { 

  namespace prop {
  
    bool check(prop::Tag p, IntPolyRef f, SamplePointManagerRef SM, SamplePointId sid);
    bool check(prop::Tag p, SectionRef s, SamplePointManagerRef SM, SamplePointId sid);
    bool check(prop::Tag p, SectionRef s0, SectionRef s1, SamplePointManagerRef SM, SamplePointId sid);
    bool check(TermRef T, SamplePointManagerRef SM, SamplePointId sid);
  };

};

#endif
