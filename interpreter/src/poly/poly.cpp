#include "poly.h"
#include "../formrepconventions.h"
#include "sacMod.h"
#include <algorithm>

using namespace std;

namespace tarski {

int IntPolyObj::degree(VarSet x) const // x should be the varset for a single variable
{
  if ((svars & x).none()) return 0;
  int i = 1;
  for(VarSet::iterator itr = svars.begin(); (*itr & x).none(); ++itr)
    ++i;
  return PDEGSV(slevel,sP,i);
}

int IntPolyObj::totalDegree()
{
  if (slevel == 0) return 0;
  return PTDEG(slevel,sP);
}

VarSet IntPolyObj::linearlyOccurringVariables()
{
  VarSet res;
  Word D = PDEGV(slevel,sP);
  for(VarSet::iterator itr = svars.rbegin(); itr != svars.rend(); --itr, D = RED(D))
    if (FIRST(D) == 1)
      res = res + *itr;
  return res;
}

VarSet IntPolyObj::variableUniqueDegrees()
{
  VarSet res;
  Word D = PUDV(slevel,sP);
  for(VarSet::iterator itr = svars.rbegin(); itr != svars.rend(); --itr, D = RED(D))
    if (FIRST(D) > 0)
      res = res + *itr;
  return res;
}

  int IntPolyObj::compare(IntPolyRef p) {
    return (this - &(*p));
  }

int IntPolyObj::signDeduce(const VarKeyedMap<int> &varSign)
{
  VarSet S = getVars();
  Word L = NIL;
  for(VarSet::iterator itr = S.begin(); itr != S.end(); ++itr)
    L = COMP(varSign.get(*itr),L);
  int ded = ::signDeduce(slevel,sP,L);
  return ded;
}

VarSet minimalReqdForSignDeduce(Word slevel, Word sP, Word L, VarSet S, int ded)
{
  VarSet Sp;
  Word Lp = L;
  for(VarSet::iterator itr = S.rbegin(); itr != S.rend(); --itr)
  {
    int tmp = FIRST(Lp);
    SLELTI(Lp,1,ALOP);
    if (ded != ::signDeduce(slevel,sP,L))
    {
      SLELTI(Lp,1,tmp);
      Sp = Sp + *itr;
    }
    Lp = RED(Lp);
  }
  return Sp;
}

// NOTE:  This is a godawful implementation!
int IntPolyObj::signDeduceMinimalAssumptions(const VarKeyedMap<int> &varSign, VarSet &used)
{
  VarSet S = getVars();
  Word L = NIL;
  for(VarSet::iterator itr = S.begin(); itr != S.end(); ++itr)
    L = COMP(varSign.get(*itr),L);
  int ded = ::signDeduce(slevel,sP,L);

  used = minimalReqdForSignDeduce(slevel, sP, L, S, ded);
  return ded;
}


int IntPolyObj::indexOfVar(VarSet x)
{
  int i = 1;
  for(VarSet::iterator itr = svars.begin(); (*itr & x).none(); ++itr)
    ++i;
  return i;
}

int IntPolyObj::gcdOfVariableExponents(VarSet x) 
{
  if ((x & svars).none()) return 0;
  return PGCDEXP(slevel, sP, indexOfVar(x)); 
}

IntPolyRef IntPolyObj::reduceDegreeInX(VarSet x, int d)
{
  if ((x & svars).none()) return this;
  Word newP = PREDDEGXi(slevel, sP, indexOfVar(x), d);
  return new IntPolyObj(slevel,newP,svars);
}

void IntPolyObj::write(VarContext &C) // Write to the current Saclib Output Context
{
  Word V = varList(C,svars);
  IPDWRITE(slevel,sP,V);
}

int IntPolyObj::read(VarContext &C, VarSet S) // Read from current saclib input context
{
  Word V = varList(C,S);
  Word r = LENGTH(V), t, A;
  IPEXPREAD(r,V,&A,&t);
  if (t) compactify(r,A,S); else A = 0;
  return t;
}

namespace WriteSMTLIBImplementation
{
  // IntPolyObj::writeSMTLib(C,out) writes the "this" polynomial in SMTLib
  // format to the output stream out.  The class & functions in this namespace
  // help make the possible.

