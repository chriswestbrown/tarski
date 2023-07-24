/*************************************************************
 **
 **  Rend_Sample.cc
 **
 **
 *************************************************************/
#include "Rend_Sample.h"  
#include "../lift2D/lift2d.h"

/*************************************************************
 **  Class Rend_Sample Function Definitions
 *************************************************************/
Rend_Sample_1DS::Rend_Sample_1DS(Word C,Word P)
{
  Word T,i,t1,t2,t3,j1,j2,s1,s2,s;

  //-- Set P_index --//
  for(i = 1, s = FIRST(LELTI( C, SIGNPF )); FIRST(s) != 0; i++, s = RED(s));
  P_index = THIRD( LELTI( LELTI(LELTI(P,1),i) , PO_LABEL) );

  //-- Set A to the minpol of sample point, and I to isolating int --/
  T = LELTI( C , SAMPLE );
  FIRST3(T,&t1,&t2,&t3); // t3 is a LIST of ANF elements.
  ANFAF(t1,t2,FIRST(t3),&(A.W),&T);
  FIRST2(T,&j1,&j2);
  j1 = RNLBRN(j1);
  j2 = RNLBRN(j2);
  I.W = LIST2( j1 , j2 );

  //-- Set t to the trend. --//
  s1 = IUPBRES(A.W,j1);
  s2 = IUPBRES(A.W,j2);
  if (s1 == 0) {
    t = 0;
    I.W = LIST2(j1,j1); }
  else if (s2 == 0) {
    t = 0;
    I.W = LIST2(j2,j2); }
  else
    t = s2;	 
}


Rend_Sample_1DS::~Rend_Sample_1DS()
{
  
}

/*************************************************************
 **  Refines I to 2^k or less, and returns binary rational 
 **  middle of interval.
 *************************************************************/
Word Rend_Sample_1DS::coordinate(int k)
{
  Word P,J,j1,j2,js1,js2,s1,s2;

  P = A.W; J = I.W;

Step1: /* Refine root to desired accuracy.  Note: I'm assuming
that J is a binary rational. */
  if (t != 0)
    J = IPIIRB(P,J,t,k);

Step2: /* Save interval refinement and return. */
  I.W = J;
  FIRST2(J,&j1,&j2);
  return LBRNP2PROD(LBRNSUM(j1,j2),-1);

}

Word Rend_Sample_1DS::round(int k, int roundup)
{
  Word P,J,j1,j2,js1,js2,s1,s2;

  P = A.W; J = I.W;

Step1: /* Refine root to desired accuracy.  Note: I'm assuming
that J is a binary rational. */
  if (t != 0)
    J = IPIIRB(P,J,t,k);

Step2: /* Save interval refinement and return. */
  I.W = J;
  FIRST2(J,&j1,&j2);
  return (roundup) ? j2 : j1;
}

/*************************************************************
 **  weakcompare
 *************************************************************/
Word Rend_Sample_1DS::weakcompare(Word R)
{
  Word i1,i2;
  FIRST2(I.W,&i1,&i2);
  if ( ICOMP(R, LBRNRN(i1) ) <= 0 )
    return -1;
  if ( ICOMP(R, LBRNRN(i2) ) >= 0 )
    return 1;
  return 0;
}



/*************************************************************
 **  Class Rend_Sample_1DO Function Definitions
 *************************************************************/
Rend_Sample_1DO::Rend_Sample_1DO(Rend_Cell *dad)
{
  Word i;
  i = dad -> index;
  l = &(dad -> parent -> child[i-1]);
  r = &(dad -> parent -> child[i+1]);
  L.W = NIL;
}


Rend_Sample_1DO::~Rend_Sample_1DO()
{
  
}


