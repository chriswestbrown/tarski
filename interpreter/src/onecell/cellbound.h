#ifndef _OC_CELL_BOUND_
#define _OC_CELL_BOUND_

#include "../realroots/realroots.h"
#include "varorder.h"

namespace tarski {

class CellBoundObj; typedef GC_Hand<CellBoundObj> CellBoundRef;

class CellBoundObj : public GC_Obj
{
private:
  CellBoundObj(Word _alphaAsRN, IntPolyRef _l, IntPolyRef _u,
	       RealAlgNumRef _a, RealAlgNumRef _b, RealAlgNumRef _alpha
	       ) 
  {
    alphaAsRN = _alphaAsRN;
    l = _l;
    u = _u;
    a = _a;
    b = _b;
    alpha = _alpha;
  }
public:
  GCWord alphaAsRN;
  IntPolyRef l, u;
  RealAlgNumRef a, b, alpha;
  CellBoundObj(Word rat);
  CellBoundObj(Word rat, Word pb, IntPolyRef p);

  RealAlgNumRef getBoundAbovePoint(int dir, int k, Word beta, VarOrderRef X)
  {
    if (dir == -1) // lower bound!
    {      
      if (a->compareToNegInfty() == 0) return a;
      RealRootIUPRef zlow = asa<RealRootIUPObj>(a);
      int j = zlow->rootIndex();
      IntPolyRef lbp = X->partialEval(l,beta,k-1);
      std::vector<RealRootIUPRef> VL = RealRootIsolateSquarefree(lbp->getSaclibPoly());
      if (VL.size() <= j-1)
	throw TarskiException("In getBoundAbovePoint, too few roots of lower bound polynomial!");
      RealAlgNumRef intervalLowerBound = VL[j-1];
      return intervalLowerBound;
    }
    else // upper bound!
    {
      if (b->compareToPosInfty() == 0) return b;
      RealRootIUPRef zhigh = asa<RealRootIUPObj>(b);
      int j = zhigh->rootIndex();
      IntPolyRef ubp = X->partialEval(u,beta,k-1);
      std::vector<RealRootIUPRef> VL = RealRootIsolateSquarefree(ubp->getSaclibPoly());
      if (VL.size() <= j-1) 
	throw TarskiException("In getBoundAbovePoint, too few roots of upper bound polynomial!");
      RealAlgNumRef intervalUpperBound = VL[j-1];
      return intervalUpperBound;
    }
  }

  /*
    res <- compareWithBound(dir,k,beta,betak,X)

    dir : -1 for lower bound, +1 for upper bound

    res : +1 if betak is above the chosen boundary, -1 if betak is below the chosen boundary,
          and 0 if betak is on the chosen boundary.

  */
  int compareWithBound(int dir, int k, Word beta, RealAlgNumRef betak, VarOrderRef X)
  {
    if (dir == -1) // lower bound!
    {      
      if (a->compareToNegInfty() == 0) return 1;
      RealRootIUPRef zlow = asa<RealRootIUPObj>(a);
      int j = zlow->rootIndex();
      IntPolyRef lbp = X->partialEval(l,beta,k-1);
      std::vector<RealRootIUPRef> VL = RealRootIsolateSquarefree(lbp->getSaclibPoly());
      if (VL.size() <= j-1)
	throw TarskiException("In compareWithBound, too few roots of lower bound polynomial!");
      RealAlgNumRef intervalLowerBound = VL[j-1];
      int tst = intervalLowerBound->compareToRobust(betak);
      if (tst > 0) return -1;
      if (tst < 0) return +1;
      return 0;
    }
    else // upper bound!
    {
      if (b->compareToPosInfty() == 0) return -1;
      RealRootIUPRef zhigh = asa<RealRootIUPObj>(b);
      int j = zhigh->rootIndex();
      IntPolyRef ubp = X->partialEval(u,beta,k-1);
      std::vector<RealRootIUPRef> VL = RealRootIsolateSquarefree(ubp->getSaclibPoly());
      if (VL.size() <= j-1) 
	throw TarskiException("In compareWithBound, too few roots of upper bound polynomial!");
      RealAlgNumRef intervalUpperBound = VL[j-1];
      int tst = intervalUpperBound->compareToRobust(betak);
      if (tst < 0) return +1;
      if (tst > 0) return -1;
      return 0;
    }
  }

