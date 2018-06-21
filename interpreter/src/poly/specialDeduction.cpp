/****************************************************************************
 * This whole file is dedicated to making a "special deduction".
 * See documentation for "specialLinCombQ" below.
 ****************************************************************************/
#include "poly.h"
#include "../formrepconventions.h"

using namespace std;

namespace tarski {

// DEFINITION: 3-bit number abc, with a  = lsb, 
//Interval qualifiers are leftEndpoint, rightEndpoint, all, none
const int iq_left = 1, iq_right = 4, iq_all = 7, iq_none = 0, iq_mid = 2;

// DEFINITION: Condition is FALSE or a list (I,t_strict,t_equal)

// New Condition: find conditions on b for x*(P + b*Q) >= 0 given x sigma_x 0
// x*(P + b*Q) >= 0 --> P + b*Q sigma 0 --> b sigma^* -P/Q
// Condition is 
// FALSE or [x,y] where x,y rational numbers or -1 for -infinty or +1 for 
// positive infinity, and x<=y.
Word newConditionIntegerConstants(Word sigma_x, Word P, Word Q)
{
  if (sigma_x == EQOP) { return LIST3(LIST2(-1,1),iq_none,iq_all); }

  //                          LTOP,EQOP,LEOP,GTOP,NEOP,GEOP,ALOP
  static int dedSigma[] = {-1,LEOP,ALOP,LEOP,GEOP,EQOP,GEOP,EQOP};
  int sigma = dedSigma[sigma_x];
  int sigmas = sigma;
  if (ISIGNF(Q) == 0) 
  {
    int LHSsigma = T_prod[sigma_x][signToSigma(ISIGNF(P))];
    switch(LHSsigma)
    {
    case GTOP: return LIST3(LIST2(-1,1),iq_all,iq_none); break;
    case EQOP: return LIST3(LIST2(-1,1),iq_none,iq_all); break;
    case GEOP: return LIST3(LIST2(-1,1),iq_none,iq_none); break;
    default: return FALSE;
    }
    // if (consistentWith(signToSigma(ISIGNF(P)),sigma))
    // {
    //   if (P == 0) return LIST3(LIST2(-1,1),iq_none,iq_all); <-- OLD VERSION!
    //   else return LIST3(LIST2(-1,1),iq_all,iq_none);
    // }
    // else
    //   return FALSE;
  }
  if (ISIGNF(Q) < 0) sigmas = reverseRelop(sigma);
  Word RHS = RNRED(INEG(P),Q);
  switch(sigmas)
  {
  case ALOP: return LIST3(LIST2(-1,1),iq_none,iq_all);
  case EQOP: return LIST3(LIST2(RHS,RHS),iq_none,iq_all);
  case LEOP: return LIST3(LIST2(-1,RHS),(sigma_x & EQOP ? iq_none : iq_left|iq_mid),iq_right);
  case GEOP: return LIST3(LIST2(RHS,1), (sigma_x & EQOP ? iq_none : iq_mid|iq_right),iq_left);
  default: return FALSE;
  }
}

// New Condition: find conditions on b for x*(P + b*Q) >= 0 given x sigma_x 0
// x*(P + b*Q) >= 0 --> P + b*Q sigma 0 --> b sigma^* -P/Q
// Condition is 
// TRUE or FALSE or [x,y] where x,y rational numbers or -1 for -infinty or +1 for 
// positive infinity, and x<=y.
// One of the two relops must be EQOP
Word newConditionRelopsOLD(Word sigma_x, Word sigma_P, Word sigma_Q)
{
  //                          LTOP,EQOP,LEOP,GTOP,NEOP,GEOP,ALOP
  static int dedSigma[] = {-1,LEOP,ALOP,LEOP,GEOP,EQOP,GEOP,EQOP};
  int sigma;
  Word res = NIL;

  // x = 0 case:  x*(P + b*Q) = 0 for all b!
  if (sigma_x == EQOP) { res = LIST3(LIST2(-1,1),iq_none,iq_all); goto Return; }

  sigma = dedSigma[sigma_x];
  if (sigma_Q == EQOP)
  {
    switch(T_prod[sigma_x][sigma_P]) { // if Q = 0, we check whether x*P >= 0
    case GTOP: res = LIST3(LIST2(-1,1),iq_all,iq_none); break;
    case GEOP: res = LIST3(LIST2(-1,1),iq_none,iq_none); break;
    case EQOP: res = LIST3(LIST2(-1,1),iq_none,iq_all); break;
    default:   res = FALSE; break; }
    goto Return;
  }
  switch(T_prod[sigma][sigma_Q])
  {
  case EQOP: // In this case, P must equal zero!
    if (sigma_P == EQOP)
      res = LIST3(LIST2(0,0),iq_none,iq_all);
    else
      res = FALSE; // You'd need to know actual integer values for P and Q to find interval I
    break;
  case LEOP: 
    if (!(sigma_x & EQOP) && !(sigma_Q & EQOP))
      res = LIST3(LIST2(-1,0),iq_left|iq_mid,iq_right);
    else
      res = LIST3(LIST2(-1,0),iq_none,iq_right);
    break;
  case GEOP: 
    if (!(sigma_x & EQOP) && !(sigma_Q & EQOP))
      res = LIST3(LIST2(0,1),iq_mid|iq_right,iq_left);
    else
      res = LIST3(LIST2(0,1),iq_none,iq_left);
    break;
  default: res = FALSE; break;
  }
 Return:
  return res;
}

// New Condition: find conditions on b for x*(P + b*Q) >= 0 
// given x sigma_x 0, P sigma_P 0, and Q sigma_Q 0.  Note that the only considered
// intervals for b are (-infy,0], [0,0], or [0,infty) ... no other 
// intervals, in particlular intervals depending parametrically
// on P or Q, are considered.  The result is FALSE or (I,c_strict,c_equal),
// where I is the list (-1,0), (0,0) or (0,1).  c_strict and c_equal tell
// us which portion of interval I (left-end, middle, right-end) give a b-value that satisfies
// the above inequality strictly and with equality, respectively.
Word newConditionRelopsMOD(Word sigma_x, Word sigma_P, Word sigma_Q)
{
  // This table, indexed by sigma_x, sigma_P and sigma_Q, describes interval, strict and equals
  // info by a 3-bit number 1 = (-infty,0), 2 = [0,0], 3 = (0,+infty).  Note that c_strict and
  // c_equal are in terms of interval I, so translation is necessary ... which the code below
  // the table does!
static int ncrT[8][8][8][3] = {
{ {{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7}} // x^2<0,P^2<0
 ,{{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7}} // x^2<0,P<0
 ,{{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7}} // x^2<0,P=0
 ,{{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7}} // x^2<0,P<=0
 ,{{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7}} // x^2<0,P>0
 ,{{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7}} // x^2<0,P/=0
 ,{{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7}} // x^2<0,P>=0
 ,{{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7}} // x^2<0,P^2>=0
},
{ {{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7}} // x<0,P^2<0
 ,{{7,7,7},{6,6,0},{7,7,0},{6,6,0},{3,3,0},{2,2,0},{3,3,0},{2,2,0}} // x<0,P<0
 ,{{7,7,7},{6,4,2},{7,0,7},{6,0,2},{3,1,2},{2,0,2},{3,0,2},{2,0,2}} // x<0,P=0
 ,{{7,7,7},{6,4,0},{7,0,0},{6,0,0},{3,1,0},{2,0,0},{3,0,0},{2,0,0}} // x<0,P<=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x<0,P>0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x<0,P/=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x<0,P>=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x<0,P^2>=0
},
{ {{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7}} // x=0,P^2<0
 ,{{7,7,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7}} // x=0,P<0
 ,{{7,7,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7}} // x=0,P=0
 ,{{7,7,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7}} // x=0,P<=0
 ,{{7,7,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7}} // x=0,P>0
 ,{{7,7,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7}} // x=0,P/=0
 ,{{7,7,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7}} // x=0,P>=0
 ,{{7,7,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7},{7,0,7}} // x=0,P^2>=0
},
{ {{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7}} // x<=0,P^2<0
 ,{{7,7,7},{6,0,0},{7,0,0},{6,0,0},{3,0,0},{2,0,0},{3,0,0},{2,0,0}} // x<=0,P<0
 ,{{7,7,7},{6,0,2},{7,0,7},{6,0,2},{3,0,2},{2,0,2},{3,0,2},{2,0,2}} // x<=0,P=0
 ,{{7,7,7},{6,0,0},{7,0,0},{6,0,0},{3,0,0},{2,0,0},{3,0,0},{2,0,0}} // x<=0,P<=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x<=0,P>0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x<=0,P/=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x<=0,P>=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x<=0,P^2>=0
},
{ {{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7}} // x>0,P^2<0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x>0,P<0
 ,{{7,7,7},{3,1,2},{7,0,7},{3,0,2},{6,4,2},{2,0,2},{6,0,2},{2,0,2}} // x>0,P=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x>0,P<=0
 ,{{7,7,7},{3,3,0},{7,7,0},{3,3,0},{6,6,0},{2,2,0},{6,6,0},{2,2,0}} // x>0,P>0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x>0,P/=0
 ,{{7,7,7},{3,1,0},{7,0,0},{3,0,0},{6,4,0},{2,0,0},{6,0,0},{2,0,0}} // x>0,P>=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x>0,P^2>=0
},
{ {{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7}} // x/=0,P^2<0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x/=0,P<0
 ,{{7,7,7},{2,0,2},{7,0,7},{2,0,2},{2,0,2},{2,0,2},{2,0,2},{2,0,2}} // x/=0,P=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x/=0,P<=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x/=0,P>0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x/=0,P/=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x/=0,P>=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x/=0,P^2>=0
},
{ {{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7}} // x>=0,P^2<0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x>=0,P<0
 ,{{7,7,7},{3,0,2},{7,0,7},{3,0,2},{6,0,2},{2,0,2},{6,0,2},{2,0,2}} // x>=0,P=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x>=0,P<=0
 ,{{7,7,7},{3,0,0},{7,0,0},{3,0,0},{6,0,0},{2,0,0},{6,0,0},{2,0,0}} // x>=0,P>0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x>=0,P/=0
 ,{{7,7,7},{3,0,0},{7,0,0},{3,0,0},{6,0,0},{2,0,0},{6,0,0},{2,0,0}} // x>=0,P>=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x>=0,P^2>=0
},
{ {{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7},{7,7,7}} // x^2>=0,P^2<0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x^2>=0,P<0
 ,{{7,7,7},{2,0,2},{7,0,7},{2,0,2},{2,0,2},{2,0,2},{2,0,2},{2,0,2}} // x^2>=0,P=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x^2>=0,P<=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x^2>=0,P>0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x^2>=0,P/=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x^2>=0,P>=0
 ,{{7,7,7},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}} // x^2>=0,P^2>=0
}
};
  Word res;
  const int *t = ncrT[sigma_x][sigma_P][sigma_Q];
  switch(t[0]) {
  case NOOP: res = FALSE; break;
  case LTOP: res = FALSE; break;
  case EQOP: res = LIST3(LIST2(0,0), t[1]&2 ? iq_all : iq_none, t[2]&2 ? iq_all : iq_none); break;
  case LEOP: res = LIST3(LIST2(-1,0),
			 (t[1]&1 ? 3 : 0)|(t[1]&2 ? 4 : 0),
			 (t[2]&1 ? 3 : 0)|(t[2]&2 ? 4 : 0)); break;
  case GTOP: res = FALSE; break;
  case NEOP: res = FALSE; break;
  case GEOP: res = LIST3(LIST2(0,1),
			 (t[1]&2 ? 1 : 0)|(t[1]&4 ? 6 : 0), 
			 (t[2]&2 ? 1 : 0)|(t[2]&4 ? 6 : 0)); break;
  case ALOP: res = LIST3(LIST2(-1,1),t[1],t[2]); break;
  }
  return res;
}