Word Rend_Sample_1DO::coordinate(int k)
{
  Word e,j1,j2,J,kp = k;
  do {
    j1 = l -> sample -> round(kp,1);
    j2 = r -> sample -> round(kp,0);
    kp--;
  } while(LBRNCOMP(j1,j2) >= 0);

  /*  e = LBRNFIE(1,k-1);
  if (l -> index != AD2D_N_In )
    j1 = LBRNSUM(j1,e);
  if (r -> index != AD2D_Infy)
    j2 = LBRNDIF(j2,e);
    */

  J = LIST2(j1,j2);
  return J;
}

/*************************************************************
 **  Class Rend_Sample_2DS
 *************************************************************/
Rend_Sample_2DS::Rend_Sample_2DS(Word C)
{
  Word T,i,t1,t2,t3,j1,j2,s1,s2,s;
  Word tB,tJ,tA,tI,d1,d2,d3,d4,d5;

  this->C = C;
  
  //-- Set A to the minpol of sample point, and I to isolating int --/
  T = LELTI( C , SAMPLE );
  // PushOutputContext(cerr);
  // SWRITE("|T| = "); IWRITE(LENGTH(T)); SWRITE(" ");
  // SWRITE("cell"); OWRITE(LELTI(C,INDX));
  // SWRITE("\n");
  // PopOutputContext();
  if (LENGTH(T) < 5) {
    //-- The sample point is in primitive representation. --//
    FIRST3(T,&t1,&t2,&t3); 
    t3 = SECOND(t3); // t3 is a LIST of ANF elements. 
    ANFAF(t1,t2,t3,&(A.W),&(tJ)); 
    I.W = LIST2(RNLBRN(FIRST(tJ)),RNLBRN(SECOND(tJ)));

    //-- Set t to the trend. --//
    FIRST2(I.W,&j1,&j2);
    s1 = IUPBRES(A.W,j1);
    s2 = IUPBRES(A.W,j2);
    if (s1 == 0) {
      t = 0;
      I.W = LIST2(j1,j1); }
    else if (s2 == 0) {
      t = 0;
      I.W = LIST2(j2,j2); }
    else
      t = s2;
  }
  else {
    /* The sample point is not in primitive representation.
       Length 5: qepcad "extended" rep.
       Length > 5: LIFTSRD2D rep.     */    
    FIRST5(T,&tB,&tJ,&tA,&tI,&d1);
    I.W = LIST2(RNLBRN(FIRST(tJ)),RNLBRN(SECOND(tJ)));
  }
}


Rend_Sample_2DS::~Rend_Sample_2DS()
{
  
}

/*
 LBRNILOW(J)
 LBRN Interval, logarithm of width
 Assume J is a proper *open* interval
*/
Word LBRNILOW(Word J) {
  Word w = LBRNDIF(SECOND(J),FIRST(J));
  Word kd = SECOND(w); // exponent of denominator
  Word kn = ILOG2(FIRST(w));
  return kn - kd;
}

/*************************************************************
 **  Refines I to 2^k or less, and returns binary rational 
 **  lower endpoint.
 *************************************************************/
