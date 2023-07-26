
/*
  Lifts over cell c in CAD D with projection factor set P.
  c must satisfy CELLSRRQ2D, and both siblings of c must
  already have their child stacks in place.  Needs L, a list
  of 2-level projection factors whose discriminants have
  order 1 zeros in c.
  Return 1 if successfull, otherwise 0
*/

#include "lift2d.h"
#define _CUT_OUT_FOR_TESTING_

//#define _PRE_ 22
const Word qe_ISO_PREC_ = 22;

void PrettyPrintModIBPOutput(Word L) {
  PushOutputContext(cerr);
  SWRITE("\n");
  for(Word Lp = L; Lp != NIL; Lp = RED(Lp)) {
    Word ap, bp, e, F, t;
    FIRST5(FIRST(Lp),&ap,&bp,&e,&F, &t);
    SWRITE("LBRNI: (");
    LBRNWRITE(ap);
    SWRITE(", ");
    LBRNWRITE(bp);
    SWRITE("), e = ");
    IWRITE(e);
    SWRITE(", F = ");
    IPDWRITE(2,F,LIST2(LFS("x"),LFS("y")));
    SWRITE(", t = ");
    IWRITE(t);
    SWRITE("\n");
  }  
  SWRITE("\n");
  PopOutputContext();
}