  // Returns true if the lower bounds of "this" and B are identical
  bool identicalLower(CellBoundRef B) 
  { 
    bool f1 = a->isFinite(), f2 = B->a->isFinite();
    if (!f1 || !f2)
      return !f1 && !f2;
    if (!l.identical(B->l))
      return false;
    RealRootIUPRef r1 = asa<RealRootIUPObj>(a), r2 = asa<RealRootIUPObj>(B->a);
    return r1->rootIndex() == r2->rootIndex();
  }

  // Returns true if the upper bounds of "this" and B are identical
  bool identicalUpper(CellBoundRef B) 
  { 
    bool f1 = b->isFinite(), f2 = B->b->isFinite();
    if (!f1 || !f2)
      return !f1 && !f2;
    if (!u.identical(B->u))
      return false;
    RealRootIUPRef r1 = asa<RealRootIUPObj>(b), r2 = asa<RealRootIUPObj>(B->b);
    return r1->rootIndex() == r2->rootIndex();
  }

  /*
    res <- locateRationalPoint(k,beta,X)
    Inputs:
    k    - the level of this bound
    beta - a rational point of level at least k, it is required that
           (beta_1,...,beta_{k-1}) is not a zero of the discrim or leading
           coeff of l or u, and that l & u have sufficient roots over that
           point for a & b to make sense.
    X    - VarOrder for this problem
    Outputs:
    res  - (sgn,bflag), where sgn is -1,0,+1 as beta_k is below the bound 
           over (beta_1,...,beta_{k-1}), inside the bound or above the bound; 
           and bflag is 1 if (beta_1,...,beta_{k}) is a zero of the lower/upper
           boundary defining section (in which case sgn is necessarily 0).
  */
  pair<int,int> locateRationalPoint(int k, Word beta, VarOrderRef X)
  {
    RealAlgNumRef Q = rationalToRealAlgNum(LELTI(beta,k));
    
    // lower bound!
    bool lbflag = a->compareToNegInfty() == 0;
    if (!lbflag)
    {
      RealRootIUPRef zlow = asa<RealRootIUPObj>(a);
      int j = zlow->rootIndex();
      IntPolyRef lbp = X->partialEval(l,beta,k-1);
      std::vector<RealRootIUPRef> VL = RealRootIsolateSquarefree(lbp->getSaclibPoly());
      if (VL.size() <= j-1)
	throw TarskiException("In locateRationalPoint, too few roots of lower bound polynomial!");
      RealAlgNumRef intervalLowerBound = VL[j-1];
      int tst = intervalLowerBound->compareToRobust(Q);
      if (tst > 0) return make_pair(-1,0);
      if (tst == 0) return make_pair(-1,1);
    }

    // upper bound!
    bool ubflag = b->compareToPosInfty() == 0;
    if (!ubflag)
    {
      RealRootIUPRef zhigh = asa<RealRootIUPObj>(b);
      int j = zhigh->rootIndex();
      IntPolyRef ubp = X->partialEval(u,beta,k-1);
      std::vector<RealRootIUPRef> VL = RealRootIsolateSquarefree(ubp->getSaclibPoly());
      if (VL.size() <= j-1) 
	throw TarskiException("In locateRationalPoint, too few roots of upper bound polynomial!");
      RealAlgNumRef intervalUpperBound = VL[j-1];
      int tst = intervalUpperBound->compareToRobust(Q);
      if (tst < 0) return make_pair(+1,0);
      if (tst == 0) return make_pair(+1,1);
    }

    return  make_pair(0,0);
  }

