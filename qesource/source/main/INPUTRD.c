/*======================================================================
                      INPUTRD(; Fs,V, errMode)

Input formula read.

\Output
   \parm{F*} is a formula.
   \parm(V)  is a variable list.
   \parm{errMode} is 0->throw Exception, 1->write error message and return.

======================================================================*/
#include "qepcad.h"

void INPUTRD(Word *Fs_,Word *V_, int errMode)
{
       Word C,Fs,V,f,r,t;
       /* hide C,r,t; */

Step1: /* Read in a description. */
       do
         {
         PROMPT(); SWRITE("Enter an informal description ");
         SWRITE(" between '[' and ']':\n"); 
         C = CREADB();
         if (C != '[')
	 {
           INPUTRD_ERROR("Error INPUTRD: '[' was expected.\n",errMode);
           t = 0;
           }
         else
           t = 1;
         }
       while (!(t == 1));
       BKSP();  GVMCOMM = COMMNT();

Step2: /* Read a variable list. */
       do
         {
         PROMPT(); SWRITE("Enter a variable list:\n"); FILINE();
         VLREADR(&V,&t);
         if (t == 1 && ISNIL(V))
	 {
           INPUTRD_ERROR("Error INPUTRD: There should be at least one variable.\n",errMode);
           t = 0;
           }
         }
       while (!(t == 1));
       r = LENGTH(V);

Step3: /* Read in the number of free variables. */
       do
         {
         PROMPT(); SWRITE("Enter the number of free variables:\n"); FILINE();
         GREADR(&f,&t);
         if (t == 1 && f < 0)
	 {
	   INPUTRD_ERROR("Error INPUTRD: the number of free variables must be non-negative.\n",errMode);
           t = 0;
           }
         if (t == 1 && f > r)
	 {
	   char emsg[128] = {'\0'};
	   sprintf(emsg,"Error INPUTRD: the number of free variables must be <= %d\n",r);
	   INPUTRD_ERROR(emsg,errMode);
           t = 0;
	 }
         }
       while (!(t == 1));

Step4: /* Read in a formula. */
       do
         {
         PROMPT(); SWRITE("Enter a prenex formula:\n"); FILINE();
         FREADR(V,f,&Fs,&t);
         }
       while (!(t == 1));
       SWRITE("\n\n=======================================================\n");

Return: /* Prepare for return. */
       *Fs_ = Fs;
       *V_ = V;
       return;
}




