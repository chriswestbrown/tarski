/*======================================================================

Boundary 2D.

Takes current 2D set and sets truth values to define the boundary of
the set. NOTE: p is on the boundary of S iff in every neighborhood of p
there has non-empty intersection with both S and its complement.
I think this is assuming that we have a full CAD of 2-space.

10/18/01 - just copied CLOSURE2D
======================================================================*/
#include "truthbytop.h"
#include "adj2D.h"

/*
  D is a QEPCAD 2D data-structure.
  Checks that D is complete as a 2D CAD, i.e. that there are no
  1-level or 0-level cells w/o children ... even if it's a vacuous
  single cell.  Also that each cell in 2D has TRUE/FALSE as a truth
  value (not e.g. UNDET).  Also that each 1-level cell has UNDET as
  its truth value.
  Return 0 if check & clean is OK, non-zero otherwise
*/
Word checkAndClean2D(Word D) {
  /* Check that the 0-level cell has children. */
  if (LELTI(D,CHILD) == NIL)
    return FALSE;
  
  /* Check no 1-level cells w/o children, 2-level cells w/o TRUE/FALSE */
  Word errors = 0;
  const int no_child = 1; // i.e. 1-level cell w/o child
  const int no_tv = 2; // i.e. 2-level cell w/o TRUE/FALSE truth value
  for(Word L = LELTI(D,CHILD); L != NIL && errors == 0; L = RED(L)) {
    Word c = FIRST(L);
    if (LELTI(c,CHILD) == NIL)
      errors |= no_child;
    else {
      for(Word S = LELTI(c,CHILD); S != NIL && errors == 0; S = RED(S)) {
	Word tv = LELTI(FIRST(S),TRUTH);
	if (tv != TRUE && tv != FALSE)
	  errors |= no_tv;
      }
    }
  }

  /* Clean up by turning any 1-level cells with TRUE/FALSE tv's to UNDET */
  for(Word L = LELTI(D,CHILD); L != NIL && errors == 0; L = RED(L)) {
    Word c = FIRST(L);
    Word tv = LELTI(c,TRUTH);
    if (tv == TRUE || tv == FALSE)
      SLELTI(c,TRUTH,UNDET);
  }

  return errors;
}

