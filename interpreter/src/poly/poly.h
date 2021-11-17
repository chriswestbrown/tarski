#ifndef _CWBPOLY_
#define _CWBPOLY_

#include "gcword.h"
#include "variable.h"
#include "../GC_System/GC_System.h"
#include "sacpolyutils.h"
#include <set>
#include <vector>
#include <list>
#include <forward_list>


namespace tarski {

// This is a base class that serves as a mapping from Tarski variables back
// to whatever variable names we want in the SMTLIB output.  It's too bad it has to 
class WSIRevmapper
{
public:
  virtual const string& revMap(const string& var) { return var; }
};


/********************************************************************************
 * An IntPolyObj represents a polynomial as
 * - svars: the (ordered) set of variables in which the polynomial has positive degree.
 * -    sP: the saclib representation of the polynomial in the variable ordering
 *          given by svars.
 * Additionally, the redundent value "slevel", the number of variables, is stored.
 * Note that polynomials are always in "compactified" form, meaning that each 
 * variable in svars actually appears to positive degree in the polynomial.
 ********************************************************************************/
class IntPolyObj;
typedef GC_Hand<IntPolyObj> IntPolyRef;
  
class IntPolyObj : public GC_Obj
{
  //private:
 public:
  VarSet svars; // svars.begin() gives you an iterator for the innermost variable in sP.
  GCWord sP;
  Word slevel;

public:
  IntPolyObj() { sP = 0; slevel = 0; }
  IntPolyObj(short int c) { sP = c; slevel = 0; }
  IntPolyObj(VarSet x) { sP = LIST2(1,1); slevel = 1; svars = x; } // x is assumed to be a single var
  IntPolyObj(Word r, Word A, VarSet S) { compactify(r,A,S); }
  bool isConstant() const { return svars == 0; }
  VarSet isVariable() const { return slevel == 1 && EQUAL(sP,LIST2(1,1)) ? svars : 0; }
  bool isVar() const { return slevel == 1 && EQUAL(sP, LIST2(1,1)) ? true : false; }
  int signIfConstant() const { return isConstant() ? ISIGNF(sP) : 0; }
  Word valueIfConstant() const { return sP; }
  bool isZero() const { return sP == 0; }
  const VarSet& getVars() const { return svars; }
  int numVars() { return slevel; }
  int degree(VarSet S) const;
  int totalDegree();
  bool equal(IntPolyRef p) 
  { 
    return (this == &(*p)) || (getVars() == p->getVars() && EQUAL(sP,p->sP)); 
  }
  //NOTE: Assumes canonical copies! Returns the difference between the pointers
  int compare(IntPolyRef p);
  VarSet linearlyOccurringVariables();
  VarSet variableUniqueDegrees();
  int gcdOfVariableExponents(VarSet x);
  IntPolyRef reduceDegreeInX(VarSet x, int d);
  int signDeduce(const VarKeyedMap<int> &varSign); // varSign[x] = sigma means that "x sigma 0" holds.
  void write(VarContext &C); // Write to the current Saclib Output Context
  int read(VarContext &C, VarSet S); // Read from current saclib input context

  // r : level
  // A : saclib poly
  // V : saclib variable set with respect to which A is an r-level poly
  // C : variable context
  static IntPolyRef saclibToNonCanonical(Word r, Word A, Word V, VarContext &C);
  
  void writeSMTLIB(VarContext &C, ostream& out, WSIRevmapper& vrm);
  void writeMAPLE(VarContext &C, ostream& out);
  void writeMATHEMATICA(VarContext &C, ostream& out);

  // A->integerProduct(a)
  // returns a new IntPolyRef for a polynomial a*A.
  IntPolyRef integerProduct(Word a) const;
  
  // M maps VarSet->VarSet, such that, when restricted to domain this.getVars(), the mapping is 1-1
  // returns a new IntPolyRef that represents p(M(x1),M(x2),...,M(xr))
  // NOTE: reducibility and primitiveness are preserved, but the property of being "positive" may
  //       not be!
  IntPolyRef renameVars(VarKeyedMap<VarSet> &M);

  // makes this polynomial positive, sets sn to 1 if the poly is unchanged, -1 if negated.
  IntPolyRef makePositive(int& sn);

  // Makes the same sign deduction as signDeduce, but returns a minimal set of variables "used"
  // whose sign conditions are actually used to make the deduction.  Note, this is *minimal*
  // not *minimum* sized!
  int signDeduceMinimalAssumptions(const VarKeyedMap<int> &varSign, VarSet &used);

  // Read Rational polynomial, Set to Similar integer polynomial.  Similarity constant is positive!
  int readRatSetSim(VarContext &C, VarSet S); // Read from current saclib input context

  // Linear root further from zero
  // Assumes "this" and q are both linear, univariate, containing the same variable.
  // both roots have the same side.  Return true if "this" has root further
  // from zero than q.
  bool linearRootFurtherFromZero(IntPolyRef q);

  // Evaluate p at x=z and return the primitive similar integral polynomial with positive content
  IntPolyRef evalAtRationalMakePrim(VarSet x, Word z);



  /** Evaluate at rational point, return the positive primitive similar integral polynomial
   * values is a map mapping to each variable either a rational number or NIL
   * if "this" is f(x1,...,xn), where x1,...,xn is in the order given by getVars()
   * and values[xi] = pi, then the output is the positive primitive similar integral polynomial with content
   * to polynomial f evaluated at xi = pi if pi != NILL, and xi remains unevaluated otherwise.
   * I.e. if "this" is A, the returned poly is B and content is c, we have A, partially evaluated at
   * values is equal to c*B, where c is a Saclib rational number, and B is a integer polynomial.
   */
  IntPolyRef evalAtRationalPointMakePrim(VarKeyedMap<GCWord> &values, GCWord &content);


  IntPolyRef derivative(VarSet x);

  GCWord getSaclibPoly() const { return sP; }
  int getLevel() const { return slevel; }
  
  void varStats(VarKeyedMap< vector<int> > &M); //updates M,  M[x] = [maxdeg of x, max tot degree of term in x, num terms with x]

  int sizeStats(int &numTerms, int &sotd, int &maxCeffBitlength) const;

  Word expand(VarSet newV) const { return expand(slevel,sP,getVars(),newV); }

private:
  void compactify(Word r, Word A, VarSet S);
  Word expand(Word r, Word A, VarSet oldV, VarSet newV) const ; // oldV contained in newV
  int indexOfVar(VarSet x);

public:
  friend class PolyManager;
  friend class FactObj;


  //  static IntPolyRef sum(IntPolyRef p, IntPolyRef q);
  static bool ipcmp(IntPolyRef a, IntPolyRef b); // this is a < operator for IntPolyObjects, i.e. true or false
  static int ipcompare(IntPolyRef a, IntPolyRef b); // this is a proper compare, i.e. -1,0,+1
};

int specialLinCombQ(IntPolyRef P, IntPolyRef Q, VarKeyedMap<int> &varSign, int ssf_init, 
		    int&,int&, int&,int&,VarContext *);

// Given poly sP of level slevel, on varset S, with variable sign assumptions
// given in L, and deduced sign ded, returns a minimal sized VarSet S, s.t. the
// assumptions on elts of S assume to make the same sign-deduction.
VarSet minimalReqdForSignDeduce(Word slevel, Word sP, Word L, VarSet S, int ded);


}//end namespace tarski
#endif