  RealAlgNumRef getBoundNumber(int dir) { return dir > 0 ? b : a; }
  int getBoundIndex(int dir)
  {
    RealAlgNumRef ar = getBoundNumber(dir);
    RealRootIUPRef x = asa<RealRootIUPObj>(ar);
    return x.is_null() ? 0 : x->rootIndex();
  }
  IntPolyRef getBoundDefPoly(int dir) { return dir > 0 ? u : l; }

  // alpha - rational point of level >= k
  // k - the level of the bound that will be created
  // l - IntPolyRef of level k (or NULL)
  // j_l - positive integer such that if l is non-NIL, it has at least j_l distinct real roots
  // u - IntPolyRef of level k (or NULL)
  // j_u - positive integer such that if u is non-NIL, it has at least j_u distinct real roots
  // NOTE: if l /= NULL then alpha[k] > j_lth root of l, and
  //       if u /= NULL then alpha[k] < j_uth root of u, and
  // The CellBoundObj that gets constructed 
  CellBoundObj(VarOrderRef X, Word alpha, int k, IntPolyRef l, Word j_l, IntPolyRef u, Word j_u);

  CellBoundObj(CellBoundRef b1, CellBoundRef b2); // these *must* share the same "alpha" value!
  std::string toStr();
  std::string toStr(PolyManager* ptrPM);
  std::string toStrFormula(PolyManager* ptrPM, VarSet mainVariable);

  CellBoundRef clone()
  {
    return new CellBoundObj(alphaAsRN,l,u,
			    a,b, alpha);
  }

  void expand(CellBoundRef c, int dir) // -1 means c below this, +1 means c above this
  {                                    // NOTE: c and this must share the other boundary!
    if (dir == -1) { l = c->l; a = c->a; }
    else { u = c->u; b = c->b; }
  }

  bool equal(CellBoundRef b)
  {
    bool lna = this->l.is_null(), lnb = b->l.is_null();
    if (!( (lna && lnb) || (!lna && !lnb && this->l->equal(b->l)))) return false;
    bool una = this->u.is_null(), unb = b->u.is_null();
    if (!( (una && unb) || (!una && !unb && this->u->equal(b->u)))) return false;
    if (!lna)
    {
      RealRootIUPObj* pa = dynamic_cast<RealRootIUPObj*>(&*(this->a));
      RealRootIUPObj* pb = dynamic_cast<RealRootIUPObj*>(&*(b->a));
      if (pa->rootIndex() != pb->rootIndex())
	return false;
    }
    if (!una)
    {
      RealRootIUPObj* pa = dynamic_cast<RealRootIUPObj*>(&*(this->b));
      RealRootIUPObj* pb = dynamic_cast<RealRootIUPObj*>(&*(b->b));
      if (pa->rootIndex() != pb->rootIndex())
	return false;
    }
    return true;
  }

};



inline CellBoundRef merge(CellBoundRef b1, CellBoundRef b2) 
{ 
  // std::cerr << "b1: " << b1->toStr() << std::endl;
  // std::cerr << "b2: " << b2->toStr() << std::endl;


  if (b1->a->isFinite() && b2->a->isFinite() && !b1->l->equal(b2->l) && equal(b1->a,b2->a)) return NULL;
  if (b1->b->isFinite() && b2->b->isFinite() && !b1->u->equal(b2->u) && equal(b1->b,b2->b)) return NULL;
  return new CellBoundObj(b1,b2); 
}

inline CellBoundRef mergeWithError(CellBoundRef b1, CellBoundRef b2, int &lowOrHigh) 
{ 
  // std::cerr << "b1: " << b1->toStr() << std::endl;
  // std::cerr << "b2: " << b2->toStr() << std::endl;

  if (b1->a->isFinite() && b2->a->isFinite() && !b1->l->equal(b2->l) && equal(b1->a,b2->a)) { lowOrHigh = -1; return NULL; }
  if (b1->b->isFinite() && b2->b->isFinite() && !b1->u->equal(b2->u) && equal(b1->b,b2->b)) { lowOrHigh =  1; return NULL; }
  return new CellBoundObj(b1,b2); 
}
}//end namespace tarski
#endif