void QepcadCls::BOUNDARY2D(Word D, Word P, Word J)
{
  Word G,L,S,s,c,Sp,i,j,cl,cm,cr,E,Lp,L0,L1,L2,v,t,tc,fc,LH,LI0;

Step0: /* Check and clean D! */
  if (checkAndClean2D(D) != 0) {
    SWRITE("Error! BOUNDARY2D requires a 'full' CAD of R^2 where all 2-level cells have truth values.");
    goto Return;
  }    
  
Step1: /* Initialization. */
  G = NIL;

Step2: /* Graph vertices. L is a list of all vert's in descending lex order. */
  L = NIL;
  for(S = LELTI(D,CHILD); S != NIL; S = RED(S))
    for(s = LELTI(FIRST(S),CHILD); s != NIL; s = RED(s))
      L = COMP(FIRST(s),L);
  for(Lp = L; Lp != NIL; Lp = RED(Lp)) {
    c = FIRST(Lp);
    GADDVERTEX(LELTI(c,INDX),LELTI(c,TRUTH),&G); }

Step3: /* Add edges. */
  S = LELTI(D,CHILD);
  if (LENGTH(S) < 3)
  {
    if (S != NIL)
    {
      Word c = FIRST(S);
      Word S_c = LELTI(c,CHILD);
      while(S_c != NIL && RED(S_c) != NIL)
      {
	Word c1 = FIRST(S_c), c2 = SECOND(S_c), c3 = THIRD(S_c);
	if ((LELTI(c1,TRUTH) == TRUE && LELTI(c3,TRUTH) == FALSE)
	    ||
	    (LELTI(c1,TRUTH) == FALSE && LELTI(c3,TRUTH) == TRUE))
	{
	  SLELTI(c2,TRUTH,TRUE);
	  SLELTI(c2,HOWTV,TOPINF);
	}
	else if (LELTI(c1,TRUTH) == TRUE && LELTI(c3,TRUTH) == TRUE)
	{
	  SLELTI(c2,TRUTH,(LELTI(c2,TRUTH) == TRUE ? FALSE : TRUE));
	  SLELTI(c2,HOWTV,TOPINF);
	}
	SLELTI(c1,TRUTH,FALSE); SLELTI(c1,HOWTV,TOPINF);
	S_c = RED2(S_c);
      }	
      if (S_c != NIL) { SLELTI(FIRST(S_c),TRUTH,FALSE); SLELTI(FIRST(S_c),HOWTV,TOPINF); }
    }
    goto StepX;
  }
  
Step4: /* Edges between cells in the same stack. */
  for(Sp = S; Sp != NIL; Sp = RED(Sp)) {
    for(s = LELTI(FIRST(Sp),CHILD); s != NIL; s = RED(s)) {
      FIRST2(LELTI(FIRST(s),INDX),&i,&j);
      if (j % 2 == 0) {
	GADDEDGE(LIST2(LIST2(i,j+1),LIST2(i,j)),G);
	GADDEDGE(LIST2(LIST2(i,j-1),LIST2(i,j)),G); } } }
      
Step5: /* Edges between cells in different stacks. */
 do {
    ADV2(S,&cl,&cm,&S); cr = FIRST(S);
    for(E = C1DTOEDGELIST(cl,cm,cr,P,J); E != NIL; E = RED(E))
      GADDEDGE(FIRST(E),G);
  }while(RED(S) != NIL);

Step6: /* Split cell list by dimension. */
 for(L0 = NIL, L1 = NIL, L2 = NIL, Lp = L; Lp != NIL; Lp = RED(Lp)) {
   c = LELTI(FIRST(Lp),INDX);
   switch(vert2dim(c)) {
   case 0: L0 = COMP(c,L0); break;
   case 1: L1 = COMP(c,L1); break;
   case 2: L2 = COMP(c,L2); break; } }

 /* Find isolated true L0 cells and isolated false L0 cells*/
 LI0 = NIL;
 for(Lp = L0; Lp != NIL; Lp = RED(Lp)) {
   v = FIRST(Lp);
   Word tv_v = GVERTEXLABEL(v,G);
   if (tv_v != TRUE && tv_v != FALSE) continue;
   Word neg_tv_v = tv_v == TRUE ? FALSE : (tv_v == FALSE ? TRUE : UNDET);
   LH = NIL;
   for(S = GPREDLIST(v,G); S != NIL && GVERTEXLABEL(FIRST(S),G) == neg_tv_v; S = RED(S))
     LH = CCONC(GPREDLIST(FIRST(S),G),LH);
   if (S == NIL) {
     for(;LH != NIL && GVERTEXLABEL(FIRST(LH),G) == neg_tv_v; LH = RED(LH));
     if (LH == NIL)
       LI0 = COMP(v,LI0);
   } }

 /* Set L1 cells to TRUE IFF they have both true & false predecessors, 
    or their truth value is oppositive all of their predecessors */
 for(Lp = L1; Lp != NIL; Lp = RED(Lp)) {
   v = FIRST(Lp);
   for(S = GPREDLIST(v,G), tc = 0, fc = 0; S != NIL; S = RED(S)) {
     if (GVERTEXLABEL(FIRST(S),G) == TRUE)
       tc++;
     else
       fc++; }
   Word lab_v = GVERTEXLABEL(v,G);
   if ((tc > 0 && fc > 0) || (lab_v == TRUE && tc == 0) || (lab_v == FALSE && fc == 0))
     GNEWLABEL(v,TRUE,G);
   else
     GNEWLABEL(v,FALSE,G); }
   
 /* Set L2 cells to FALSE */
 for(Lp = L2; Lp != NIL; Lp = RED(Lp))
   GNEWLABEL(FIRST(Lp),FALSE,G);


 /* Set L0 cells to TRUE IFF they have a true predecessor */
 for(Lp = L0; Lp != NIL; Lp = RED(Lp)) {
   v = FIRST(Lp);
   for(S = GPREDLIST(v,G), tc = 0, fc = 0; S != NIL; S = RED(S)) {
     if (GVERTEXLABEL(FIRST(S),G) == TRUE)
       tc++;
     else
       fc++; }
   if (tc > 0)
     GNEWLABEL(v,TRUE,G);
   else
     GNEWLABEL(v,FALSE,G); }
 
 /* Set all the isolated L0 cells to true */
 for(Lp = LI0; Lp != NIL; Lp = RED(Lp))
   GNEWLABEL(FIRST(Lp),TRUE,G);

Step10: /* Assign new TV's to CAD. */
 for(Lp = L; Lp != NIL; Lp = RED(Lp)) {
   c = FIRST(Lp);
   t = GVERTEXLABEL(LELTI(c,INDX),G);
   if (t != UNDET) {
     SLELTI(c,TRUTH,t);
     SLELTI(c,HOWTV,TOPINF); } }

StepX: /* Assignments between 1D cells. 
  TVCLOSURE1D(D,P,J,3);
  CTVPROPUP(D,UNDET,GVNFV,TOPINF); */

Return: /* Prepare to return. */
  return;
}
 
