/**
 * @file realroots.h
 * This file defines the basic classes representing real algebraic numbers.
 * The representation is optimized for computing open CADs, and is not
 * necessarily the best representation for arbitrary computations with
 * real algebraic numbers.
 */

#ifndef _OC_REAL_ROOTS_
#define _OC_REAL_ROOTS_

#include <algorithm>
#include <string>
#include <iostream>
#include <cstdio>
#include <sstream>
#include <vector>
#include <map>
#include <queue>
#include <cctype>
#include <limits>
#include "../poly/sacMod.h"
#include "../poly/factor.h"
#include "../tarski.h"

namespace tarski {
  
class RealRootException : public TarskiException
{
public:
  RealRootException(const std::string & msg) : TarskiException(msg) { }
};

class RealAlgNumObj; typedef GC_Hand<RealAlgNumObj> RealAlgNumRef;
class NegInftyObj; typedef GC_Hand<NegInftyObj> NegInftyRef;
class PosInftyObj; typedef GC_Hand<PosInftyObj> PosInftyRef;
class RealRootIUPObj; typedef GC_Hand<RealRootIUPObj> RealRootIUPRef;

class RealAlgNumObj : public GC_Obj
{
public:
  /** A synonym for compareToRobust */
  virtual int compareTo(RealAlgNumRef y) = 0; // returns -1 if less than y, 0 if exal, 1 if greater

  /** A faster comparison that requires that it is known a priori that the two algebraic numbers are distinct. */
  virtual int compareToBrittle(RealAlgNumRef y) { return compareTo(y); }

  /** A slower comparison that correctly handles the case in which the two algebraic numbers are the same.
   * This method functions properly even if the defining polynomials for the two algebraic numbers are not identical. 
   */
  virtual int compareToRobust(RealAlgNumRef y) { return compareTo(y); }

  virtual std::string toStr() = 0;
  virtual int compareToNegInfty() { return 1; } 
  virtual int compareToPosInfty() { return -1; } 
  virtual Word LBRNPointAbove() { return NIL; }
  virtual Word LBRNPointBelow() { return NIL; }
  virtual bool isFinite() { return false; }
  virtual void refineTo(int logOfWidth) { }
  virtual double approximate(int dir) = 0; // -1 means round down, +1 means round up

  // -1 if this is less than x, 0 if this is equal to x, +1 if this is greater than x
  virtual int compareToLBRN(Word x) { return -99999; }

  // If returns non-NIL, then the real-algebraic number is in fact rational, and
  // that rational in SACLIB format is returned.  Otherwise, nothing is known.
  virtual Word tryToGetExactRationalValue() { return NIL; }
};
template<class T>
T* asa(const RealAlgNumRef& p) { return dynamic_cast<T*>(&(*p)); }

class NegInftyObj : public RealAlgNumObj
{
public:
  int compareTo(RealAlgNumRef y) { return -y->compareToNegInfty(); }
  int compareToNegInfty() { return 0; }
  std::string toStr() { return "-infty"; }
  double approximate(int dir) { return -std::numeric_limits<double>::infinity(); }
  virtual int compareToLBRN(Word x) { return -1; }
};

class PosInftyObj : public RealAlgNumObj
{
public:
  int compareTo(RealAlgNumRef y) { return -y->compareToPosInfty(); }
  int compareToPosInfty() { return 0; }
  std::string toStr() { return "+infty"; }
  double approximate(int dir) { return std::numeric_limits<double>::infinity(); }
  virtual int compareToLBRN(Word x) { return +1; }
};

class RealRootIUPObj : public RealAlgNumObj
{
private:
  GCWord sP; // saclib integral univariate polynomial, must be squarefree (do I want irreducible?)
  int numRoots; // number of roots of sP (-1 indicates "unknown")
  int j; // index of the root
  GCWord I;  // binary rational logarithmic interval for the jth distinct real root of sP
  int t;     // the trend of sP in I
public:  
  RealRootIUPObj(Word _sP, int _j, Word _I, int _t, int _numRoots) 
  { 
    Word s, c, pp;
    IPSCPP(1,_sP,&s,&c,&pp);
    sP = pp; j = _j; I = _I; t = (s >= 0 ? _t : -_t); numRoots = _numRoots;
    if (EQUAL(sP,LIST4(1,1,0,-3)) && j == 1 && t == -1) {  std::cerr << "bad init" <<std::endl;  }
  }
  int rootIndex() { return j; }
  int numberOfRoots() { return numRoots; }

  // -1 if this is less than x, 0 if this is equal to x, +1 if this is greater than x
  virtual int compareToLBRN(Word x);

