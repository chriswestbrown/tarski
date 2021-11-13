/*======================================================================
                         SETUPSYS()

Set up the System.
======================================================================*/
#include "qepcad.h"


void SETUPSYS()
{

Step1: /* Initialize IO. */
       INITIO(NULL, NULL);

Step2: /* Read in Help file. */
       HELPFRD();

Return: /* Prepare for return. */
       return;
}
   
   
