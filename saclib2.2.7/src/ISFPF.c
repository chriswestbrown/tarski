/*===========================================================================
			L <- ISFPF(r,A)

Integral squarefree polynomial factorization.

Inputs
  r : a BETA-digit, r >= 1.
  A : in Z[X1,...,Xr], A positive, of positive degree, primitive, and
      squarefree with respect to its main variable Xr.

Outputs
  L : a list (A1,...,Ak), with Ai in Z[X1,...,Xr], the distinct positive
      irreducible factors of A.
===========================================================================*/
#include "saclib.h"

/*
  SACLIB Object Size
  if L is an atom, return 1
  if L is a list (l1,...,lk) return 1 + SOSIZE(l1) + ... + SISIZE(lk)
 */
static Word SOSIZE(Word L) {
  if (ISATOM(L) || ISNIL(L)) return 1;
  Word  count = 1;
  for(Word Lp = L; Lp != NIL; Lp = RED(Lp)) {
    count += SOSIZE(FIRST(Lp));
  }
  return count;
}

/* Integral Squarefree Primitive Polynomial Irreducibility Quick Test 
Inputs
   r : the number of variables of A
   A : a squarefree, primitive integral polynomial
Outputs
   t : 1 if A is determined to be irreducible, 0 otherwise
*/
static Word ISPPIQT(Word r, Word A) {
  /* 
     The idea is to evaluate with random prime p for coefficients and
     random values for variables to get down to polynomial As that is 
     univariate in the main variable xr.  Since A is primitive, if As
     is irredicible over Z_p then A is irreducible over the integers. 
     Of course if As isn't irreducible we don't know anything, which
     is why this is a "quick test".
  */

  Word sz = SOSIZE(A);
#ifdef _ISFPF_DEBUG_
  SWRITE("In ISPPIQT ("); IWRITE(sz); SWRITE("):\n");
  Word V = NIL;
  char vn[2] = {'a','\0'};
  for(int i = 0; i < r; i++) {
    V = COMP(LFS(vn),V);
    vn[0]++;
  }
  IPDWRITE(r,A,V);
  SWRITE("\n");
#endif

  Word PL = MPRIME;
  Word EL = SPRIME;
  Word t = 0;

  Word N = IMIN(50,10 + 3*ILOG2(sz));
  for(int k = 0; k < N; k++) {
    /* Choose prime p and map A to Z_p. */
    Word p = FIRST(PL);
    PL = RED(PL);
    Word Ap = MPHOM(r,p,A);

    /* Loop: evaluating variables x1,...,x_{r-1} Note: we evaluate at
       small prime values ... for lack of a better idea. */
    Word As = Ap;
    Word rs = r;
    while(rs > 1) {
      Word x = FIRST(EL);
      EL = RED(EL);
      if (EL == NIL) { EL = SPRIME; }
      As = MPEVAL(rs,p,As,1,x);
      rs--;
    }
#ifdef _ISFPF_DEBUG_
    SWRITE("Evaluated to("); IWRITE(k); SWRITE("): "); IPDWRITE(1,As,V);
    SWRITE(" mod "); IWRITE(p); SWRITE("\n"); 
#endif    
    t = MMUPIT(p,As);
#ifdef _ISFPF_DEBUG_
    SWRITE("Result is "); IWRITE(t); SWRITE("\n");			 
#endif    
    if (t == 1) { goto Return; }
  }
  
 Return:  
  return t;
}

Word ISFPF(r,A)
       Word r,A;
{
       Word Abp,Ap,Ap1,As,As1,B,Bp,B1,C,D,F,L,Lp,Ls,M,N,P,T,Tp,V,
            ap,b,d,f,k,m,n,p,t,v;

Step1: /* r = 1. */
       if (r == 1) {
         L = IUSFPF(A);
         goto Return; }

Step2: /* Degree 1. */
       n = PDEG(A);
       if (n == 1) {
         L = LIST1(A);
         goto Return; }

Step3: /* Degree 2. */
       if (n == 2) {
         L = ISPFAC2(r,A);
         goto Return; }

Step4: /* Look for variable of degree 1 or 2. */
       IPVDEG12(r,A,&k,&B);
       if (k > 0) {
         IPFAC(r,B,&t,&d,&N);
         L = NIL;
         while (N != NIL) {
           ADV(N,&F,&N);
           FIRST2(F,&f,&B1);
           B1 = PCPV(r,B1,k,r);
           B1 = IPABS(r,B1);
           L = COMP(B1,L); }
         goto Return; }

StepX: /* Try quick test for irreducibility. */
       if (r > 2 && ISPPIQT(r,A) == 1) {
	 L = LIST1(A);
         goto Return;
       }
       
Step5: /* Evaluate to univariate polynomial, As, and factor As. */
       IPCEVP(r,A,&As,&T);
       As = IPABS(1,As);
       As = IPPP(1,As);
       Ls = IUSFPF(As);

Step6: /* As irreducible. */
       if (RED(Ls) == NIL) {
         L = LIST1(A);
         goto Return; }

Step7: /* Translate A to Ap and find prime p not dividing discr(As). */
       T = COMP(0,INV(T));
       Ap = IPTRAN(r,A,T);
       P = LPRIME;
       n = PDEG(As);
       do {
         if (P == NIL)
           FAIL("ISFPF","Prime list exhausted",r,A);
         ADV(P,&p,&P);
         B = MPHOM(1,p,As);
         m = PDEG(B);
         if (m == n) {
           Bp = MUPDER(p,B);
           C = MUPGCD(p,B,Bp);
           k = PDEG(C); } }
       while (m != n || k != 0);

Step8: /* Convert As factors to monic factors modulo p. */
       Lp = NIL;
       do {
         ADV(Ls,&As1,&Ls);
         Ap1 = MPHOM(1,p,As1);
         Ap1 = MPMON(1,p,Ap1);
         Lp = COMP(Ap1,Lp); }
       while (Ls != NIL);

Step9: /* Compute a factor coefficient bound for Abp = Ap*ldcf(Ap). */
       ap = PLDCF(Ap);
       Abp = IPPROD(r,Ap,LIST2(0,ap));
       b = IPGFCB(r,Abp);
       b = IMP2(b,1);
       M = p;
       while (ICOMP(M,b) <= 0)
         M = IDPR(M,p);

Step10: /* Compute factor degree bounds. */
       V = PDEGV(r,Abp);
       D = NIL;
       V = RED(V);
       do {
         ADV(V,&v,&V);
         d = v + 1;
         D = COMP(d,D); }
       while (V != NIL);

Step11: /* Lift modular factors. */
       L = MPIQHL(r,p,Lp,M,D,Ap);

Step12: /* Combine lifted factors. */
       Lp = IPFLC(r,M,D,Ap,L,0);

Step13: /* Translate Ap factors to A factors. */
       Tp = NIL;
       do {
         ADV(T,&t,&T);
         Tp = COMP(INEG(t),Tp); }
       while (T != NIL);
       Tp = INV(Tp);
       L = NIL;
       do {
         ADV(Lp,&Ap1,&Lp);
         L = COMP(IPTRAN(r,Ap1,Tp),L); }
       while (Lp != NIL);

Return: /* Prepare for return. */
       return(L);
}