  // tpair: represents value "w" with type "kind",paired together.
  // char "kind" can be one of: +, -, *, u (unary minus), v (variable), i integer
  class tpair 
  { public: GCWord w; char kind; 
    tpair(Word w, char kind) { this->w = w; this->kind = kind; } 
    bool isOne() { return kind == 'i' && ICOMP(w,1) == 0; }
  };

  // foo(V,r,A,drem) takes saclib polynomial r,A writes it in postfix notation
  // into the vector V, following Horner's rule.
  //  drem is "degree remaining", which is -1 for the initial
  // call, and then is used to determine whether to consume the next term in the
  // saclib representation, or multiply by the main variable instead.  
  void foo(vector<tpair> &V,  Word r, Word A, int drem = -1)
  {
    Word e, c, Ap;
    if (r == 0)
    {
      if (ISIGNF(A) < 0) { V.push_back(tpair(IABSF(A),'i')); V.push_back(tpair(0,'u')); }
      else {  V.push_back(tpair(IABSF(A),'i')); }
    }
    else if (A == NIL)
    {
      while (drem > 0) { V.push_back(tpair(r,'v')); V.push_back(tpair(0,'*')); drem--; }
    }
    else if (drem == -1)
    {
      ADV2(A,&drem,&c,&Ap);
      foo(V,r-1,c);
      if (drem > 0) { V.push_back(tpair(r,'v')); V.push_back(tpair(0,'*')); }
      foo(V,r,Ap,drem-1);
    }
    else
    {
      ADV2(A,&e,&c,&Ap);
      if (e == drem)
      {
	foo(V,r-1,c);
	V.push_back(tpair(0,'+'));
	if (drem > 0) { V.push_back(tpair(r,'v')); V.push_back(tpair(0,'*')); }
	foo(V,r,Ap,drem-1);
      }
      else
      {
	V.push_back(tpair(r,'v')); V.push_back(tpair(0,'*'));
	foo(V,r,A,drem-1);
      }
    }
  }

