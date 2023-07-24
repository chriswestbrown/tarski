/*======================================================================
                      modIBPRRIOAPSF(M,I,B,p,k, J_,L_)

Integral bivariate polynomial real root isolation over an algebraic
point - software floating point version (This is modified so that it
keeps track of fail intervals.)

Inputs
   A : the minimal integral polynomial of an algebraic number
       \alpha.
   I : a logarithmic open standard isolating interval for \alpha.
   B : an integral bivariate
       polynomial of degree less than n.
   p : a positive beta integer, the precision of the software
       arithmetic to be employed.
   k : if k is NIL, only root isolation is performed.  Otherwise, k
       is the target precision of refinement - i.e. we try to refine
       isolating intervals to a width of 1/2^k.

Outputs
   J : a logarithmic open standard isolating interval for \alpha that
       is a subinterval of I, the smallest that could be obtained
       with precision p arithmetic.
   L : If L = 0 then root isolation failed, otherwise L is a list  
       (I_1,...,I_m) of strongly disjoint intervals, each of which has
       logarithmic binary rational endpoints and is either open
       or one-point, and that are isolating intervals for the
       real roots of B(alpha,y).
======================================================================*/
#include "lift2d.h"

Word ProveNotRoot(Word A, Word M, Word I, Word a, Word b, Word Ap, Word K);

void modIBPRRIOAPSF(Word M, Word I, Word B, Word p, Word k, BDigit *J_, BDigit *L_)
{
  BDigit *Mp,*bp,*c = NULL,i,m,n,q1,q2,S,s,t,tc;
	Word a,b,Bp,I1,I2,J,K,L,Ls,Lp,T,Jp,Js,Ld;

 Step1: /* Convert the minimal polynomial to a software interval
          polynomial. */
        n = PDEG(M);
        q1 = p + 3;
        q2 = q1 + q1;
        S = (n + 1) * q2 + 1;
        Mp = GETARRAY(S);
        IPSIP(M,p,Mp);

Step2: /* Compute the trend of \alpha. */
	b = SECOND(I);
	bp = GETARRAY(q1);
	t = LBRNFEC(b,p,bp);
	J = I;
        L = 0;
	if (t == 0) {
	   FREEARRAY(bp);
	   goto Return; }
	t = SIPES(Mp,bp);
	FREEARRAY(bp);
	if (t == NIL)
	   goto Return;

Step3: /* Refine the isolating interval for \alpha. */
        J = SIPIR(Mp,I,t,- (p * ZETA));
        FREEARRAY(Mp);

Step4: /*  Isolate the real roots of each basis polynomial. */
	L = NIL;
	m = PDEG(B);
	s = (m + 1) * q2 + 1;
	c = GETARRAY(s);
	IBPELBRISIPR(B,J,p,c);
	L = modSIPRRID(c);
	if (L == 0)
	  goto Return;
	else  {
	  Word failCount = 0;
	  for(Word Lp = L; Lp != NIL; Lp = RED(Lp))
	    if (THIRD(FIRST(Lp)) != 0)
	      failCount++;
	  if (failCount > 1) {
	    t = 3;
	    goto Return;
	  }
	}
	
	tc = c[s - q2 + 1];
	if (EVEN(m))
	  tc = -tc;

Step5: /* Refine roots? */
	if (k == NIL)
	  goto Return;
	Js = NIL;
	Ls = NIL;
	for(Lp = L; Lp != NIL; Lp = RED(Lp))
        { 
	  Jp = FIRST(Lp);
	  FIRST2(Jp,&a,&b);

	  /* Take proper care of 1-point intervals! */
	  if (LBRNCOMP(a,b) == 0) {
	    // (a,a) is a simple root of B(alpha,y), and it is a simple root of linear poly
	    Word Jpnew = LIST5(a,b,0,LIST4(1,LIST2(0,IMP2(1,SECOND(a))),0,LIST2(0,INEG(FIRST(a)))),1);
	    Ls = COMP(Jpnew,Ls);
	    tc *= -1;
	    continue;
	  }

	  /***** (a,b) is an isolating interval for a single, simple root ***********/
	  if (THIRD(Jp) == 0) {
	    Js = SIPIR(c,Jp,tc,-k);
	    Jp = LIST5(FIRST(Js),SECOND(Js),0,B,tc);
	    tc *= -1;
	  }

	  /***** (a,b) is the only "don't know" interval. ***************************/
	  /* isolate and refine all the roots of the the derivative of B w.r.t. y over alpha, and if only 
	     one root is in (a,b), it must be the double root of B(alpha,y), and no other root of of
	     B(alpha,y) can be in the interval (y the mean value theorem!).
	  */
	  else {
	    Word Bp = IPDER(2,B,2);
	    Word Bpsqf = IPPGSD(2,Bp);
	    IBPRRIOAPSF(M,I,Bpsqf,p,k,&I,&Ld);
	    if (Ld != 0) {
	      Word Leftover = NIL;
	      for(Word Ldp = Ld; Ldp != NIL; Ldp = RED(Ldp)) {
		int tst = ProveNotRoot(B,M,I,a,b,Bp,FIRST(Ldp));
		if (!tst)
		  Leftover = COMP(FIRST(Ldp),Leftover);
	      }
	      if (LENGTH(Leftover) == 1) {
		Word ap, bp, tp;
		FIRST3(FIRST(Leftover),&ap,&bp,&tp);		  
		Jp = LIST5(ap,bp,1,Bpsqf,tp);
	      }
	      else {
		// We failed to determine which derivative root is the root of A we are looking for!
		t = 4;
		L = NIL;
		goto Return;
	      }
	    }
	  }
	  
	  Ls = COMP(Jp,Ls);
	}
	L = CINV(Ls);

Return: /* Return J and L. */
	if (c != NULL)
	  FREEARRAY(c);
	*J_ = J;
	*L_ = L;
	return;
}

