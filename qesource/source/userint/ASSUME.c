/*======================================================================
                      ASSUME()

Add assumption
======================================================================*/
#include "qepcad.h"

void QepcadCls::ASSUME()
{
  Word F, s;
  QFFRDR(CINV(REDI(CINV(GVVL),GVNV - GVNFV)),&F,&s);
  if (s) {
    if (GVUA == NIL)
      GVUA = F;
    else if (FIRST(GVUA) != ANDOP)
      GVUA = LIST3(ANDOP,GVUA,F);
    else
      GVUA = CCONC(GVUA,LIST1(F));
  }
  else
    SWRITE("WARNING! Incorrect format:  No assumptions added!\n");  
}

