/*======================================================================
                      INITIO()

Initialize Input/Output.
======================================================================*/
#include "qepcad.h"
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "db/readlineistream.h"

void InputContextInit(istream&);
void OutputContextInit(ostream&);

void INITIO(istream *is, ostream *os)
{
       Word i;

Step1: /* Initialize Input. */	  
       if (is == NULL) {
         if (isatty(0)) 
	   InputContextInit(*(new readlineIstream()));
         else
	   InputContextInit(std::cin);
       } else
           InputContextInit(*is);
       
Step2: /* Initialize Output. */
       if (os == NULL)
         OutputContextInit(std::cout);
       else
         OutputContextInit(*os);

Step3: /* Control Echo. */
       if (!isatty(0) && !NOECHOSWITCHSET)
         setECHO(1);
       else
         setECHO(0);

Return: /* Prepare for return. */
       return;
}