Word Rend_Sample_2DS::coordinate(int k)
{
  Word J,j1,j2,JL,S,tB,tJ,tA,tI,mu,i;

  
Step1: /* Initialize and decide if refinement is even necessary. */
  S = LELTI(C,SAMPLE);
  J = I.W;

  if (EQUAL(FIRST(J),SECOND(J)))
    return FIRST(J);
  // if (LSILW(J) <= k)
   if (LBRNILOW(J) <= k)
    return LBRNP2PROD(LBRNSUM(FIRST(J),SECOND(J)),-1);

  // DEBUG!!!
  // PushOutputContext(cerr);
  // SWRITE("("); LBRNWRITE(FIRST(J)); SWRITE(","); LBRNWRITE(SECOND(J)); SWRITE(") ");
  // SWRITE("Prec is "); IWRITE(LBRNILOW(J)); SWRITE("\n");
  // OWRITE(S); SWRITE("\n");
  // SWRITE("S = "); OWRITE(S); SWRITE("\n");
  // PopOutputContext();

  if (LENGTH(S) == 6) { // This case comes from LIFTSRD2D sample point
    Word B, J, M, I, cl, trend, a, A;
    FIRST6(S,&B,&J,&M,&I,&cl,&trend);
    IPSRP(2,B,&a,&A);
    Word Ip = BRILBRI(I), Jp = BRILBRI(J), Jnew, t;
  
    IBPRSRAN(M,Ip,B,-k,Jp,trend,&Jnew,&t); // refine using hardware floats
    Jp = Jnew;
    Word p = 0;
    while(t != 0) {
      p += 8;
      IBPRSRANSF(M,Ip,B,-k,Jp,trend,p,&Jnew,&t); // refine using software
      Jp = Jnew;
    }
    J = Jp;
  }
  else if (LENGTH(S) == 5) { // This case comes from qepcad extended sample point
    Word M, I, Mp, Ip, bp, F,M1,Ms,Is,j;
    FIRST5(S,&M,&I,&Mp,&Ip,&bp);
    F = AFPNIP(Mp,M);  
    M = AFPICR(1,M);
    IPSRP(2,M,&M1,&M);
    AMUPMPR(Mp,Ip,M,I,F,&Is,&j);
    Ms = LELTI(F,j); 
    Is = IPSIFI(Ms,Is);  
    //ANDWRITE(Ms,Is,6); SWRITE("\n");
    Word tr = -IUPBRES(Ms,RNLBRN(FIRST(Is)));
    // PushOutputContext(cerr);
    // SWRITE("ASDFSADFSAFSF: ");
    // IPDWRITE(1,Ms,LIST1(LFS("x"))); SWRITE(" ");
    // OWRITE(Is); SWRITE("\n");
    Word Jnew = IPIIRB(Ms,BRILBRI(Is),tr,k);
    J = Jnew;    
    // PopOutputContext();
  }
  else { // This case comes from qepcad primitive sample point
    /* Refine root to desired accuracy.  Note: I'm assuming
       that J is a binary rational. */
    if (t != 0)
      J = IPIIRB(A.W,J,t,k);
  }

  
Step2: /* Further refinement required. */
  // if (ISLIST(FIRST(S))) {
  //   FIRST5(S,&tB,&tJ,&tA,&tI,&mu);
  //   if (mu != 2) {
  //     // what to do here?
  //   }
  //   else {
  //     // What to do here?
  //   }
  // }
  // else {
  //   /* Refine root to desired accuracy.  Note: I'm assuming
  //      that J is a binary rational. */
  //   if (t != 0)
  //     J = IPIIRB(S,J,t,k);

  I.W = J;
  return LBRNP2PROD(LBRNSUM(FIRST(J),SECOND(J)),-1);

}


/*************************************************************
 **  Class Rend_Sample_2DC
 *************************************************************/
Rend_Sample_2DC::Rend_Sample_2DC(Word C,Word P)
{
  Word i,s;
  //-- Set P_index --//
  for(i = 1, s = FIRST(LELTI( C, SIGNPF )); FIRST(s) != 0; i++, s = RED(s));
  P_index = THIRD( LELTI( LELTI(LELTI(P,2),i) , PO_LABEL) );

  L.W = NIL;

}

Rend_Sample_2DC::~Rend_Sample_2DC()
{
  
}

Word Rend_Sample_2DC::coordinate(int k)
{
  return L.W;
}
  
void Rend_Sample_2DC::add_point(Word p)
{
  L.W = COMP(p,L.W);
}

void Rend_Sample_2DC::clear_points()
{
  L.W = NIL;
}

/*************************************************************
 **  Class Rend_Sample_BR
 *************************************************************/
Rend_Sample_BR::Rend_Sample_BR(Word a)
{
  N.W = a;
}

Rend_Sample_BR::~Rend_Sample_BR()
{
  
}

Word Rend_Sample_BR::coordinate(int k)
{
  return N.W;
}
  