  // bar(V,W,C,out,i) writes out the polynomial represented in postfix in V to
  // output stream out.  For the initial call, i should be the largest index in V.
  int bar(vector<tpair>& V, vector<VarSet>& W, VarContext &C, ostream& out, int i)
  {
    if (i < 0) return i;
    Word w = V[i].w;
    char kind = V[i].kind;
    int newi = i-1;
    if (kind == 'v')
      out << C.getName(W[w]);
    else if (kind == 'i')
      IWRITE(w);
    else if (kind == 'u')
    {
      out << "(- "; newi = bar(V,W,C,out,newi); out << ")";
    }
    else if (kind == '*' && V[newi].isOne()) { newi = bar(V,W,C,out,newi-1); }
    else if (kind == '*' && V[newi-1].isOne()) { swap(V[newi-1],V[newi]); newi = bar(V,W,C,out,newi-1); }  
    else
    {
      out << '(' << kind << ' ';
      newi = bar(V,W,C,out,newi);
      out << ' ';
      newi = bar(V,W,C,out,newi);
      out << ")";
    }
    return newi;
  }
}

void IntPolyObj::writeSMTLIB(VarContext &C, ostream& out)
{
  Word r = numVars();
  Word A = sP;
  vector<VarSet> W;
  W.push_back(0);
  for(VarSet::iterator itr = getVars().begin(); itr != getVars().end(); ++itr)
    W.push_back(*itr);
  vector<WriteSMTLIBImplementation::tpair> V;
  WriteSMTLIBImplementation::foo(V,r,A);
  PushOutputContext(out);
  WriteSMTLIBImplementation::bar(V,W,C,out,V.size()-1);  
  PopOutputContext();
}

void IntPolyObj::writeMAPLE(VarContext &C, ostream& out)
{
  PushOutputContext(out);
  Word V = varList(C,svars);
  IPDWRITEMAPLE(slevel,sP,V);
  PopOutputContext();
}

void IntPolyObj::writeMATHEMATICA(VarContext &C, ostream& out)
{ // This is easy since Mathematica's polynomial input format is the same as Saclib's!
  PushOutputContext(out);
  write(C);
  PopOutputContext();
}


int IntPolyObj::readRatSetSim(VarContext &C, VarSet S) // Read from current saclib input context
{
  Word V = varList(C,S);
  Word r = LENGTH(V), t, A,Ap,c;
  RPEXPREAD(r,V,&Ap,&t);
  IPSRP(r,Ap,&c,&A);
  if (!t) { A = 0; return t; }
  if (RNSIGN(c) < 0) { A = IPNEG(r,A); }
  compactify(r,A,S);
  return t;
}

//private:
void IntPolyObj::compactify(Word r, Word A, VarSet S)
{
  if (r == 0) { svars = 0; slevel = 0; sP = A; return; }
  // Set Sp = compacted VarSet, rp = # appearing vars, Lf & Lb
  // to appearing & non-appearing variable indices
  VarSet Sp;
  VarSet::iterator itr = S.rbegin();
  Word D = PDEGV(r,A), rp = 0, i = r, Lf = NIL, Lb = NIL;
  while(D != NIL)
  {
    if (FIRST(D) == 0) { Lb = COMP(i,Lb); }
    else { Lf = COMP(i,Lf); Sp = Sp + *itr; ++rp; }
    --itr;
    --i;
    D = RED(D);
  }
  /*
  SWRITE("\nr = "); IWRITE(r); SWRITE("\n");
  SWRITE("A = "); OWRITE(A); SWRITE("\n");
  SWRITE("Lf = "); OWRITE(Lf); SWRITE("\n");
  */
  // Remove non-appearing vars from saclib rep, retaining order
  Word Ap;
  if (rp == r)
    Ap = A;
  else {
    Word P = CCONC(Lf,Lb);
    Ap = PPERMV(r,A,P);
    for(int k = 0; k < r - rp; ++k)
      Ap = PLDCF(Ap);
  }
  
  // Set this poly to the compacted form.
  svars = Sp;
  sP = Ap;
  slevel = rp;
}
Word IntPolyObj::expand(Word r, Word A, VarSet oldV, VarSet newV) const // oldV contained in newV
{
  Word rp = r, P = NIL, i = 1;
  for(VarSet::iterator i_old = oldV.begin(), i_new = newV.begin();
      i_new != newV.end(); ++i_new)
  {
    if (*i_old == *i_new) { P = COMP(i++,P); ++i_old; }
    else { P = COMP(++rp,P); }
  }
  P = CINV(P);

  if (rp == 0) return A;

  Word Ap = A;
  for(int k = 0; k < rp - r; ++k) Ap = LIST2(0,Ap);
  Ap = PPERMV(rp,Ap,P);
  return Ap;
}

// IntPolyRef IntPolyObj::sum(IntPolyRef p, IntPolyRef q)
// {  
//   if (p->sP == 0) return q;
//   if (q->sP == 0) return p;
//   if (p->isConstant() && q->isConstant()) return makeTerm(0,ISUM(p->sP,q->sP));
//   VarSet V = p->getVars() + q->getVars();
//   Word r = V.numElements();
//   Word A = p->expand(V), B = q->expand(V);
//   Word S = IPSUM(r,A,B);
//   return new IntPolyObj(r,S,V);
// }

  bool IntPolyObj::ipcmp(IntPolyRef a, IntPolyRef b) 
  { 
    if (a.vpval() == b.vpval()) return false;
    if (a->svars.to_ulong() !=  b->svars.to_ulong()) 
      return a->svars.to_ulong() <  b->svars.to_ulong();
    int t = cmp(a->slevel, a->sP, b->sP);
    return t < 0;
  }  

