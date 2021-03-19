#ifndef _NUCAD_PROP_HANDLER_
#define _NUCAD_PROP_HANDLER_

#include "property.h"
#include "termbase.h"
#include "termqueue.h"
#include "samplePointManager.h"

namespace tarski { 

  class PropHandler;
  typedef GC_Hand<PropHandler> PropHandlerRef;

  class NuCADCell;
  typedef GC_Hand<NuCADCell> NuCADCellRef;
  
  class PropHandler : public GC_Obj
  {
  public:
    virtual void handle(int currentLevel, PropertyRef pt, TermId nextid, NuCADCellRef c,
			int& nextSectionIdentifier) = 0;
  };

 class TempPropHandler : public PropHandler
 {
  public:
   void handle(int currentLevel, PropertyRef pt, TermId nextid, NuCADCellRef c,
			int& nextSectionIdentifier);
 };

 class TempPropHandlerMOD : public PropHandler
 {
  public:
   void handle(int currentLevel, PropertyRef pt, TermId nextid, NuCADCellRef c,
			int& nextSectionIdentifier);
 };


}
#endif
