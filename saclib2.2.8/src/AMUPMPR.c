/*======================================================================
                      AMUPMPR(M,I,B,J,L; Js,j)

Algebraic module univariate polynomial minimal polynomial of a real root.

Inputs
  M  : in Z[X], the minimal polynomial of an algebraic number alpha.  
  I  : an acceptable isolating interval for alpha.  
  J  : a Binary rational interval which is either left-open and right-closed
       or a one-point interval.
  B  : in Z[alpha,X]. B has a unique root beta of odd multiplicity 
       in J.  
  L  : is a nonempty list (L1,...,Lt) of positive irreducible elements of
       Z[X]. Exactly one Li has beta as a root.

Outputs
  Js : a Binary rational interval. Js is a subinterval of J which is an
       isolating interval for beta as a root of Lj. Js is either left-open
       and right-closed or a one-point interval.
  j  : a BETA-digit. Lj is the unique element of L having beta as a root.
======================================================================*/
#include "saclib.h"

void AMUPMPR(M,I,B,J,L, Js_,j_)
       Word M,I,B,J,L, *Js_,*j_;
{
       Word Js,L1,Lp,a,b,c,j,jp,s,t,v,vp;
       /* hide L1,Lp,j,jp,s,t,v,vp; */

Step1: /* Initialize. */
       FIRST2(J,&a,&b);
       t = AMUPBES(M,I,B,b);
       if (t == 0)
         goto Step4;

Step2: /* Test for real roots of each Li in current interval. */
       v = 0;
       j = 0;
       jp = 0;
       Lp = L;
       Js = LIST2(a,b);
       do
         {
         ADV(Lp,&L1,&Lp);
         jp = jp + 1;
         vp = IUPVOI(L1,Js);
         if (vp > 1)
           goto Step3;
         if (vp == 1)
           if (v == 1)
             goto Step3;
           else
             {
             v = 1;
             j = jp;
             }
         }
       while (!(Lp == NIL));
       goto Return;

Step3: /* Bisect current interval. */
       c = RIB(a,b);
       s = AMUPBES(M,I,B,c);
       if (s == 0)
         {
         b = c;
         goto Step4;
         }
       else
         if (s * t < 0)
           a = c;
         else
           {
           b = c;
           t = s;
           }
       goto Step2;

Step4: /* B has root at right end point of current interval. */
       j = 0;
       Js = LIST2(b,b);
       Lp = L;
       do
         {
         ADV(Lp,&L1,&Lp);
         j = j + 1;
         if (PDEG(L1) == 1)
           if (IUPBES(L1,b) == 0)
             goto Return;
         }
       while (1);

Return: /* Prepare for return. */
       *Js_ = Js;
       *j_ = j;
       return;
}
