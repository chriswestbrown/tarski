/*======================================================================
                      modIBPRRIOAP(M,I,B,k; L,t)

Integral bivariate polynomial real root isolation over an algebraic point

Inputs
   M : the minimal integral polynomial of an algebraic number
       \alpha.
   I : an open standard logarithmic isolating interval for \alpha.
   B : a bivariate polynomial such that B(alpha,y) is squarefree
   k : if k is NIL, only root isolation is performed.  Otherwise, k
       is the target precision of refinement - i.e. we try to refine
       isolating intervals to a width of 1/2^k.
Outputs
   L : If t = 0, L is a list  (I_1,...,I_m)
       of the form I_i = (a,b,e), where (a,b) is strongly disjoint interval with
       logarithmic binary rational endpoints and is either open
       or one-point, and e is either 0 or 1.  If e is zero, then (a,b)
       is an isolating interval for exactly one root of B(alpha,y).
       If e is 1, then the interval may contain roots, nothing is
       known.
   t : 0, 1 or 2.  If t = 1, the program failed to produce an
       isolation list L because of exponent limitation.  If t = 2, 
       the failure was due to mantissa limitation.
======================================================================*/
#include "lift2d.h"

Word ProveNotRoot(Word A, Word M, Word I, Word a, Word b, Word Ap, Word K);


void modIBPRRIOAP(Word M, Word I, Word B, Word k, Word *L_, BDigit *t_)
{
        Word L,CFP,Bp,Ls,Lp,Ld,td,a,b,Jp;
	BDigit t,n,n1,n2,np,u,s,e,i,j,t1,tc,c;
	ieee F1,F2;
	double p,w1,w2;
	interval *A = NULL,K,*Q = NULL,*HICFP,J;

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

Step3: /* Isolate the roots of B(alpha,y) */
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
	
	/* get trend of first root */
	if ((PDEG(B) % 2 == 0 && s == 1) || (PDEG(B) % 2 == 1 && s == -1))
	  t1 = -1;
	else
	  t1 = 1;

	/* Isolate the roots of B(alpha,y) */
	modHIPRRID(PDEG(B),Q, &L,&t);
	if (t != 0)
	  goto Return;

 Step4: /* Refine roots? */
	if (k == NIL)
	  goto Return;
	Ls = NIL;
	for(Lp = L, tc = t1; Lp != NIL; Lp = RED(Lp), tc *= -1)
        { 
	  FIRST2(FIRST(Lp),&a,&b);

	  /* Take proper care of 1-point intervals! */
	  if (LBRNCOMP(a,b) == 0) {
	    Ls = COMP(FIRST(Lp),Ls);
	    continue; }
	    
	  Jp = LIST2(a,b);
	  LBRIHI(FIRST(Lp),&J,&t);
	  j = -LSILW(Jp);

	  if (j < k) {
	    if (THIRD(FIRST(Lp)) == 0) {
	      /***** EXACTLY ONE ROOT ***********/
	      HIPIR(PDEG(B),Q,J,tc,j,k,&J,&j); 
	      Jp = HILBRI(J); }
	    else {
	      //-------------------------------------------------//	      
	      /***** DON'T KNOW HOW MANY ROOTS **/
	      /* This is horribly inefficient, but I'm going to
		 isolate and refine all the roots of the the 
		 derivative of B w.r.t. y over alpha, and if only 
		 one root is in (a,b), it must be my double root.
		 TODO: check for IPDER not being square-free as a poly in y!
		*/
	      /* PushOutputContext(cerr); */
	      /* SWRITE("Don't know interval ("); LBRNWRITE(a); SWRITE(","); LBRNWRITE(b); SWRITE(") must be refined!\n"); */
	      /* PopOutputContext(); */
	      Word Bp = IPDER(2,B,2);
	      IBPRRIOAP(M,I,IPPGSD(2,Bp),k,&Ld,&td);
	      if (td != 0) {
	      	IBPRRIOAPSF(M,I,IPPGSD(2,Bp),8,k,&td,&Ld);
	      	td = !Ld;
	      }
	      if (td == 0) {
		// Filter out all isolating intervals not overlapping (a,b)
		/* PushOutputContext(cerr); SWRITE("Before: |Ld| = "); IWRITE(LENGTH(Ld)); SWRITE("\n"); PopOutputContext(); */
	      	while(LBRNCOMP(SECOND(FIRST(Ld)),a) <= 0)
	      	  Ld = RED(Ld);
		Word Ltmp = NIL;
		while(Ld != NIL && LBRNCOMP(b,FIRST(FIRST(Ld))) >= 0) {
		  Ltmp = COMP(FIRST(Ld),Ltmp);
		  Ld = RED(Ld);
		}
		Ld = CINV(Ltmp);

		/* PushOutputContext(cerr); SWRITE("After: |Ld| = "); IWRITE(LENGTH(Ld)); SWRITE("\n"); PopOutputContext(); */
		
		// If there is only one interval, it contains the root beta of B(alpha,y),
		// and is an isolating inteverval for beta as a root of B'(alpha,y)
		if (LENGTH(Ld) == 1) {		  
	      	  //Jp = FIRST(Ld);
	      	  Jp = CCONC(FIRST(Ld),LIST1(Bp));
		}
		else {
		  Word Leftover = NIL;
		  for(Word Ldp = Ld; Ldp != NIL; Ldp = RED(Ldp)) {
		    int tst = ProveNotRoot(B,M,I,a,b,Bp,FIRST(Ldp));
		    if (!tst)
		      Leftover = COMP(FIRST(Ldp),Leftover);
		  }
		  if (LENGTH(Leftover) == 1) {
		    Jp = CCONC(FIRST(Leftover),LIST1(Bp));
		    /* PushOutputContext(cerr); SWRITE("Successfully found the root of deriviative!"); PopOutputContext(); */
		  }
		  else {
		    // We failed to determine which derivative root is the root of A we are looking for!
		    /* PushOutputContext(cerr); SWRITE("Failed to find the root of deriviative!"); PopOutputContext(); */
		  }
		}
	      }
	      //-------------------------------------------------//
	    }
	  }
	  /* PushOutputContext(cerr); SWRITE("Jp = "); OWRITE(Jp); SWRITE("\n"); PopOutputContext(); */
	  Word Jpnew = NIL;
	  if (RED2(Jp) == NIL) // i.e. length is 2
	    Jpnew = CCONC(Jp,LIST1(THIRD(FIRST(Lp))));
	  else
	    Jpnew = LIST4(FIRST(Jp),SECOND(Jp),THIRD(FIRST(Lp)),THIRD(Jp));
	  Ls = COMP(Jpnew,Ls);
	  if (THIRD(FIRST(Lp)) != 0)
	    tc *= -1;
	}
	L = CINV(Ls);
	t = 0;
	  
Return: /* Return L and t. */
	if (Q != NULL)
	  FREEARRAY(Q);
	if (A != NULL)
	  FREEARRAY(A);
	*L_ = L;
	*t_ = t;
	return;
}

