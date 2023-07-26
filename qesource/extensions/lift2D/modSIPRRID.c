/*======================================================================
                         L <- modSIPRRID(A)

Software interval polynomial real root isolation, Descartes method.

Input
   A : a software interval polynomial of degree n > 0.  The
       coefficient of x^n does not contain 0.

Output
   L : Either 0 or a list (I_1,...,I_r) of standard logarithmic open
       or one-point binary rational isolating intervals for all of
       the real roots of A.  Let (a_j,b_j) denote the endpoints
       of I_j.  Then a_1 <= b_1 <= ... <= a_r <= b_r.  L = 0 in
       case it is not possible to isolate all real roots using
       p-precision interval arithmetic.
======================================================================*/
#include "lift2d.h"

Word modSIPRRID(BDigit *A)
{
       BDigit *Ap,k,s,S;
       Word a,b,B,I,L,L1;

Step1: /* Isolate the positive roots. */
	S = SIPSIZE(A);
	Ap = GETARRAY(S);
	SIPCOPY(A,Ap);
        k = SIPPRB(A);
        if (k == NIL) {
	   L = NIL;
           goto Step2; }
        SIPBHT(Ap,k);
	B = LIST2(1,-k);
        L = modSIPRRISD(Ap,0,B);

Step2: /* Is 0 a root? */
	s = SISIGN(A + 1);
	if (s == NIL) {
	  L = 0; /* This is a problem! */
	  goto Step4; }
	if (s == 0) {
	  Word n = A[0];
	  if (n == 0) { I = LIST3(0,0,0); }
	  else {
	    Word p = A[3];
	    Word q1 = p + 3;
	    Word q2 = q1 + q1;
	    Word s1 = SISIGN(A + 1 + q2); // A + 1 + q2 is the start of the degree 1 coefficient
	    if (s1 == NIL) { I = LIST3(0,0,1); }
	    if (s1 == 0) { I = LIST3(0,0,1); }
	    else { I = LIST3(0,0,0); }
	  }
	  L = COMP(I,L); }
	
Step3: /* Isolate the negative roots. */
	SIPNT(A,Ap);
	k = SIPPRB(Ap);
	if (k == NIL)
	   goto Step4;
	SIPBHT(Ap,k);
	B = LIST2(1,-k);
        L1 = modSIPRRISD(Ap,0,B);
	while (L1 != NIL) {
	   ADV(L1,&I,&L1);
	   FIRST2(I,&a,&b);
	   I = LIST3(LBRNNEG(b),LBRNNEG(a),THIRD(I));
	   L = COMP(I,L); }

Step4: /* Free array. */
	FREEARRAY(Ap);

Return: /* Return. */
        return(L);
}
