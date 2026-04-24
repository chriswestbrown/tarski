/*======================================================================
                 I <- IMPCONS(c,L_F,L_A,P)

Implicant construction.

Input
 c   :  a cell (PCAD or ESPCAD)
 L_F :  a list of cells (PCAD or ESPCAD)
 L_A :  a list of FMA formulas
 P   :  the projection factor set upon which the signitures of the
        elements of L_C are based.
Output
 I   :  a minimum length conjunction of the elements of L_A that
        are satisfied c, such that I is satisfied by none of L_F.
======================================================================*/
#include "qepcad.h"

Word IMPCONS(Word c, Word L_F, Word L_A, Word P)
{
     Word I,A,n,S,F,f,S_f,Ap,i,T,L;

Step1: /* Initialize. Set A to the sublist of L_A stasified by c. */
     if (L_F == NIL) {
       I = LIST1(TRUE);
       goto Return; }
     for(A = NIL, L = L_A; L != NIL; L = RED(L))
       if (FMACELLEVAL(FIRST(L),c,P) == TRUE)
	 A = COMP(FIRST(L),A);
     n = LENGTH(A);
     S = NIL;

Step2: /* Construct the minimum hitting set problem. */
     for(F = L_F; F != NIL; F = RED(F)) {
       f = FIRST(F);
       S_f = NIL;
       for(i = n, Ap = A; Ap != NIL; i--,Ap = RED(Ap)) {
	 if (EQUAL(PCADCINDEX(f),LIST2(21,13)) && EQUAL(PCADCINDEX(c),LIST2(10,5))
	     && EQUAL(FIRST(Ap),LIST2(LIST2(2,6),4))
	     ) {
	   SWRITE("Problem spot?\n");
	 }
	 if (FMACELLEVAL(FIRST(Ap),f,P) == FALSE) {
	   S_f = COMP(i,S_f);
	 }
       }
       // if S_f is NIL we have a problem because all FMAs in L_A are
       // true in cell f, so we have no formula to rule them out!
       if (S_f == NIL) {
	 SWRITE("Problem! non-rule-out-able cell ");
	 OWRITE(PCADCINDEX(f));
	 SWRITE(" original cell ");
	 OWRITE(LELTI(LELTI(f,1),6));
	 SWRITE(", where c is ");
	 OWRITE(PCADCINDEX(c));
	 SWRITE(" original cell ");
	 OWRITE(LELTI(LELTI(c,1),6));
	 SWRITE("\n");
	 SWRITE("A = "); OWRITE(A); SWRITE("\n");
       }
       S = COMP(S_f,S); }
	
Step3: /* Get the hitting set. */
     T = MINHITSETSR(S,-1);
     if (!ISLIST(T)) {
       I = 0;
       goto Return; }

Step4: /* Convert hitting set to an implicant. */
     T = LBIBMS(T);
     for(I = NIL, L = INV(A), i = 1; T != NIL; i++, L = RED(L))
       if (i == FIRST(T)) {
	 T = RED(T);
	 I = COMP(FIRST(L),I); }
     if (LENGTH(I) == 1)
       I = FIRST(I);
     else
       I = COMP(ANDOP,INV(I));
  
Return: /* Prepare to return. */
     return I;
}
