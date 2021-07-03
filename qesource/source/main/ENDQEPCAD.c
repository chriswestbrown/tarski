/*======================================================================
                      ENDQEPCAD()

Clean up the qepcad system.
======================================================================*/
#include "qepcad.h"
#include "caserver/CAPolicy.h"
extern ServerBase GVSB;
extern bool useExistingCAServer;

void ENDQEPCAD()
{
  if (!useExistingCAServer) {
    /* Delete the CAServers ... mostly to let them kill child processes! */
    for(ServerBase::iterator p = GVSB.begin(); p != GVSB.end(); ++p)    
      delete p->second;
    GVSB.clear();
    
    if (GVCAP != 0) { delete GVCAP; GVCAP = 0; }
  }
  useExistingCAServer = false;
  
  if (GVContext != 0) { delete GVContext; GVContext = 0; }

Return: /* Prepare for return. */
       return;
}
