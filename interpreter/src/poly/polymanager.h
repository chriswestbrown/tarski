#ifndef _CWBPOLYMANAGER_
#define _CWBPOLYMANAGER_

#include "poly.h"
#include "sacMod.h"
#include <sstream>
#include <vector>

namespace tarski {
  class FactObj; typedef GC_Hand<FactObj> FactRef;
class PolyManagerStatsObj : public GC_Obj
{
 public:
  unsigned int time_sac_factor, time_sac_resultant;
  PolyManagerStatsObj() { time_sac_factor = time_sac_resultant = 0; }
  void print(ostream &out)
  {
    cout << "Time in IPFAC = " << time_sac_factor << "ms" << endl;
    cout << "Time in IPRESPRS = " << time_sac_resultant << "ms" << endl;
  }
};
typedef GC_Hand<PolyManagerStatsObj> PolyManagerStatsRef;

/********************************************************************************
 * A PolyManager keeps "cannonical copies" of irreducible, primitive, positive
 * integer polynomials.
 ********************************************************************************/ 
class PolyManager : public VarContext
{
public:
  class PolyLT { public: int operator()(IntPolyRef a, IntPolyRef b) { return IntPolyObj::ipcmp(a,b); } };
private:
  set<IntPolyRef,PolyLT> polyset;
  PolyManagerStatsRef _stats;
public:
  PolyManager() : polyset(PolyLT()), _stats(new PolyManagerStatsObj) { } 
  IntPolyRef getCannonicalCopy(IntPolyRef p);

  // NOTE1: These operations DO NOT produce cannonical copies!
  // NOTE2: In the future I may be assuming that arguments for the ops ARE cannonical copies.
  IntPolyRef resultant(IntPolyRef p, IntPolyRef q, VarSet x);
  IntPolyRef discriminant(IntPolyRef p, VarSet x);
  IntPolyRef prem(IntPolyRef p, IntPolyRef q, VarSet x);
  IntPolyRef ldcf(IntPolyRef p, VarSet x);
  IntPolyRef trcf(IntPolyRef p, VarSet x);
  IntPolyRef coef(IntPolyRef p, VarSet x, int deg);
  IntPolyRef prod(IntPolyRef p, IntPolyRef q);
  IntPolyRef sum(IntPolyRef p, IntPolyRef q);
  IntPolyRef dif(IntPolyRef p, IntPolyRef q);
  IntPolyRef neg(IntPolyRef p);
  void nonZeroCoefficients(IntPolyRef p, VarSet Z, vector<IntPolyRef> &V); 

  /*
    polynomial A after applying assignment xi <-- q xj, where q is a non-zero rational number.
    So, if B is the returned polynomial and c the content, A after the evaluation equals 
    NOTE: there is no guarantee that A has xi and/or xj as variables!
   */
  IntPolyRef evalAtRationalMultipleOfVariable(IntPolyRef A, VarSet xi, Word q, VarSet xj, GCWord &content);

  /* This in particular we shoud *not* make any assumptions about being a canonical copy */
  virtual void factorHelper(IntPolyRef p, Word &s, Word &c, Word &L);

  /* This in particular we shoud *not* make any assumptions about being a canonical copy */
  virtual IntPolyRef evalAtRationalPointMakePrim(const IntPolyRef &p, VarKeyedMap<GCWord> &value, GCWord &content);
  virtual IntPolyRef evalAtRationalPointMakePrimHelper(const IntPolyRef &p, Word Q, 
						       Word perm1, Word perm2, VarSet Vremain, GCWord &content); // basically for internal use

  virtual FactRef resultantFactors(IntPolyRef p, IntPolyRef q, VarSet x);
  virtual FactRef discriminantFactors(IntPolyRef p, VarSet x);
  virtual FactRef ldcfFactors(IntPolyRef p, VarSet x);
  
  // Special S-polynomial
  // Input: Polynomials p and q, variable x. deg_x(p) = deg_x(q) > 1.
  // Output: NULL or polynomial (not a cannonical copy!)
  IntPolyRef specialSpoly(IntPolyRef p, IntPolyRef q, VarSet x);
  vector<IntPolyRef> standardSpoly(IntPolyRef p, IntPolyRef q, VarSet x);
  
  string polyToStr(IntPolyRef p) 
  { ostringstream sout; PushOutputContext(sout); 
    p->write(*this); PopOutputContext(); return sout.str(); }

  PolyManagerStatsRef getStats() { return _stats; }

  void printContents()
  {
    for(set<IntPolyRef,PolyLT>::iterator itr = polyset.begin(); itr != polyset.end(); ++itr)
    {
      SWRITE("## ");
      (*itr)->write(*this);
      SWRITE(" : ");
      (*itr)->svars.write();
      SWRITE("\n");
    }
  }
};
}//end namespace tarski
#endif
