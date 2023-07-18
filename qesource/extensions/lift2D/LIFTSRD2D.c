
/*
  Lifts over cell c in CAD D with projection factor set P.
  c must satisfy CELLSRRQ2D, and both siblings of c must
  already have their child stacks in place.  Needs L, a list
  of 2-level projection factors whose discriminants have
  order 1 zeros in c.
  Return 1 if successfull, otherwise 0
*/

#include "lift2d.h"

#define _PRE_ 22

Word LIFTSRD2D(Word c, Word D, Word P, Word L)
{
  Word S_L,S_R,s_L,s_R,S,i,c_L,c_R,cp,flag,m_L,m_R,so,mo,m,s,j;
  Word M,I,P2,Rp,t,R,Rs,Rt,SP,r,k,c1,c2,prev,Sp,sor,next,s2,nextc,X;
  Word DL,Rps,pf,a,b,e,temp,count,J;
  Word R_L;

  /* Initialization */
  S_L = LELTI(CELLLEFTSIB(c,D),CHILD); 
  S_R = LELTI(CELLRIGHTSIB(c,D),CHILD);
  S = NIL;

  /* Get coordinate info for c: M the min pol, I the interval */
  FIRST2(LELTI(c,SAMPLE),&M,&I);


  /* Isolating the roots of all 2-level projection factors over c. 
     Note:  I know this is inefficient! */
  DL = NIL; /* this will be a list of pairs (j,k) such that projection
	       factor P_2,j's discrim has a order 1 root at c, and the
	       kth root or P_2,j is the multiple root. */
  R = NIL;
  for(P2 = LELTI(P,2); P2 != NIL; P2 = RED(P2)) {
    /**********************************************
     ** LOOP OVER EACH 2-LEVEL PROJECTION FACTOR pf
     **********************************************/
    pf = FIRST(P2);
    Word p_o = LELTI(pf,PO_POLY);
      
    /***** pf's discriminant vanishes in c *****/
    if (LSRCH(pf,L)) {
      // !!! shift by sufficiently unlikely number so we probably won't get binary rational root!
      // !!! p_o is the original poly, p_m is the shifted poly
      Word a_n = 1;
      Word a_d = 601; // note: 601 is prime    
      // construct a_d*z - (a_d*y + a_n)
      Word tmp = LIST4(1,LIST2(0,LIST2(0,a_d)), // z + ...
		       0,IPNEG(2,LIST4(1,LIST2(0,a_d),0,LIST2(0,a_n)))); // -(a_d y + a_n)
      // construct p_o(x,z) over x,y,z from p_o(x,y) over x,y
      Word p_n = NIL;
      for(Word A = p_o; A != NIL; A = RED2(A)) {
	p_n = COMP(FIRST(A),p_n);
	p_n = COMP(LIST2(0,SECOND(A)),p_n);
      }
      p_n = INV(p_n);
      Word p_m = IPRES(3,p_n,tmp);
      
      /* First attempt to isolate roots! (Using Hardware!)*/
      i = 8;
      modIBPRRIOAP(M,BRILBRI(I),p_m,_PRE_,&Rp,&t);
      count = 0;
      if (t == 0) {
	for(Rps = Rp; Rps != NIL; Rps = RED(Rps))
	  if (THIRD(FIRST(Rps)) > 0)
	    count++; }

      /* If the first attempt fails, try again with software floats! */
      if (t != 0 || count > 1) {
	Word Ip = BRILBRI(I);
	for(i = 8, Rp = 0; Rp == 0 && i < 50; i += 8) 
	  modIBPRRIOAPSF(M,Ip,p_m,i,_PRE_,&Ip, &Rp);
	if (PCVERBOSE) { SWRITE("Tried up to precision "); IWRITE(i - 8); SWRITE("\n"); }
	if (Rp == 0) {
	  if (PCVERBOSE) { SWRITE("Even the highprecision call to modIBPRRIOAPSF failed!\n"); }
	  X = 0;
	  goto Return; }
	t = 0;
      }

      for(Rps = NIL, i = 1, j = 0 ; Rp != NIL; Rp = RED(Rp), i++) {
	FIRST3(FIRST(Rp),&a,&b,&e);
	Rps = COMP(LIST2(a,b),Rps);
	if (e == 1 && j != 0)
	{
	  if (PCVERBOSE) { SWRITE("Problem in \"LIFTSRD2D\": More than one \"don't know\"!\n");	}
	  X = 0;
	  goto Return;
	}
	else if (e == 1)
	  j = i; }
      Rp = CINV(Rps);
      DL = COMP(LIST2(THIRD(LELTI(pf,PO_LABEL)),j),DL);

      //!!! Unshift each isolating interval (using current precision i), and rep. as BRI not LBRI
      Word* SI = GETARRAY(2*i+6);
      Word* SJ = GETARRAY(2*i+6);
      Word* SK = GETARRAY(2*i+6);
      for(Rs = Rp, Rt = NIL; Rs != NIL; Rs = RED(Rs)) {
	LBRISI(FIRST(Rs),i,SI);
	RSI(i,a_n,a_d,SJ);
	SISUM(SI,SJ,SK);
	Word delta = i + 3;
	Word I_next = LBRIBRI(LIST2(FLBRN(SK),FLBRN(SK+delta)));
	if (Rt != NIL && RNCOMP(SECOND(FIRST(Rt)),FIRST(I_next)) > 0) {
	  FREEARRAY(SI);
	  FREEARRAY(SJ);
	  FREEARRAY(SK);
	  X = 0;
	  goto Return;
	}
	Rt = COMP(I_next,Rt);
	// old: Rt = COMP(LBRIBRI(FIRST(Rs)),Rt);
      }
      FREEARRAY(SI);
      FREEARRAY(SJ);
      FREEARRAY(SK);
    }

    /***** pf's discriminant does NOT vanish in c. *****/
    else { 
      i = 8;
      IBPRRIOAP(M,BRILBRI(I),LELTI(FIRST(P2),PO_POLY),_PRE_,&Rp,&t); 
      if (t) {/* This line is my test stuff! */
	Word Ip = BRILBRI(I);
	for(i = 8, Rp = 0; Rp == 0 && i < 50; i += 8) 
	  modIBPRRIOAPSF(M,BRILBRI(I),LELTI(FIRST(P2),PO_POLY),i,_PRE_,&Ip, &Rp);
	if (PCVERBOSE) { SWRITE("Tried up to precision "); IWRITE(i - 8); SWRITE("\n"); }
	t = (Rp == 0); }
      if (t) {
	X = 0;
	goto Return; }
      for(Rs = Rp, Rt = NIL; Rs != NIL; Rs = RED(Rs)) {
	Rt = COMP(LBRIBRI(FIRST(Rs)),Rt);
      }
    }

    R = COMP(CINV(Rt),R);
  }
  R = CINV(R);
  DL = CINV(DL);

  /* LIMITATION OF THE CURRENT IMPLEMENTATION 
     I'll just assume only one pf's discrim vanishes in c. */
  if (LENGTH(DL) > 1) {
    X = 0;
    goto Return; }


  /* We'll have trouble if there's more than one real root in
     a "don't know" interval! */
  R_L = 0;
  for(Rp = R; Rp != NIL; Rp = RED(Rp))
    R_L += LENGTH(FIRST(Rp));
  if (R_L < IMAX(LENGTH(S_L)/2,LENGTH(S_R)/2) - 1) {
    X = 0;
    goto Return; }


  /**************************************/
  /* Go through the neighboring stacks! */
  /**************************************/
  i = 0;
  flag = FALSE;

  /* LIMITATION OF CURRENT IMPLEMENTATION 
     make sure the larger stack is to the right */
  if (LENGTH(S_R) < LENGTH(S_L)) {
    temp = S_L;
    S_L = S_R;
    S_R = temp; }
  J = FIRST(FIRST(DL));
  count = SECOND(FIRST(DL));


  for(S = NIL; S_R != NIL; ) { /******* BIG LOOP!!!! **********/

    c_L = FIRST(S_L);
    c_R = FIRST(S_R);

    if (LELTI(c_R,MULSUB) != NIL && FIRST(FIRST(LELTI(c_R,MULSUB))) == J)
      count--;

    if (count != 0) {
      /************************************************************
       ** No double roots here!
       ************************************************************/
      i++;
      /* Set up sample point correctly */
      SP = NIL;
      s_R = FIRST(LELTI(c_R,SIGNPF));
      for(k = 1; s_R != NIL; s_R = RED(s_R), k++) {
	if (FIRST(s_R) == 0) {
	  r = LELTI(R,k);
	  SP = LIST5(LELTI(LELTI(LELTI(P,2),k),PO_POLY),FIRST(r),LIST2(0,LIST2(RNINT(1),LIST2(1,1))),0,0); /* SP */
	  SLELTI(R,k,RED(r)); } }

      /* CONSTRUCT CELL */
      cp = LIST10(FIRST(c_R),NIL,THIRD(c_R),UNDET,SP,
		  CCONC(LELTI(c,INDX),LIST1(i)),COMP(FIRST(LELTI(c_R,SIGNPF)),LELTI(c,SIGNPF)),
		  NOTDET,
		  LELTI(c_R,DEGSUB),LELTI(c_R,MULSUB));
      S = COMP(cp,S);
      flag = FALSE; 
      if (RED(S_L) == NIL && RED(S_R) != NIL)
	S_R = RED(S_R);
      else {
	S_L = RED(S_L);
	S_R = RED(S_R); }
    }

    else {
      /************************************************************
       ** DOUBLE ROOTS HERE
       ************************************************************/
      i++;

      /* Contruct and add SIGNPF and SAMPLEPOINT and MULT*/
      SP = NIL;
      m = LIST1(LIST2(J,2));
      s_R = FIRST(LELTI(c_R,SIGNPF));
      so = FIRST(LELTI(FIRST(S),SIGNPF));
      for(s = NIL, k = 1; s_R != NIL; s_R = RED(s_R), so = RED(so), k++) {
	if (FIRST(s_R) == 0) {
	  r = LELTI(R,k);
	  SP = LIST5(LELTI(LELTI(LELTI(P,2),k),PO_POLY),FIRST(r),LIST2(0,LIST2(RNINT(1),LIST2(1,1))),0,0); /* SP */
	  SLELTI(R,k,RED(r));
	  s = COMP(0,s);
	}
	else
	  s = COMP(FIRST(so),s); }
      s = COMP(CINV(s),LELTI(c,SIGNPF));

      cp = LIST10(FIRST(c_R),NIL,THIRD(c_R),UNDET,SP,
		  CCONC(LELTI(c,INDX),LIST1(i)),s,NOTDET,
		  LELTI(c_R,DEGSUB),m);
      
      S = COMP(cp,S);


      S_R = RED3(S_R);
      if (RED(S_L) != NIL)
	S_L = COMP(0,S_L);
      count--;
    } }
  

  /***********************************/
  /* Add in sector sample points! ****/
  /***********************************/
  S = CINV(S);
  prev = NIL;
  Sp = S;
  while(Sp != NIL) {
    ADV(Sp,&sor,&Sp);
    if (Sp == NIL)
      next = NIL;
    else {
      ADV(Sp,&nextc,&Sp);
      next = FIRST(SECOND(LELTI(nextc,SAMPLE))); }
    s2 = CSSP(prev,next);

    // set it!
    c1 = LIST2(RNINT(1),LIST2(1,1)); //ANF element alpha
    c2 = LIST2(s2,LIST2(0,1)); // ANF element s2
    SLELTI(sor,SAMPLE,LIST3(M,I,LIST2(c1,c2)));

    if (Sp != NIL)
      prev = SECOND(SECOND(LELTI(nextc,SAMPLE)));
  }

  

  SLELTI(c,CHILD,S);
  X = 1;

 Return:
  return X;
}