Word newConditionRelops(Word sigma_x, Word sigma_P, Word sigma_Q)
{
  Word resMOD = newConditionRelopsMOD(sigma_x,sigma_P,sigma_Q);
  // Word resOLD = newConditionRelopsOLD(sigma_x,sigma_P,sigma_Q);
  // if (!EQUAL(resOLD,resMOD))
  // {
  //   SWRITE("\n%%%%%% ");
  //   SWRITE("args are ");
  //   OWRITE(LIST3(sigma_x,sigma_P,sigma_Q));
  //   SWRITE("OLD is ");
  //   OWRITE(resOLD);
  //   SWRITE("MOD is ");
  //   OWRITE(resMOD);
  //   SWRITE("\n");
  // }
  return resMOD;
}

// Extended RNSIGN: returns sign of rationals extended by +1 = +infinity, -1 = -infinity
Word ERNSIGN(Word x) { if (ISATOM(x)) return x; else return RNSIGN(x); }
Word ERNMAX(Word a, Word b) { return (a == 1 || b == 1) ? 1 : (a == -1 ? b : (b == -1 ? a : RNMAX(a,b))); }
Word ERNMIN(Word a, Word b) { return (a == -1 || b == -1) ? -1 : (a == 1 ? b : (b == 1 ? a : RNMIN(a,b))); }
Word ERNCOMP(Word a, Word b) { 
  return EQUAL(a,b) ? 0 : (a == -1 || b == 1 ? -1 : (a == 1 || b == -1 ? 1 : RNCOMP(a,b))) ; }
