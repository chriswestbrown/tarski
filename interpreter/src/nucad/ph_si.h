#ifndef _NUCAD_PH_SI_
#define _NUCAD_PH_SI_

#include "prophandler.h"

namespace tarski { 

  
 class PH_si : public PropHandler
 {
  public:
   void handle(
	       VarOrderRef V,
	       SamplePointManagerRef SM,
	       SamplePointId sid,
	       TermContextRef TC,
	       TermBaseRef TB,
	       LeveledTermQueueRef Q,
	       int currentLevel,
	       PropertyRef pt,
	       TermId nextid
	       )
   {
   }
  };
}
#endif