  virtual int compareTo(RealAlgNumRef y) { return compareToRobust(y); }
  virtual int compareToBrittle(RealAlgNumRef y);
  virtual int compareToRobust(RealAlgNumRef y);
  virtual bool separate(RealAlgNumRef y); // refine this and/or y until their isolating interval endpoints are separated
                                          // return false if this is not possible.
  virtual std::string toStr();
  virtual Word saclibPoly() { return sP; }
  virtual Word LBRNPointAbove() { return I == 0 ? 0 : SECOND(I); }
  virtual Word LBRNPointBelow() { return I == 0 ? 0 : FIRST(I); }
  virtual bool isFinite() { return true; }
  virtual bool singlePointInterval() { return I == 0 || EQUAL(FIRST(I),SECOND(I)); }
  virtual Word isolatingInterval() { return I; }
  virtual void refineTo(int logOfWidth);
  virtual double approximate(int dir);
  inline int getTrend() const { return t; }
  virtual Word tryToGetExactRationalValue();
};

double approximateLBRN(Word x, int dir);

inline RealAlgNumRef max(RealAlgNumRef a, RealAlgNumRef b) { return a->compareToRobust(b) < 0 ? b : a; }
inline RealAlgNumRef min(RealAlgNumRef a, RealAlgNumRef b) { return b->compareToRobust(a) < 0 ? b : a; }

extern RealAlgNumRef integerToRealAlgNum(Word I); // I a saclib integer
extern RealAlgNumRef rationalToRealAlgNum(Word R); // R a saclib rational number

extern std::vector<RealRootIUPRef> RealRootIsolateSquarefree(Word p); // p must be a non-zero, squarefree integral univariate saclib poly
inline std::vector<RealRootIUPRef> RealRootIsolateSquarefree(IntPolyRef p) // p must be a non-zero, squarefree integral univariate poly
{
  return RealRootIsolateSquarefree(p->sP);
}
inline std::vector<RealRootIUPRef> RealRootIsolateRobust(IntPolyRef p) // p must be a non-zero univariate poly
{
  Word A = p->sP;
  if (p->isZero()) { throw RealRootException("cannot isolate roots of the zero polynomial"); }
  if (p->isConstant()) { return std::vector<RealRootIUPRef>(); } 
  if (p->numVars() > 1) {  throw RealRootException("root isolation requires a univariate polynomial"); }
  Word Ap = IPDMV(1,A), GCD, C, Cp;
  IPGCDC(1,A,Ap,&GCD,&C,&Cp);
  return RealRootIsolateSquarefree(C);
}

extern std::vector<RealRootIUPRef> RealRootIsolate(FactRef F);

// returns a rational point in the interval (X,Y): I.e. must be an open interval!
// requires X < Y, returns NIL if it discovers otherwise,
// but it might not discover otherwise!
extern Word rationalPointInInterval(RealAlgNumRef X,RealAlgNumRef Y);


// returns true if a and b represent the same number
// the big deal is that if needed, this will compute the GCD of defining polys for
// a and b in order to determine whether a and b really are the same.
bool equal(RealAlgNumRef a, RealAlgNumRef b);

// RealRoot1Ext is for representing roots over a single extension
class RealRoot1ExtObj; typedef GC_Hand<RealRoot1ExtObj> RealRoot1ExtRef;
class RealRoot1ExtObj : public RealAlgNumObj
{
 private:
  RealRootIUPRef alpha; // should probably require that the polynomial defining alpha is irreducible!
  GCWord A; // an element of Q(alpha)[x] that this number is a root of
  GCWord I; // binary rational logarithmic interval for the jth distinct real root of A
  int j; // index of the root
  
 public:
  RealRoot1ExtObj(RealRootIUPRef alpha, Word A, Word I, int j) { this->alpha = alpha; this->A = A; this->I = I; this->j = j; }
  int compareTo(RealAlgNumRef y) { throw RealRootException("In RealRoot1ExtObj: compareTo not yet implemented."); } 
  int compareToBrittle(RealAlgNumRef y)  { throw RealRootException("In RealRoot1ExtObj: compareToBrittle not yet implemented."); }
  int compareToRobust(RealAlgNumRef y) { throw RealRootException("In RealRoot1ExtObj: compareToRobust not yet implemented."); }
  std::string toStr()
  {
    std::ostringstream sout;
    PushOutputContext(sout);
    SWRITE("[");
    AFUPWRITE(A,LFS("x"),LFS("a"));
    SWRITE(",");
    IWRITE(j);
    SWRITE(",");
    LBRIWRITE(I);
    SWRITE(",");
    SWRITE("trend=?");
    SWRITE("]");    
    PopOutputContext();
    return sout.str();
  }
  Word LBRNPointAbove() { return I == 0 ? 0 : SECOND(I); }
  Word LBRNPointBelow() { return I == 0 ? 0 : FIRST(I); }
  bool isFinite() { return true; }
  void refineTo(int logOfWidth);
  // -1 means round down, +1 means round up
  double approximate(int dir);
  // -1 if this is less than x, 0 if this is equal to x, +1 if this is greater than x
  int compareToLBRN(Word x) {  throw RealRootException("In RealRoot1ExtObj: compareToLBRN not yet implemented."); }

  virtual bool singlePointInterval() { return I == 0 || EQUAL(FIRST(I),SECOND(I)); }
};

// p must be a non-zero bivariate poly
std::vector<RealRoot1ExtRef> RealRootIsolate(IntPolyRef p, int tmpOrderFlag, RealRootIUPRef alpha);
}//end namespace tarski  
#endif