/*
IUPSHIFTISPR - Integral univariate polynomial shift, integral similar polynomial result 
Inputs:
  A : integral univariate polynomial
  q : a rational number
Outputs:
  B : integral polynomial similar to A(x + q)
Word IUPSHIFTISPR(Word A, Word q)
{
  if (q == 0)
    return A;
  
  Word a_n = FIRST(q);
  Word a_d = SECOND(q);

  // construct a_d*y - (a_d*x + a_n)
  Word tmp = LIST4(1,LIST2(0,a_d), 0,IPNEG(1,LIST4(1,a_d,0,a_n)));
  // construct p_o(x,z) over x,y,z from p_o(x,y) over x,y
  Word p_n = NIL;
  for(Word A = p_o; A != NIL; A = RED2(A)) {
    p_n = COMP(FIRST(A),p_n);
    p_n = COMP(LIST2(0,SECOND(A)),p_n);
  }
  p_n = INV(p_n);
  Word B = IPRES(2,p_n,tmp);

  return B;
}
 */

// just cherry pick the positive root parts of IPRRID
Word IPRRIDPOS(Word A)
{
       Word n,k;
       Word Ab,As,I,L,Ls,a,b;

Step1: /* Degree zero. */
       n = PDEG(A);
       L = NIL;
       if (n == 0)
	  goto Return;

Step2: /* Isolate positive roots. */
       Ab = PDBORD(A);
       b = IUPPRB(Ab);
       if (b == 0)
	  goto Return;
       k = -SECOND(b);
       As = IUPBHT(Ab,k);
       L = IPRRISD(As,0,b);

 Return: /* Prepare for return. */
       return(L);
}
 
// just cherry pick the negative root parts of IPRRID
Word IPRRIDNEG(Word A)
{
       Word n,k;
       Word Ab,As,I,L,Ls,a,b;

Step1: /* Degree zero. */
       n = PDEG(A);
       L = NIL;
       if (n == 0)
	  goto Return;

Step4: /* Isolate negative roots. */
       Ab = IUPNT(Ab);
       b = IUPPRB(Ab);
       if (b == 0)
	  goto Return;
       k = -SECOND(b);
       As = IUPBHT(Ab,k);
       Ls = IPRRISD(As,0,b);
       while (Ls != NIL) {
	  ADV(Ls,&I,&Ls);
	  FIRST2(I,&a,&b);
	  I = LIST2(LBRNNEG(b),LBRNNEG(a));
	  L = COMP(I,L); }

Return: /* Prepare for return. */
       return(L);
 }       


