/*======================================================================
                      INPUTRD_ERROR(msg, errMode)

Input read error.

\Input
   \parm{msg} a C-string that is the error message.
   \parm{errMode} is 0->throw Exception, 1->write error message and return.

\Side Effect
   Handle error based on errMode
======================================================================*/
#include "qepcad.h"

void INPUTRD_ERROR(const char* msg, int errMode)
{
  if (errMode) { SWRITE(msg); FILINE(); }
  else { throw QepcadException(msg); }      
}