Word LIFTSRD2D(Word c, Word D, Word P, Word L)
{
  Word S_L,S_R,s_L,s_R,S,i,c_L,c_R,cp,flag,m_L,m_R,so,mo,m,s,j;
  Word M,I,P2,Rp,t,R,Rs,Rt,SP,r,k,c1,c2,prev,Sp,sor,next,s2,nextc,X;
  Word DL = NIL, Rps,pf,a,b,e,temp,count,J;

  Word COORDLIST(LIST1(RPFIP(1,LIST2(1,1)))); // This is (alpha), which will appear in position 5 of each sample point
  
  /******************************************************************
   ** PART 1 : Set up R and DL.
   ** R  : A list of lists (R_1,...,R_w) where R_i = (a,b,e,F)
   **      where (a,b) is a (not logarithmic) binary rational isolating
   **      interval for a simple root of F, and that value is a root
   **      of projection factor P_2,i.  The root is simple if e=0
   **      and double otherwise.
   ** DL : as described a few lines below
   ******************************************************************/
  /* Initialization */
  S_L = LELTI(CELLLEFTSIB(c,D),CHILD); 
  S_R = LELTI(CELLRIGHTSIB(c,D),CHILD);
  S = NIL;

  // Set snumL[i] and snumR[i] to the number of sections of poly (2,i) in left( resp. right) nbr stack
  P2 = LELTI(P,2);
  int N = LENGTH(P2);  
  vector<int> snumL(N+1,0);
  vector<int> snumR(N+1,0);
  for(Word L = P2; L != NIL; L = RED(L)) {
    int j = THIRD(LELTI(FIRST(L),PO_LABEL));
    if (j > N) { N = j; snumL.resize(N+1); snumR.resize(N+1); }
  }
  for(Word C = RED(S_L); C != NIL; C = RED2(C)) { // check left stack
    Word cell = FIRST(C);
    for(Word mlist = LELTI(cell,MULSUB); mlist != NIL; mlist = RED(mlist)) {
      Word i, j;
      FIRST2(FIRST(mlist),&i,&j);
      snumL[i]++;
    }
  }
  for(Word C = RED(S_R); C != NIL; C = RED2(C)) { // check right stack
    Word cell = FIRST(C);
    for(Word mlist = LELTI(cell,MULSUB); mlist != NIL; mlist = RED(mlist)) {
      Word i, j;
      FIRST2(FIRST(mlist),&i,&j);
      snumR[i]++;
    }
  }

  
  /* Get coordinate info for c: M the min pol, I the interval */
  FIRST2(LELTI(c,SAMPLE),&M,&I);

  /* PushOutputContext(cerr); */
  /* SWRITE("\n\n###################### Cell ");  OWRITE(LELTI(c,INDX)); SWRITE("\n"); */
  /* SWRITE("I = "); RNWRITE(FIRST(I)); SWRITE(", "); RNWRITE(SECOND(I)); SWRITE("\n");  */
  /* PopOutputContext();   */

  /* Isolating the roots of all 2-level projection factors over c. 
     Note:  I know this is inefficient! */
  R = NIL;
  for(P2 = LELTI(P,2); P2 != NIL; P2 = RED(P2)) {
    /**********************************************
     ** LOOP OVER EACH 2-LEVEL PROJECTION FACTOR pf
     **********************************************/
    pf = FIRST(P2);
    Word p_o = LELTI(pf,PO_POLY);

    // Calculate the expected number of roots of p_o
    Word pindex = THIRD(LELTI(pf,PO_LABEL));
    Word delta = abs(snumL[pindex] - snumR[pindex]);
    if (delta != 0 && delta != 2)
      FAIL("LIFTSRD2D","LIFTSRD2D should only be called over multiplicity one roots of discriminants!");
    Word expNumRoots = (delta == 0 ? snumL[pindex] : 1 + min(snumL[pindex],snumR[pindex]));
    
    //PushOutputContext(cerr); SWRITE("Processing ... "); IPDWRITE(2,p_o,LIST2(LFS("x"),LFS("y"))); SWRITE("\n\n"); PopOutputContext();
      
    /***** pf's discriminant vanishes in c *****/
    if (LSRCH(pf,L)) {
#ifndef _CUT_OUT_FOR_TESTING_
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
#else
      Word p_m = p_o;
#endif
      /* First attempt to isolate roots! (Using Hardware!)*/
      i = 8;
      modIBPRRIOAP(M,BRILBRI(I),p_m,qe_ISO_PREC_,&Rp,&t);
      
      /* If the first attempt fails, try again with software floats! */
      if (t != 0 || LENGTH(Rp) != expNumRoots) {
	//PushOutputContext(cerr); SWRITE(">>>>>>>>>>>>>>>> trying with Software Floats!\n"); PopOutputContext();
	Word Ip = BRILBRI(I);
	for(i = 8, Rp = 0; (Rp == 0 || LENGTH(Rp) != expNumRoots) && i < 50; i += 8) 
	  modIBPRRIOAPSF(M,Ip,p_m,i,qe_ISO_PREC_,&Ip, &Rp);
	if (PCVERBOSE) { SWRITE("Tried up to precision "); IWRITE(i - 8); SWRITE("\n"); }
	if (Rp == 0) {
	  if (PCVERBOSE) { SWRITE("Even the highprecision call to modIBPRRIOAPSF failed!\n"); }
	  X = 0;
	  goto Return; }
	t = 0;
      }
      
      //PrettyPrintModIBPOutput(Rp);

#ifndef _CUT_OUT_FOR_TESTING_      
      //!!! Unshift each isolating interval (using current precision i), and rep. as BRI not LBRI
      Word* SI = GETARRAY(2*i+6);
      Word* SJ = GETARRAY(2*i+6);
      Word* SK = GETARRAY(2*i+6);
      for(Rs = Rp, Rt = NIL; Rs != NIL; Rs = RED(Rs)) {
	//PushOutputContext(cerr); SWRITE("\n\nFIRST(Rs): "); OWRITE(FIRST(Rs)); SWRITE("\n"); PopOutputContext();
	Word ap, bp, e, F, trend;
	FIRST4(FIRST(Rs),&ap,&bp,&e,&F,&trend);
	LBRISI(LIST2(ap,bp),i,SI);
	RSI(i,a_n,a_d,SJ);
	SISUM(SI,SJ,SK);
	Word SK_lbri = SILBRI(SK);
	Word I_next = SK_lbri;
	//PushOutputContext(cerr); SWRITE("\n\nI_next: "); OWRITE(I_next); SWRITE("\n"); PopOutputContext();	
	I_next = LIST5(FIRST(I_next),SECOND(I_next),e,F,trend);
	
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
      Rt = CINV(Rt);
#else
      Rt = Rp;
#endif
      // Find index of pf's double root
      Word j = 1, Rts = Rt;
      for(; Rts != NIL && THIRD(FIRST(Rts)) == 0; Rts = RED(Rts))
	j++;
      if (Rts == NIL) { FAIL("LIFTSRD2D","No double root found!"); }
      DL = COMP(LIST2(THIRD(LELTI(pf,PO_LABEL)),j),DL);
    }

    /***** pf's discriminant does NOT vanish in c. *****/
    else { 
      i = 8;
      IBPRRIOAP(M,BRILBRI(I),p_o,qe_ISO_PREC_,&Rp,&t); // CHRIS NOTE ... should be mod version?
      if (t) {/* This line is my test stuff! */
	Word Ip = BRILBRI(I);
	for(i = 8, Rp = 0; Rp == 0 && i < 50; i += 8) 
	  modIBPRRIOAPSF(M,BRILBRI(I),LELTI(FIRST(P2),PO_POLY),i,qe_ISO_PREC_,&Ip, &Rp);
	if (PCVERBOSE) { SWRITE("Tried up to precision "); IWRITE(i - 8); SWRITE("\n"); }
	t = (Rp == 0); }
      if (t) {
	X = 0;
	goto Return; }
      for(Rs = Rp, Rt = NIL; Rs != NIL; Rs = RED(Rs)) {
	Word tmp = CCONC(FIRST(Rs),LIST2(0,p_o));
	Rt = COMP(tmp,Rt);
      }
      Rt = CINV(Rt);
    }

    if (LENGTH(Rt) != expNumRoots) { X = 0; goto Return; }    
    R = COMP(Rt,R);
  }
  R = CINV(R);
  //PushOutputContext(cerr); SWRITE("\n\nR: "); OWRITE(R); SWRITE("\n"); PopOutputContext();

  /******************************************************************
   ** PART 2 - Build the cells 
   ******************************************************************/
  
  /* LIMITATION OF THE CURRENT IMPLEMENTATION 
     I'll just assume only one pf's discrim vanishes in c. */
  if (LENGTH(DL) > 1) {
    X = 0;
    goto Return; }


  // I believe this is no longer relevent
  /* /\* We'll have trouble if there's more than one real root in */
  /*    a "don't know" interval! *\/ */
  /* R_L = 0; */
  /* for(Rp = R; Rp != NIL; Rp = RED(Rp)) */
  /*   R_L += LENGTH(FIRST(Rp)); */
  /* if (R_L < IMAX(LENGTH(S_L)/2,LENGTH(S_R)/2) - 1) { */
  /*   X = 0; */
  /*   goto Return; } */


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
	  Word Jp = FIRST(r);
	  Word ap, bp, e, F, trend;
	  FIRST5(Jp,&ap,&bp,&e,&F,&trend);
	  if (ap == 0 && ap == bp) {
	    SP = LIST3(M,
		       I,
		       CCONC(COORDLIST,0));
	  }
	  else {
	    SP = LIST6(RPFIP(2,F),
		       LIST2(LBRNRN(ap),LBRNRN(bp)),
		       M,
		       I,
		       COORDLIST,
		       trend); 
	    SLELTI(R,k,RED(r)); } }}

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
      so = S == NIL ? NIL : FIRST(LELTI(FIRST(S),SIGNPF));
      SP = NIL; // this should change!
      for(s = NIL, k = 1; s_R != NIL; s_R = RED(s_R), (so == NIL || (so = RED(so))), k++) {
	if (FIRST(s_R) == 0) {
	  r = LELTI(R,k);
	  Word Jp = FIRST(r);
	  Word ap, bp, e, F, trend;
	  FIRST5(Jp,&ap,&bp,&e,&F,&trend);
	  SP = LIST6(RPFIP(2,F),
		     LIST2(LBRNRN(ap),LBRNRN(bp)),
		     M,
		     I,
		     COORDLIST,
		     trend);
	  SLELTI(R,k,RED(r));	  
	  s = COMP(0,s);
	}
	else
	  s = COMP(FIRST(so),s); }
      s = COMP(CINV(s),LELTI(c,SIGNPF));

      cp = LIST10(FIRST(c_R),NIL,THIRD(c_R),UNDET,SP,
		  CCONC(LELTI(c,INDX),LIST1(i)),s,NOTDET,
		  LELTI(c_R,DEGSUB),m);

      //PushOutputContext(cerr); SWRITE("new cell create: "); OWRITE(cp); SWRITE("\n"); PopOutputContext();
      
      S = COMP(cp,S);

      S_R = RED3(S_R);
      if (RED(S_L) != NIL)
	S_L = COMP(0,S_L);
      count--;
    } }
  

  /***********************************/
  /* Add in sector sample points! ****/
  /***********************************/
  Word nextI, nextS;
  S = CINV(S);
  prev = NIL;
  Sp = S;
  while(Sp != NIL) {
    ADV(Sp,&sor,&Sp);
    if (Sp == NIL)
      next = NIL;
    else {
      ADV(Sp,&nextc,&Sp);
      nextS = LELTI(nextc,SAMPLE);
      nextI = NIL;
      if (LENGTH(nextS) == 3) {
	//PushOutputContext(cerr); SWRITE("Error!"); PopOutputContext();
	FAIL("LIFTSRD2D","Primitive sample point ... which is not expected!");
      }
      else {
	nextI = SECOND(nextS);
      }
      next = FIRST(nextI); }
    //PushOutputContext(cerr); SWRITE("calling CSSP: "); OWRITE(prev); SWRITE(", "); OWRITE(next); SWRITE("\n"); PopOutputContext();
    s2 = CSSP(prev,next);

    // set it!
    c1 = LIST2(RNINT(1),LIST2(1,1)); //ANF element alpha
    c2 = LIST2(s2,LIST2(0,1)); // ANF element s2
    SLELTI(sor,SAMPLE,LIST3(M,I,LIST2(c1,c2)));

    if (Sp != NIL)
      prev = SECOND(nextI);
  }

  /* PushOutputContext(cerr); */
  /* for(Word i = 1, St = S; St != NIL; (St = RED(St)), i++) { */
  /*   SWRITE("child "); IWRITE(i); SWRITE(": "); OWRITE(FIRST(St)); SWRITE("\n\n"); */
  /* } */
  /* PopOutputContext(); */

  
  SLELTI(c,CHILD,S);
  X = 1;

 Return:
  return X;
}