Word IPIISFLBRN(Word A1, Word I1, Word q, Word* J1_);

/*
Inputs:
  A : poly in x, y
  M : minpoly for root alpha of disc_y(A)
  I : isolating interval for alpha
      NOTE: alpha is a simple foor if disc_y(A)!!!
  a,b : LBRNs, an isolating interval for beta, a multiplicity two root of A(alpha,y) (necessarily the only such)
  Ap: derivative of A wrt y
  K : an isolating interval for a root of Ap(alpha,y)
Outputs:
  R : TRUE if we can *prove* that beta is not in K, FALSE otherwise
      NOTE: our attempt to prove is just to ensure no roots of A on the permieter of the rectangle
            formed by I and (J intersect K).
 */
Word ProveNotRoot(Word A, Word M, Word I, Word a, Word b, Word Ap, Word K)
{
  // restrict interval
  Word ap = FIRST(K);
  Word low = LBRNCOMP(a,ap) < 0 ? ap : a;
  Word bp = SECOND(K);
  Word hi = LBRNCOMP(b,bp) > 0 ? bp : b;

  // I should check whether (low,hi) still contains a root of Ap!
  
  // each rectangle edge creates a univariate polynomial!
  Word Abot = IPBREI(2,A,2,LBRNRN(low));
  Word Atop = IPBREI(2,A,2,LBRNRN(hi));
  Word Aleft = IPBREI(2,A,1,LBRNRN(FIRST(I)));
  Word Aright = IPBREI(2,A,1,LBRNRN(SECOND(I)));

  // check that Abot and Atop have no roots in I TODO: make these squarefree!
  Word Lbot = IPRRISD(Abot,FIRST(I),SECOND(I));
  if (Lbot != NIL) return FALSE;
  Word Ltop = IPRRISD(Atop,FIRST(I),SECOND(I));
  if (Ltop != NIL) return FALSE;

  // check that Aleft and Aright have no roots in (low,high) TODO: make these squarefree!
  //Word r_low = LBRNRN(low);
  //Word C = IUPSHIFTISPR(Aleft,r_low);
  //Word newtop = LBRNDIF(hi-low);
  Word L = IPRRID(Aleft);
  for(Word Lp = L; Lp != NIL; Lp = RED(Lp)) {
    Word R = FIRST(Lp);
    if (IPIISFLBRN(Aleft,R,low,&R) > 0 && IPIISFLBRN(Aleft,R,hi,&R) < 0)
      return FALSE;
  }

  Word Ls = IPRRID(Aright);
  for(Word Lp = Ls; Lp != NIL; Lp = RED(Lp)) {
    Word R = FIRST(Lp);
    if (IPIISFLBRN(Aright,R,low,&R) > 0 && IPIISFLBRN(Aright,R,hi,&R) < 0)
      return FALSE;
  }

  return TRUE;
}


/*======================================================================
               IPIISFLBRN(A1,I1,q;J1,s)

Integral polynomial isolating interval separation from LBRN.

Inputs
   A1 : integral univariate polynomial
   I1 : logarithmic binary rational isolating intervals for 
           simple real root alpha_1 of A1
   q  : logarimic binary rational number
Outputs
   J1 : logarithmic binary rational subinterval of I1.
   s     : s = -1 if alpha_1 < q, s = +1 if alpha_1 > q, s = 0 otherwise
======================================================================*/
#include "saclib.h"

Word IPIISFLBRN(Word A1, Word I1, Word q, Word* J1_)

{
  Word a1,b1,c,J1,s, ta, tq;


Step1: /* Get the interval endpoints. */
	FIRST2(I1,&a1,&b1);
	if (LBRNCOMP(a1,b1) == 0) {
	  J1 = I1;
	  s = LBRNCOMP(a1,q);
	  goto Return;
	}

Step2: /* Case that the intervals are already disjoint. */
	if (LBRNCOMP(b1,q) <= 0) {
           J1 = I1;
           s = -1;
           goto Return; }
        else if (LBRNCOMP(q,a1) <= 0) {
           J1 = I1;
           s = 1;
           goto Return; }
        else
           s = 0;

	ta = IUPBRES(A1,a1);
	tq = IUPBRES(A1,q);
	if (tq == 0) {
	  J1 = LIST2(q,q);
	  s = 0;
	}
	else if (ta == tq) {
	  J1 = LIST2(q,b1);
	  s = 1;
	}
	else {
	  J1 = LIST2(a1,q);
	  s = -1;
	}

 Return: /* Return J1, J2 and s. */
        *J1_ = J1;
        return s;
}

 