  int IntPolyObj::ipcompare(IntPolyRef a, IntPolyRef b) 
  { 
    if (a.vpval() == b.vpval()) return 0;
    if (a->svars.to_ulong() !=  b->svars.to_ulong()) 
      return ISIGNF(a->svars.to_ulong() -  b->svars.to_ulong());
    return cmp(a->slevel, a->sP, b->sP);
  }  
  


IntPolyRef IntPolyObj::integerProduct(Word a) const
{
  return new IntPolyObj(getLevel(),IPIP(getLevel(),a,getSaclibPoly()),getVars());
}



// Linear root further from zero
// this and q are both linear, univariate, containing the same variable.
// both roots have the same side.  Return true if "this" has root further
// from zero than q.
bool IntPolyObj::linearRootFurtherFromZero(IntPolyRef q)
{  
  Word r_p = RNRED(PCOEFF(sP,0),PLDCF(sP)); 
  Word r_q = RNRED(PCOEFF(q->sP,0),PLDCF(q->sP)); 
  return RNSIGN(RNDIF(RNABS(r_p),RNABS(r_q))) > 0;
}

IntPolyRef IntPolyObj::evalAtRationalPointMakePrim(VarKeyedMap<GCWord> &value, GCWord &content)
{
  VarSet V = getVars(), Vremain = 0;
  Word perm1 = NIL, perm2 = NIL, Q = NIL;
  int i = 1;
  for(VarSet::iterator itr = V.begin(); itr != V.end(); ++itr, ++i)
  {
    Word a = value[*itr];
    if (a == NIL) { perm2 = COMP(i,perm2); Vremain = Vremain + *itr; }
    else { perm1 = COMP(i,perm1); Q = COMP(a,Q); }
  }
  
  // if Q == NIL, there are no values to evaluate at, so just return
  if (Q == NIL) { content = RNINT(1); return this; }

  Q = CINV(Q);

  Word perm = CINV(CCONC(perm2,perm1));
  Word sPnew = PPERMV(slevel,sP,perm);
  Word sPeval = IPRNME(slevel,sPnew,Q);
  Word sleveleval = slevel - LENGTH(Q);

  if (sPeval == 0) { content = RNINT(1); return new IntPolyObj(); }
  if (sleveleval == 0) { content = sPeval; return new IntPolyObj(1); }

  Word c, Cb;
  IPSRP(sleveleval,sPeval,&c,&Cb);
  content = RNINV(c);
  return new IntPolyObj(sleveleval,Cb,Vremain);
}



IntPolyRef IntPolyObj::evalAtRationalMakePrim(VarSet x, Word z)
{
  Word i = svars.positionInOrder(x);
  if (i == 0) return this;
  Word A = PMMVPO(slevel,sP,i);
  Word B = RPFIP(slevel,A);
  Word C = RPEMV(slevel,B,z);
  Word c, Cb;
  IPSRP(slevel-1,C,&c,&Cb);
  if (RNSIGN(c) < 0) { Cb = IPNEG(slevel-1,Cb); }
  return Cb == 0 ? new IntPolyObj() : new IntPolyObj(slevel-1,Cb,svars ^ x);
}



IntPolyRef IntPolyObj::derivative(VarSet x)
{
  Word i = svars.positionInOrder(x);
  if (i == 0) return new IntPolyObj();
  Word D = IPDER(slevel,sP,i);
  IntPolyRef res = new IntPolyObj(slevel,sP,svars);
  return res;
}

// M's nullvalue must be set to a vector of 3, maxdeg, maxtdeg,nterms
void IntPolyObj::varStats(VarKeyedMap< vector<int> > &M)
{
  Word L = IPVARSTATS(this->slevel,this->sP);
  VarSet V = this->getVars();
  for(VarSet::iterator vitr = V.begin(); vitr != V.end(); ++vitr)
  {
    Word w = FIRST(L);
    L = RED(L);
    vector<int> res = M[*vitr];
    res[0] = max(res[0],FIRST(w));
    res[1] = max(res[1],SECOND(w));
    res[2] = res[2] + THIRD(w);
    M[*vitr] = res;
  }
}

class RenameVarsHelper
{
public:
  int slotInOrig;
  VarSet newVar;
  RenameVarsHelper(int i, VarSet v) { slotInOrig = i; newVar = v; }
  bool operator<(const RenameVarsHelper & h) const { return newVar < h.newVar; }
};

IntPolyRef IntPolyObj::renameVars(VarKeyedMap<VarSet> &M)
{
  vector<RenameVarsHelper> H;
  VarSet V = getVars();
  int i = 1;
  for(VarSet::iterator itr = V.begin(); itr != V.end(); ++itr, ++i)
    H.push_back(RenameVarsHelper(i,M[*itr]));
  sort(H.begin(),H.end());
  VarSet Vp;
  Word W = NIL;
  for(int j = H.size() - 1; j >= 0; j--)
  {
    W = COMP(H[j].slotInOrig,W);
    Vp = Vp + H[j].newVar;
  }
  int r = numVars();
  Word Pp = PPERMV(r,sP,W);
  return new IntPolyObj(r,Pp,Vp);
}

IntPolyRef IntPolyObj::makePositive(int& sn)
{
  sn = IPSIGN(slevel,sP);
  Word P = IPABS(slevel,sP);
  return new IntPolyObj(slevel,P,getVars());
}
}//end namespace tarski
