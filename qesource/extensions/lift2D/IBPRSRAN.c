/*======================================================================
                      IBPRSRAN(M,I,B,k; L,t)

Integral bivariate polynomial refinement simple root over algebraic number

Inputs
   M : the minimal integral polynomial of an algebraic number
       \alpha.
   I : an open standard logarithmic isolating interval for \alpha.
   B : a bivariate polynomial such that B(alpha,y) is squarefree, and
       \alpha is a simple (i.e. multiplicity one) root of disc_y(B).
   Jorig : an isolating interval for a multiplicity one root of B(alpha,y)
   tr: trend for J as a root of B
   k : k is the target precision of refinement - i.e. we try to refine
       isolating intervals to a width of 1/2^k. LBRN interval.
Outputs
   Jp: if t = 0, an LBRN isolating interval refinement of J of width 1/2^k
       otherwise the smallest isolating interval that could be acheived.
   t : 0, 1
======================================================================*/
#include "lift2d.h"

Word LBRNILOW(Word J);

void IBPRSRAN(Word M, Word I, Word B, Word k, Word Jorig, Word tr, Word *Jp_, BDigit *t_)
{
        Word L,CFP,Bp,Ls,Lp,Ld,td,a,b,Jp;
	BDigit t,n,n1,n2,np,u,s,e,i,j,t1,tc,c,jnew;
	ieee F1,F2;
	double p,w1,w2;
	interval *A = NULL, K,*Q = NULL, *HICFP, J, Jnew;

	Jp = Jorig;
Step1: /* Convert the isolating interval for \alpha to a
          hardware interval. */
	L = NIL;
	LBRNIEEEE(FIRST(I), &t,&F1,&n1);
	if (t != 0)
	   goto Return;
	w1 = F1.num;
	LBRNIEEEE(SECOND(I), &t,&F2,&n2);
        if (t != 0)
           goto Return;
	w2 = F2.num;
	np = MIN(n1,n2);

Step2: /* Convert the minimal polynomial to a hardware interval
          polynomial and refine the hardware interval. */
	FPCATCH();
	IUPHIP(M,&A,&t); // If t != 0, remember to free A!
	if (t == 0) {
	  A = NULL;
	  t = 1;
	  goto Return; }
	n = PDEG(M);
	t = HIPFES(n,A,w2);
	if (FPCHECK() == 1) {
	   t = 1;
	   goto Return; }
	if (t == NIL) {
	   t = 2;
	   goto Return; }
	u = 0;
	while (u == 0 && np > 0) {
	   p = (w1 + w2) / 2.0;
	   s = HIPFES(n,A,p);
	   if ((FPCHECK() == 1) || (s == NIL))
	      u = 1;
	   else if (s == t)
	      w2 = p;
	   else if (s == -t)
	      w1 = p;
	   else {
	      w1 = p;
	      w2 = p; }
	   np = np - 1; }
	K.left = w1;
	K.right = w2;

Step3: /* Rfine */
	/* Get hardware interval array to store B(alpha,y) (init to zero) */
	Q = GETHIPARRAY(PDEG(B));
	for(i = 0; i < PDEG(B) + 1; i++)
	  IHI(0,&(Q[i]),&t); /* this can't fail! */
	
	/* Compute B(alpha,y) and store in Q */
	for(Bp = B; Bp != NIL; Bp = RED2(Bp)) {
	  FIRST2(Bp,&e,&CFP);
	  IUPHIP(CFP,&HICFP,&c);
	  Q[e] = HIPIEVAL(PDEG(CFP),HICFP,K);
	  if (c != 0)
	    FREEARRAY(HICFP);
	}

	/* Check leading coefficient */
	s = HISIGN(Q[PDEG(B)]);
	if (s == NIL) {
	  t = 3;
	  goto Return; }
	
	/* Compute j, current width. */
	j = -LBRNILOW(Jorig);

	LBRIHI(Jorig,&J,&t);
	if (t != 0)
	  goto Return;
	
	/***** (a,b) is an isolating interval for a single, simple root ***********/
	HIPIR(PDEG(B),Q,J,tr,j,k,&Jnew,&jnew);
	Jp = HILBRI(Jnew);
	if (jnew < k) {
	  t = 1;
	  goto Return;
	}

Return:	
	if (Q != NULL)
	  FREEARRAY(Q);
	if (A != NULL)
	  FREEARRAY(A);
	*t_ = t; 
	*Jp_ = Jp;
}
