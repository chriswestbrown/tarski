#include "polymanager.h"
#include "sacMod.h"
#include "tracker.h"
#include "factor.h"

using namespace std;

namespace tarski {
  
extern Tracker compTracker;

IntPolyRef PolyManager::getCannonicalCopy(IntPolyRef p)
{
  set<IntPolyRef,PolyLT>::iterator i = polyset.find(p);
  if (i == polyset.end())
  {
    polyset.insert(p);
    i = polyset.find(p);
  }
  return *i;
}

void PolyManager::factorHelper(IntPolyRef p, Word &s, Word &c, Word &L)
{
  compTracker.recordSacPoly("fac",p->slevel,p->sP);
    Word t0 = ACLOCK();
  GVCAP->IPFAC(p->slevel,p->sP,&s,&c,&L);
    Word t1 = ACLOCK();
    getStats()->time_sac_factor += (ACLOCK() - t0);
}


IntPolyRef PolyManager::resultant(IntPolyRef p, IntPolyRef q, VarSet x)
{  
  VarSet V = p->getVars() + q->getVars();
  Word r = V.numElements();
  Word i = V.positionInOrder(x);
  Word A = PMMVPO(r,p->expand(V),i), B = PMMVPO(r,q->expand(V),i);

  compTracker.recordUnordered("res",A,B);

    Word t0 = ACLOCK();
    //  Word R = IPRESPRS(r,A,B);
    Word R = GVCAP->IPRES(r,A,B);
    getStats()->time_sac_resultant += (ACLOCK() - t0);
  return new IntPolyObj(r-1,R,V ^ x);
}

IntPolyRef PolyManager::discriminant(IntPolyRef p, VarSet x)
{
  VarSet V = p->getVars();
  Word r = V.numElements();
  Word i = V.positionInOrder(x);
  Word A = PMMVPO(r,p->getSaclibPoly(),i);

  compTracker.recordSacPoly("dis",r,A);

  //Word D = IPDSCR(r,A);
  Word D = GVCAP->IPDSCR(r,A);
  return new IntPolyObj(r-1,D,V ^ x);
}

FactRef PolyManager::resultantFactors(IntPolyRef p, IntPolyRef q, VarSet x)
{
  int mult = 1;
  VarSet V = p->getVars() + q->getVars();
  Word r = V.numElements();
  Word i = V.positionInOrder(x);
  Word A = PMMVPO(r,p->expand(V),i), B = PMMVPO(r,q->expand(V),i);

  compTracker.recordUnordered("res",A,B);

  Word s,c,L;
  Word t0 = ACLOCK();
  GVCAP->IPFACRES(r,A,B,&s,&c,&L);
  Word t1 = ACLOCK();
  //SWRITE("IPFACDSCR"); OWRITE(L); SWRITE("\n");
  //M->getStats()->time_sac_factor += (ACLOCK() - t0);
  FactRef F = new FactObj(*this);
  F->content = IPROD(F->content,IEXP(IPROD(s,c),mult));
  for(Word Lp = L; Lp != NIL; Lp = RED(Lp))
    F->addFactor(new IntPolyObj(r-1,SECOND(FIRST(Lp)),V ^ x), mult*FIRST(FIRST(Lp)));
  return F;  
}

FactRef PolyManager::discriminantFactors(IntPolyRef p, VarSet x)
{
  int mult = 1;
  VarSet V = p->getVars();
  Word r = V.numElements();

  if (r == 0) { return new FactObj(*this); }

  Word i = V.positionInOrder(x);
  Word A = PMMVPO(r,p->getSaclibPoly(),i);

  compTracker.recordSacPoly("dis",r,A);

  //SWRITE("Initial A"); OWRITE(A); SWRITE(":"); IWRITE(i); SWRITE("\n");
  Word s,c,L;
    Word t0 = ACLOCK();
  GVCAP->IPFACDSCR(r,A,&s,&c,&L);
    Word t1 = ACLOCK();
    //SWRITE("IPFACDSCR"); OWRITE(L); SWRITE("\n");
    //M->getStats()->time_sac_factor += (ACLOCK() - t0);
  FactRef F = new FactObj(*this);
  F->content = IPROD(F->content,IEXP(IPROD(s,c),mult));
  for(Word Lp = L; Lp != NIL; Lp = RED(Lp))
    F->addFactor(new IntPolyObj(r-1,SECOND(FIRST(Lp)),V ^ x), mult*FIRST(FIRST(Lp)));
  return F;
}

FactRef PolyManager::ldcfFactors(IntPolyRef p, VarSet x)
{
  int mult = 1;
  VarSet V = p->getVars();
  Word r = V.numElements();

  if (r == 0) { return new FactObj(*this); }

  Word i = V.positionInOrder(x);
  Word A = PMMVPO(r,p->getSaclibPoly(),i);
  //SWRITE("Initial A"); OWRITE(A); SWRITE(":"); IWRITE(i); SWRITE("\n");
  Word s,c,L;
    Word t0 = ACLOCK();
  GVCAP->IPFACLDCF(r,A,&s,&c,&L);
    Word t1 = ACLOCK();
    //SWRITE("IPFACLDCF"); OWRITE(L); SWRITE("\n");
    //M->getStats()->time_sac_factor += (ACLOCK() - t0);
  FactRef F = new FactObj(*this);
  F->content = IPROD(F->content,IEXP(IPROD(s,c),mult));
  for(Word Lp = L; Lp != NIL; Lp = RED(Lp))
    F->addFactor(new IntPolyObj(r-1,SECOND(FIRST(Lp)),V ^ x), mult*FIRST(FIRST(Lp)));
  return F;
}

IntPolyRef PolyManager::evalAtRationalMultipleOfVariable(IntPolyRef A, VarSet xi, Word q, VarSet xj, GCWord &content)
{
  // if xi is not present in A, there's nothing to do 
  if ((A->getVars() & xi).none()) { content = RNINT(1); return A; }

  // create xi = q xj ---> xi = r/s xj ---> s xi - r xj
  Word r = RNNUM(q), s = RNDEN(q);
  IntPolyRef u = new IntPolyObj(1,LIST2(1,s),xi);
  IntPolyRef v = new IntPolyObj(1,LIST2(1,r),xj);
  IntPolyRef w = dif(u,v);

  // compute r := res_xi(A,w) = -1^deg_xi(A) * s^deg_xi(A) * A|_{xi=r/s*xj}
  IntPolyRef R = resultant(A,w,xi);
  if (R->isZero()) { content = RNINT(1); return R; }
  Word n = R->numVars(), H = R->sP;
  VarSet V = R->getVars();

  // Make R positive & primitive in order to determine content properly and produce B
  Word c, Hb;
  IPICPP(n,H,&c,&Hb);
  int d = A->degree(xi);
  Word tmp = RNRED(c,IEXP(s,d));
  content = d % 2 == 0 ? tmp : RNNEG(tmp);
  IntPolyRef B = new IntPolyObj(n,Hb,V);

  // NOTE: content*B = A|_{xi=r/s*xj}
  return B;
}

IntPolyRef PolyManager::evalAtRationalPointMakePrim(const IntPolyRef &p, VarKeyedMap<GCWord> &value, GCWord &content)
{
  VarSet V = p->getVars(), Vremain = 0;
  Word perm1 = NIL, perm2 = NIL, Q = NIL;
  int i = 1;
  for(VarSet::iterator itr = V.begin(); itr != V.end(); ++itr, ++i)
  {
    Word a = value[*itr];
    if (a == NIL) { perm2 = COMP(i,perm2); Vremain = Vremain + *itr; }
    else { perm1 = COMP(i,perm1); Q = COMP(a,Q); }
  }
  Q = CINV(Q);

  // if Q == NIL, there are no values to evaluate at, so just return
  if (Q == NIL) { content = RNINT(1); return p; }

  return evalAtRationalPointMakePrimHelper(p, Q, perm1, perm2, Vremain, content);
}

IntPolyRef PolyManager::evalAtRationalPointMakePrimHelper(const IntPolyRef &p, Word Q, Word perm1, Word perm2, VarSet Vremain, GCWord &content)
{
  // TEST: record LIST3(Q,slevel,sP);
  compTracker.record("eva",LIST3(Q,p->slevel,p->sP));

  Word perm = CINV(CCONC(perm2,perm1));
  Word sPnew = PPERMV(p->slevel,p->sP,perm);
  Word sPeval = IPRNME(p->slevel,sPnew,Q);
  Word sleveleval = p->slevel - LENGTH(Q);

  if (sPeval == 0) { content = RNINT(1); return new IntPolyObj(); }
  if (sleveleval == 0) { content = sPeval; return new IntPolyObj(1); }

  Word c, Cb;
  IPSRP(sleveleval,sPeval,&c,&Cb);
  content = RNINV(c);
  return new IntPolyObj(sleveleval,Cb,Vremain);
  
}


IntPolyRef PolyManager::prem(IntPolyRef p, IntPolyRef q, VarSet x)
{
  VarSet V = p->getVars() + q->getVars();
  Word r = V.numElements();
  Word i = V.positionInOrder(x);
  Word A = PMMVPO(r,p->expand(V),i), B = PMMVPO(r,q->expand(V),i);
  Word R = IPPSR(r,A,B);
  R = PUNDOMMVPO(r,R,i);
return new IntPolyObj(r,R,V);
}

IntPolyRef PolyManager::ldcf(IntPolyRef p, VarSet x)
{ 
  Word i = p->svars.positionInOrder(x);
  if (i == 0) return new IntPolyObj; 
  Word A = PMMVPO(p->slevel,p->sP,i);
  return new IntPolyObj(p->slevel-1,PLDCF(A),p->svars ^ x);
}

IntPolyRef PolyManager::trcf(IntPolyRef p, VarSet x)
{ 
  Word i = p->svars.positionInOrder(x);
  if (i == 0) return new IntPolyObj; 
  Word A = PMMVPO(p->slevel,p->sP,i);
  return new IntPolyObj(p->slevel-1,PTRCF(A),p->svars ^ x);
}

IntPolyRef PolyManager::coef(IntPolyRef p, VarSet x, int deg)
{ 
  Word i = p->svars.positionInOrder(x);
  if (i == 0) return new IntPolyObj; 
  Word A = PMMVPO(p->slevel,p->sP,i);
  return new IntPolyObj(p->slevel-1,PCOEFF(A,deg),p->svars ^ x);
}

IntPolyRef PolyManager::prod(IntPolyRef p, IntPolyRef q)
{
  if (p->isZero()) return p;
  if (q->isZero()) return q;
  VarSet V = p->getVars() + q->getVars();
  Word r = V.numElements();
  Word S = IPPROD(r,p->expand(V),q->expand(V));
  return new IntPolyObj(r,S,V);    
}
IntPolyRef PolyManager::sum(IntPolyRef p, IntPolyRef q)
{
  if (p->isZero()) return q;
  if (q->isZero()) return p;
  VarSet V = p->getVars() + q->getVars();
  Word r = V.numElements();
  Word S = IPSUM(r,p->expand(V),q->expand(V));
  return new IntPolyObj(r,S,V);    
}
IntPolyRef PolyManager::dif(IntPolyRef p, IntPolyRef q)
{
  if (p->isZero()) return neg(q);
  if (q->isZero()) return p;
  VarSet V = p->getVars() + q->getVars();
  Word r = V.numElements();
  Word S = IPDIF(r,p->expand(V),q->expand(V));
  return new IntPolyObj(r,S,V);    
}
IntPolyRef PolyManager::neg(IntPolyRef p)
{
  IntPolyRef r = new IntPolyObj();
  r->slevel = p->slevel;
  r->svars = p->svars;
  r->sP = IPNEG(p->slevel,p->sP);
  return r;
}

void PolyManager::nonZeroCoefficients(IntPolyRef p, VarSet Z, vector<IntPolyRef> &V)
{
  if (Z.none()) return;
  VarSet::iterator itr = Z.begin();
  VarSet x = *itr;
  VarSet Zp = Z ^ x;
  Word i = p->svars.positionInOrder(x);
  if (i == 0) { return nonZeroCoefficients(p,Zp,V); }
  Word A = PMMVPO(p->slevel,p->sP,i);
  Word C = PCL(A);
  if (Zp.none())
    for(Word Cp = C; Cp != NIL; Cp = RED(Cp)) V.push_back(new IntPolyObj(p->slevel - 1,FIRST(Cp),Zp));
  else
    for(Word Cp = C; Cp != NIL; Cp = RED(Cp)) nonZeroCoefficients(new IntPolyObj(p->slevel - 1,FIRST(Cp),Zp),Zp,V);

}

// Special S-polynomial
// Input: Polynomials p and q, variable x. deg_x(p) = deg_x(q) > 1.
// Output: NULL or polynomial (not a cannonical copy!)
IntPolyRef PolyManager::specialSpoly(IntPolyRef p, IntPolyRef q, VarSet x)
{
  VarSet V = p->getVars() + q->getVars();
  Word r = V.numElements();
  Word i = V.positionInOrder(x);
  Word P = p->expand(V), Q = q->expand(V);

  Word A = PMMVPO(r,P,i), B = PMMVPO(r,Q,i);
  Word a = PLBCF(r,A);
  Word b = PLBCF(r,B);

  Word R = IPDIF(r,IPIP(r,b,P),IPIP(r,a,Q));

  return new IntPolyObj(r,R,V);
}
}//end namespace tarski
