#ifndef _OC_MEMO_POLY_MANAGER_
#define _OC_MEMO_POLY_MANAGER_

#include <algorithm>
#include <string>
#include <iostream>
#include <cstdio>
#include <sstream>
#include <vector>
#include <map>
#include <cctype>
#include "../GC_System/GC_System.h"
#include "../algparse/algparse.h"
#include "../formula/formmanip.h"
#include "../formula/formula.h"
#include "../formula/normalize.h"
#include "../formula/linearSubs.h"
#include "../search/qfr.h"
#include "../poly/tracker.h"
#include "../poly/md5digest.h"

namespace tarski {
/*
MemoizedPolyManager 
Extends PolyManager by offerring memoized computation of 
discriminant factors and resultant factors.
 */

class MemoizedPolyManager : public PolyManager
{
private:
  class MPMDcompare 
  { 
  public:
    bool operator()(IntPolyRef p, IntPolyRef q) const { return &(*p) < &(*q); }
  };
  VarKeyedMap< std::map<IntPolyRef,FactRef,MPMDcompare>  > MDisc;
  VarKeyedMap< std::map<IntPolyRef,FactRef,MPMDcompare>  > MLdcf;
  class MPMRcompare 
  { 
  public:
    static inline bool lt(const pair<IntPolyRef,IntPolyRef> &A, const pair<IntPolyRef,IntPolyRef> &B)
    {
      return &(*(A.first)) < &(*(B.first)) 
	|| 
        (&(*(A.first)) == &(*(B.first)) && &(*(A.second)) < &(*(B.second)));

    }
    bool operator()(const pair<IntPolyRef,IntPolyRef> &A, const pair<IntPolyRef,IntPolyRef> &B) const
    { 
      return lt(A,B);
    }
  };
  VarKeyedMap< std::map<pair<IntPolyRef,IntPolyRef>,FactRef,MPMRcompare>  > MRes;

  class Triple
  {
  public:
    GCWord s, c, L;
    Triple() { s = NIL; c = NIL; L = NIL; }
    bool isUninitialized() { return s == NIL; }
    Triple(Word s, Word c, Word L) { this->s = s; this->c = c; this->L = L; }
  };

  std::map<Tracker::Hash,Triple> factorTracker;
  std::map<Tracker::Hash,pair<IntPolyRef,GCWord> > evalTracker;

public:
  MemoizedPolyManager() : 
  MDisc(std::map<IntPolyRef,FactRef,MPMDcompare>()), 
  MLdcf(std::map<IntPolyRef,FactRef,MPMDcompare>()), 
    MRes(std::map<pair<IntPolyRef,IntPolyRef>,FactRef,MPMRcompare>()) { } 

  void factorHelper(IntPolyRef p, Word &s, Word &c, Word &L)
  {
    Tracker::Hash H;
    md5_digest_saclibObj(LIST2(p->slevel,p->sP),H.A);
    Triple &T = factorTracker[H];
    if (!T.isUninitialized()) { s = T.s; c = T.c; L = T.L; return; }
    PolyManager::factorHelper(p,s,c,L);
    T.s = s; T.c = c; T.L = L;
    return;
  }

  
  IntPolyRef evalAtRationalPointMakePrimHelper(const IntPolyRef &p, Word Q, Word perm1, Word perm2, VarSet Vremain, GCWord &content)
  {    
    Tracker::Hash H;
    md5_digest_saclibObj(LIST5(Q,perm1,perm2,p->slevel,p->sP),H.A);
    pair<IntPolyRef,GCWord> &P = evalTracker[H];
    if (P.first.is_null())
      P.first = PolyManager::evalAtRationalPointMakePrimHelper(p,Q,perm1,perm2,Vremain,P.second);
    content = P.second;
    return P.first;
  }
  

  /************************************************************
   These functions assume their arguments are canonical copies.
   If not, you may recompute something!
   ************************************************************/
  virtual FactRef ldcfFactors(IntPolyRef p, VarSet x)
  {    
    std::map<IntPolyRef,FactRef,MPMDcompare> &M = MLdcf[x];
    std::map<IntPolyRef,FactRef,MPMDcompare>::iterator itr = M.find(&(*p));
    FactRef D;
    if (itr != M.end()) { D = itr->second; }
    else
    {
      D = PolyManager::ldcfFactors(p,x);
      M[&(*p)] = D;
    }
    return D;
  }


  virtual FactRef discriminantFactors(IntPolyRef p, VarSet x)
  {
    std::map<IntPolyRef,FactRef,MPMDcompare> &M = MDisc[x];
    std::map<IntPolyRef,FactRef,MPMDcompare>::iterator itr = M.find(&(*p));
    FactRef D;
    if (itr != M.end()) { D = itr->second; }
    else
    {
      D = PolyManager::discriminantFactors(p,x);
      M[&(*p)] = D;
    }
    return D;
  }

  void swap(IntPolyRef& p, IntPolyRef& q) {
    IntPolyRef tmp = p;
    p = q;
    q = tmp;
  }

  virtual FactRef resultantFactors(IntPolyRef p, IntPolyRef q, VarSet x)
  {
    FactRef R;
    Word sm = 1;
    if (&(*q) < &(*p))
    {
      if (p->degree(x) % 2 && q->degree(x) % 2) { sm = -1; }
      swap(p,q);
    } 
    pair<IntPolyRef,IntPolyRef> pp(p,q);
    std::map<pair<IntPolyRef,IntPolyRef>,FactRef,MPMRcompare> &M = MRes[x];
    std::map<pair<IntPolyRef,IntPolyRef>,FactRef,MPMRcompare>::iterator itr = M.find(pp);
    if (itr != M.end()) { R = itr->second; }
    else
    {
      R = PolyManager::resultantFactors(p,q,x);
      M[pp] = R;
    }
    if (sm == -1)
    {
      R = new FactObj(*R);
      R->content = IPROD(R->content,-1);
    }
    return R;
  }

};
}//end namespace tarski
#endif