void ERNWRITE(Word x) 
{ 
  if (ISATOM(x)) { SWRITE(x == 0 ? "0" : (x > 0 ? "+INFTY" : "-INFTY")); }
  else RNWRITE(x);
}

Word addConstraint(Word newc, Word c)
{
  if (c == FALSE || newc == FALSE) return FALSE;
  Word newc_I, newc_strict, newc_equal;
  Word c_I, c_strict, c_equal;
  FIRST3(newc,&newc_I,&newc_strict,&newc_equal);
  FIRST3(c,&c_I,&c_strict,&c_equal);

  // Empty overlap case!
  Word sLR = ERNCOMP(FIRST(newc_I),SECOND(c_I));
  Word sRL = ERNCOMP(SECOND(newc_I),FIRST(c_I));
  if (sLR > 0 || sRL < 0) return FALSE;

  // Single-point overlap case!
  if (sLR == 0) { 
    return LIST3(LIST2(FIRST(newc_I),FIRST(newc_I)),
		 iq_left & newc_strict || iq_right & c_strict ? iq_all : iq_none,
		 iq_left & newc_equal  && iq_right & c_equal  ? iq_all : iq_none);
  }
  if (sRL == 0) { 
    return LIST3(LIST2(FIRST(c_I),FIRST(c_I)),
		 iq_right & newc_strict || iq_left & c_strict ? iq_all : iq_none,
		 iq_right & newc_equal  && iq_left & c_equal  ? iq_all : iq_none);
  }

  // Non-trivial overlap case!
  Word sL,sR,lb,ub;
  Word t_strict = (c_strict & iq_mid) | (newc_strict & iq_mid);
  Word t_equal  = (c_equal & iq_mid) & (newc_equal & iq_mid);
  sL = ERNCOMP(FIRST(newc_I),FIRST(c_I));
  if (sL == -1) { 
    lb = FIRST(c_I); 
    t_strict |= iq_left*((c_strict & iq_left) || (newc_strict & iq_mid));
    t_equal  |= iq_left*((c_equal & iq_left) && (newc_equal & iq_mid)); }
  else if (sL == 1) { 
    lb = FIRST(newc_I); 
    t_strict |= iq_left*((newc_strict & iq_left) || (c_strict & iq_mid));
    t_equal  |= iq_left*((newc_equal & iq_left) && (c_equal & iq_mid)); }
  else { 
    lb = FIRST(newc_I); 
    t_strict |= iq_left*((newc_strict & iq_left) || c_strict & iq_left);
    t_equal  |= iq_left*((newc_equal & iq_left) && c_equal & iq_left); }

  sR = ERNCOMP(SECOND(newc_I),SECOND(c_I));
  if (sR == 1) { 
    ub = SECOND(c_I); 
    t_strict |= iq_right*((c_strict & iq_right) || (newc_strict & iq_mid));
    t_equal  |= iq_right*((c_equal & iq_right) && (newc_equal & iq_mid)); }
  else if (sR == -1) { 
    ub = SECOND(newc_I); 
    t_strict |= iq_right*((newc_strict & iq_right) || (c_strict & iq_mid));
    t_equal  |= iq_right*((newc_equal & iq_right) && (c_equal & iq_mid)); }
  else { 
    ub = SECOND(newc_I); 
    t_strict |= iq_right*((newc_strict & iq_right) || (c_strict & iq_right)); 
    t_equal  |= iq_right*((newc_equal & iq_right) && (c_equal & iq_right)); }
  
  return LIST3(LIST2(lb,ub),t_strict,t_equal);
}


