#ifndef _OC_VAR_ORDER
#define _OC_VAR_ORDER

#include <algorithm>
#include <string>
#include <iostream>
#include <cstdio>
#include <sstream>
#include <vector>
#include <map>
#include <queue>
#include <cctype>
#include "../GC_System/GC_System.h"
#include "../algparse/algparse.h"
#include "../formula/formmanip.h"
#include "../formula/formula.h"

namespace tarski {
  
class VarOrderObj; typedef GC_Hand<VarOrderObj> VarOrderRef;
class VarOrderObj : public GC_Obj
{
private:
  std::vector<VarSet> V;
  PolyManager* ptrPM;
public:
  VarOrderObj(PolyManager* p) { ptrPM = p; V.push_back(VarSet());}
  int size() { return V.size()-1; }
  void push_back(VarSet x) { V.push_back(x); }
  void push_back(std::string s) { V.push_back(ptrPM->addVar(s)); }
  VarSet& operator[](unsigned int i) { return V[i]; }
  VarSet& get(unsigned int i) { return V[i]; }
  PolyManager* getPolyManager() const { return ptrPM; }
  void read(std::istream &in);
  void write();
  int level(IntPolyRef p);
  VarKeyedMap<GCWord> makeValueMap(Word alpha, int r);

  /*
    p - an IntPoly
    L - a saclib list of at least r rational numbers
    r - a non-negative number
    evaluates p at x_i <-- L_i using this variable order for x_1,...,x_r
    and returns a primitive integral polynomials similar to p(L_1,...,L_r,x_{r+1},...,x_n)
    where the similarity constant is positive.
   */
  IntPolyRef partialEval(IntPolyRef p, Word L, int r);

  /*
    p - an IntPoly in variables x_1,...,x_r
    L - a saclib list of at least r rational numbers
    r - a non-negative number
    evaluates p at x_i <-- L_i using this variable order for x_1,...,x_{r-1}
    and increments n_less, n_equal and n_greater amounts equal to the number of roots of
    p(L_1,...,L_{r-1},x_r) that are less/equal/greater L_r and returns true,
    unless p(L_1,...,L_{r-1},x_r) is zero, in which case false is returned and
    n_less, n_equal and n_greater are left unchanged.
   */
  bool partialEvalRoot(IntPolyRef p, Word L, int r, int& n_less, int& n_equal, int& n_greater);

  bool isNullified(IntPolyRef p, Word L, int r) { return partialEval(p,L,r-1)->isZero(); }

};


/*****************************************************************
 * E V A L U A T I O N
 *****************************************************************/

// alpha is a point of dimension at least equal to the maximum level of any poly in C
int evalStrictConjunctionAtPoint(VarOrderRef X, GCWord alpha, TAndRef C);

// counts number of UNSAT conjuncts
int countUNSATStrictConjunctionAtPoint(VarOrderRef X, GCWord alpha, TAndRef C);

/*****************************************************************
 * C O M P A R I S O N
 *****************************************************************/
class BasicPolyCompare
{
 private:
  VarOrderRef X;
  static int SACPOLYSIZE(Word A)
  {
    if (A == 0) return 1;
    if (ISATOM(A)) return DLOG2(A);
    if (A == NIL) return 1;
    Word d1, c1, Ap;
    ADV2(A,&d1,&c1,&Ap);
    return d1 + SACPOLYSIZE(c1) + (Ap == NIL ? 0 : SACPOLYSIZE(Ap));
  }
 public:
  BasicPolyCompare(VarOrderRef X) { this->X = X; }
  virtual bool lt(IntPolyRef a, IntPolyRef b)
  {
    // if exactly one is tdegree = 1, report that one as "lt"
    int a_td = a->totalDegree();
    int b_td = b->totalDegree();
    if (a_td <= 1 && b_td > a_td) return true;
    if (b_td <= 1 && a_td > b_td) return false;

    // otherwise, make the decision based on level, if different
    int a_lev = X->level(a);
    int b_lev = X->level(b);
    if (a_lev < b_lev) return true;
    if (b_lev < a_lev) return false;

    // otherwise, make the decision based on the "size"
    int a_size = SACPOLYSIZE(a->sP);
    int b_size = SACPOLYSIZE(b->sP);
    return a_size < b_size;
  }
};

}//end namespace tarski
#endif

