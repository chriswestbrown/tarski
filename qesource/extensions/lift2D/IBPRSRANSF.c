/*======================================================================
                      IBPRSRANSF(M,I,B,k; L,t)

Integral bivariate polynomial refinement simple root over algebraic number (Software floats version)

Inputs
   M : the minimal integral polynomial of an algebraic number
       \alpha.
   I : an open standard logarithmic isolating interval for \alpha.
   B : a bivariate polynomial such that B(alpha,y) is squarefree, and
       \alpha is a simple (i.e. multiplicity one) root of disc_y(B).
   Jorig : a logarithmic binary rational isolating interval for 
           a multiplicity one root of B(alpha,y) 
   tr: trend for J as a root of B
   k : k is the target precision of refinement - i.e. we try to refine
       isolating intervals to a width of 1/2^k. LBRN interval.
   p : precision in software floats to use
Outputs
   Jp: if t = 0, an LBRN isolating interval refinement of J of width 1/2^k
       otherwise the smallest isolating interval that could be acheived.
   t : 0, 1
======================================================================*/
#include "lift2d.h"

Word LBRNILOW(Word J);

void IBPRSRANSF(Word M, Word I, Word B, Word k, Word Jorig, Word tr, Word p, Word *Jp_, BDigit *t_)
{
        BDigit *Mp,*bp,*c = NULL,i,m,n,q1,q2,S,s,t,tc;
	Word a,b,Bp,I1,I2,J,K,L,Ls,Lp,T,Jp,Js,Ld;

	
	Jp = Jorig;
	
Step1: /* Convert the minimal polynomial to a software interval
	   polynomial. */
	n = PDEG(M);
	q1 = p + 3;
	q2 = q1 + q1;
	S = (n + 1) * q2 + 1;
	Mp = GETARRAY(S);
	IPSIP(M,p,Mp);

Step3: /* Refine the isolating interval for \alpha. */
        J = SIPIR(Mp,I,t,- (p * ZETA));
        FREEARRAY(Mp);

Step4: /* Refine root of B */
	L = NIL;
	m = PDEG(B);
	s = (m + 1) * q2 + 1;
	c = GETARRAY(s);
	IBPELBRISIPR(B,J,p,c);
	Js = SIPIR(c,Jp,tr,-k);
	Word kact = -LBRNILOW(Js);

Return:	
	FREEARRAY(c);
	*t_ = (kact >= k);
	*Jp_ = Js;
}