/* 
Inputs:
Polynomials P and Q, 
constraints = (Ic,tc_strict,tc_equal) for Interval Ic and interval qualifiers tc_strict, tc_equal 
comparison op ssf, and 
VarKeyedMap varSign giving variable sign information.

Output:
Either FALSE or list (I,t_strict,t_equal)
1) interval I, nonempty  & closed,
   I \subseteq Ic such that[ b \in I] ==> (A x)[ x ssf 0 ==> x*(P + b*Q) >= 0 ].
2) b in t_strict of I implies (A x)[ x ssf 0 ==> x*(P + b*Q) + Pc + b*Qc > 0 ].
3) b in t_equal  of I implies (A x)[ x ssf 0 ==> x*(P + b*Q) + Pc + b*Qc = 0 ].


r,P : saclib rep of "P", itrP iterates backwards in varP to pick up x_r,...,x_1
s,Q : saclib rep of "Q", itrQ iterates backwards in varQ to pick up x_s,...,x_1
 */
VarContext *tmp_pM = 0;
static Word f(Word r, Word P, VarSet::iterator itrP,
	      Word s, Word Q, VarSet::iterator itrQ,
	      int porq, /* P or Q or both: -1->P, 0->both, +1->Q. */
	      int ssf, VarKeyedMap<int> &varSign, Word constraints, VarSet varP, VarSet varQ
	      ,VarSet &used)
{
  Word result = NIL;


  int ded = ALOP;
  if (r == 0 && s == 0 && porq == 0)
  {
    Word newc = newConditionIntegerConstants(ssf,P,Q);
    if (false) // Ugly debugging stuff!
    {
      SWRITE("ssf = ");IWRITE(ssf);
      SWRITE(" P = "); IWRITE(P);
      SWRITE(" Q = "); IWRITE(Q);
      SWRITE(" newc = "); OWRITE(newc); SWRITE("\n");
    }
    result = addConstraint(newc,constraints);
  }
  else if (porq == 1 || P == 0)
  {
    Word L = NIL; VarSet varQp;
    while(itrQ != varQ.rend()) { L = COMP(varSign[*itrQ],L); varQp = varQp + *itrQ; --itrQ; }
    L = CINV(L);
    ded = ::signDeduce(s,Q,L);
//     VarSet reqd = minimalReqdForSignDeduce(s,Q,L,varQp,ded);
//     used = used + reqd;
    result = addConstraint(newConditionRelops(ssf,EQOP,ded),constraints);
  }
  else if (porq == -1 || Q == 0)
  {
    Word L = NIL; VarSet varPp;
    while(itrP != varP.rend()) { L = COMP(varSign[*itrP],L); varPp = varPp + *itrP; --itrP; }
    L = CINV(L);
    ded = ::signDeduce(r,P,L);
//     VarSet reqd = minimalReqdForSignDeduce(r,P,L,varPp,ded);
//     used = used + reqd;
    result = addConstraint(newConditionRelops(ssf,ded,EQOP),constraints);
  }
  else
  {
    if (*itrP == *itrQ)
    {
      // Deal with the possiblity that the max var appears in neither polynomial!
      if (PDEG(P) == 0 && PDEG(Q) == 0) { result = f(r-1,PLDCF(P),itrP-1,s-1,PLDCF(Q),itrQ-1,porq,ssf,varSign,constraints,varP,varQ,used); }
      else {
	Word res_h, newP, newQ;
  res_h = 0;
	if (PDEG(P) == PDEG(Q))
	{
	  int k = PDEG(P);
	  int newssf = (k==0 ? ssf : (T_prod[ssf][k % 2 == 1 ? varSign[*itrP] : T_square[varSign[*itrP]]]));
	  res_h = f(r-1,PLDCF(P),itrP-1,s-1,PLDCF(Q),itrQ-1,porq,newssf,varSign,constraints,varP,varQ,used);
	  newP = PRED(P);
	  newQ = PRED(Q);
	}
	else if (PDEG(P) > PDEG(Q))
	{
	  int k = PDEG(P);
	  int newssf = T_prod[ssf][k % 2 == 1 ? varSign[*itrP] : T_square[varSign[*itrP]]];
	  res_h = f(r-1,PLDCF(P),itrP-1,s,Q,itrQ,-1,newssf,varSign,constraints,varP,varQ,used);
	  newP = PRED(P);
	  newQ = Q;
	}
	else if (PDEG(P) < PDEG(Q))
	{
	  int k = PDEG(Q);
	  int newssf = T_prod[ssf][k % 2 == 1 ? varSign[*itrQ] : T_square[varSign[*itrQ]]];
	  res_h = f(r,P,itrP,s-1,PLDCF(Q),itrQ-1,+1,newssf,varSign,constraints,varP,varQ,used);
	  newP = P;
	  newQ = PRED(Q);
	}
	if (res_h == FALSE || (newP == 0 && newQ == 0)) result = res_h;
	else result = f(r,newP,itrP,s,newQ,itrQ,porq,ssf,varSign,res_h,varP,varQ,used);
      }}
    else if (*itrQ < *itrP)
    {
      // Deal with the possiblity that the max var appears in neither polynomial!
      if (PDEG(P) == 0) { result = f(r-1,PLDCF(P),itrP-1,s,Q,itrQ,porq,ssf,varSign,constraints,varP,varQ,used); }
      else {
	int k = PDEG(P);
	int newssf = T_prod[ssf][k % 2 == 1 ? varSign[*itrP] : T_square[varSign[*itrP]]];
	Word res_h = f(r-1,PLDCF(P),itrP-1,s,Q,itrQ,-1,newssf,varSign,constraints,varP,varQ,used);
	if (res_h == FALSE) result = res_h;
	else if (PDEG(PRED(P)) > 0)
	  result = f(r,PRED(P),itrP,s,Q,itrQ,porq,ssf,varSign,res_h,varP,varQ,used);
	else
	  result = f(r-1,PLDCF(PRED(P)),itrP-1,s,Q,itrQ,porq,ssf,varSign,res_h,varP,varQ,used);
      }
    }
    else
    {
      // Deal with the possiblity that the max var appears in neither polynomial!
      if (PDEG(Q) == 0) { result = f(r,P,itrP,s-1,PLDCF(Q),itrQ-1,porq,ssf,varSign,constraints,varP,varQ,used); }
      else {
	int k = PDEG(Q);
	int newssf = T_prod[ssf][k % 2 == 1 ? varSign[*itrQ] : T_square[varSign[*itrQ]]];
	Word res_h = f(r,P,itrP,s-1,PLDCF(Q),itrQ-1,+1,newssf,varSign,constraints,varP,varQ,used);
	if (res_h == FALSE) result = res_h;
	else if (PDEG(PRED(Q)) > 0)
	  result = f(r,P,itrP,s,PRED(Q),itrQ,porq,ssf,varSign,res_h,varP,varQ,used);
	else
	  result = f(r,P,itrP,s-1,PLDCF(PRED(Q)),itrQ-1,porq,ssf,varSign,res_h,varP,varQ,used);
      }
    }
  }


  // This is a disgustingly necessary debugging section
  if (false)
  {
    {
      Word vl = NIL;
      VarSet::iterator vitr = itrP; for(int i = 0; i < r; ++i) { vl = COMP(LFS(tmp_pM->getName(*vitr)),vl); --vitr; }
      IPDWRITE(r,P,vl);SWRITE("[r = ");IWRITE(r);SWRITE("]");
      SWRITE("     ");
    }
    {
      Word vl = NIL;
      VarSet::iterator vitr = itrQ; for(int i = 0; i < s; ++i) { vl = COMP(LFS(tmp_pM->getName(*vitr)),vl); --vitr; }
      IPDWRITE(s,Q,vl);SWRITE("[s = ");IWRITE(s);SWRITE("]");
      SWRITE("     ");
    }
    ERNWRITE(FIRST(FIRST(constraints)));
    SWRITE(",");
    ERNWRITE(SECOND(FIRST(constraints)));
    SWRITE("  "); 
    IWRITE(SECOND(constraints));
    SWRITE("  ");
    IWRITE(THIRD(constraints));
    SWRITE(" ssf is "); IWRITE(ssf);
    SWRITE(" porq is "); IWRITE(porq);
    SWRITE("\n");

    OWRITE(result);
    SWRITE("\n");
  }

  return result;
}


// Special Linear Combination Query
// Linear Combination Satisfying Sign Condition Query
// Question: if ssf_init = GTOP: for which x can P + x*Q >= 0 be deduced?
//           if ssf_init = LTOP: for which x can P + x*Q <= 0 be deduced?
//
// Returns: res_flag, sL, sR, c_strict, c_equal where
//          [a,b] s.t. x \in [a,b] ==> P + x*Q >=/<= 0 and
//          res_flag: is FALSE if [a,b] empty and TRUE otherwise
//          sL : sgn(a)
//          sR : sgn(b)
//          c_strict : the strict mask from above
//          c_equal  : the equal mask from above
int specialLinCombQ(IntPolyRef P, IntPolyRef Q, VarKeyedMap<int> &varSign, int ssf_init,
		    int &sL, int &sR, int &c_strict, int &c_equal,VarContext *pM)
{
  VarSet used;
  tmp_pM = pM;
  Word res = f(P->slevel,P->sP,P->svars.rbegin(),
	       Q->slevel,Q->sP,Q->svars.rbegin(),
	       0,ssf_init,varSign,LIST3(LIST2(-1,1),iq_none,iq_all),P->svars,Q->svars,used);
  if (res == FALSE) return FALSE;
  Word I;

  if (false) // Disgustingly necessary debugging section!
  {
    cout << "P := "; P->write(*pM); cout << endl;
    cout << "Q := "; Q->write(*pM); cout << endl;
    OWRITE(res); SWRITE(": ");
    VarSet PQV = P->svars | Q->svars;
    for(VarSet::iterator vitr = PQV.begin(); vitr != PQV.end(); ++vitr)
    { cout << pM->getName(*vitr) << " " << relopString(varSign[*vitr]) << " 0, "; }
    cout << endl;
  }
  
  FIRST3(res,&I,&c_strict,&c_equal);
  sL = ERNSIGN(FIRST(I));
  sR = ERNSIGN(SECOND(I));
  return TRUE;
}
}//end namespace tarski
