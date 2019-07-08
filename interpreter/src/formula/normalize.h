#ifndef _NORMALIZE_
#define _NORMALIZE_

#include "formula.h"
#include "formmanip.h"

namespace tarski {
extern TFormRef testmi(TFormRef F); // temp to test!

// Normalization functions: 
// all assume that F is either constant, and atomic formula or
// a conjunction.

// Straight-up functions ... probably won't use except internally
  bool level1_atom(TAtomRef A, TAndRef C);
  TFormRef level1(TFormRef F, TAtomRef* ptr = NULL);
  TFormRef level2(TFormRef F);
  TFormRef level3(TFormRef F, int l3flags, int l4flags);


const int nf_ded_inconsistencies = 1;
const int nf_ded_implications = 2;
const int nf_ded_strengthenings = 4;

// Normalizer objects
class Normalizer {
public:
  virtual TFormRef normalize(TFormRef F) = 0;
  virtual ~Normalizer() { }
};
class Level0 : public Normalizer { TFormRef normalize(TFormRef F) { return F; } };
class Level1 : public Normalizer {
  TAtomRef unsatAtom;
  TFormRef normalize(TFormRef F) { return level1(F,&unsatAtom); }
 public:
  TAtomRef getUnsatAtom() { return unsatAtom; }
};
class Level2 : public Normalizer { TFormRef normalize(TFormRef F) { return level2(level1(F)); } };
class Level3and4 : public Normalizer 
{ 
 public:
  int l3flags, l4flags;
  Level3and4(int _l3flags, int _l4flags) :  l3flags(_l3flags), l4flags(_l4flags) { }
  TFormRef normalize(TFormRef F) 
  { 
    if (F->constValue() != -1) 
      return new TConstObj(F->constValue());
    TFormRef Fo = level3(level1(F),l3flags,l4flags); 
    TFormRef Fn = normalizeMOD(F);
    
    if (false && constValue(Fn) != constValue(Fo))
    {
      cout << "Fx: "; F->write(0); cout << endl;
      cout << "Fo: "; Fo->write(0); cout << endl;
      cout << "Fn: "; Fn->write(0); cout << endl;
    }
    return Fn;
  } 
  
  // stupid test
  TFormRef normalizeMOD(TFormRef F) { 
    TFormRef F0 = F;

// Commented out is hack to induce deductions on variables that don't appear as factors
    VarSet S = F0->getVars();
    for(VarSet::iterator i = S.begin(); i != S.end(); ++i)
    {
      IntPolyRef p = new IntPolyObj(*i);
      TAtomRef a = makeAtom(*(F->getPolyManagerPtr()),p,ALOP);
      F0 = F0 && a;
    }

    TFormRef F1 = testmi(level1(F0)); // better if I could fix testmi to deal with things like 1 < 0
    TFormRef F2 = level3(level1(F1),l3flags,l4flags);
    TFormRef F3 = testmi(F2);
    TFormRef F4 = level3(level1(F3),l3flags,l4flags);
    return F4;
  } 
};

/*
  Level1_5 normalizer: This essentially does blockbox normalization only.
*/
class Level1_5 : public Normalizer
{
  TFormRef normalize(TFormRef F)
  {
    TFormRef F1 = level1(F);
    switch (F1->getTFType()){
    case TF_CONST: case TF_ATOM: return F1; break;
    case TF_AND: return asa<TAndObj>(F1)->size() > 1 ? testmi(F1) : F1; break;
    default: throw TarskiException("Level1_5 Normalizer can only handle constants, atoms or conjunctions.");
    }
    
  }
};


/* NORMALIZER OF TYPE raw

   The 'raw' normalizer of level k just applies conjunction normalizer of level k to
   the individual pieces, so that: A /\ [B \/ C] is "normalized" to A' /\ [B' \/ C']
   where A', B' and C' are the results of independent calls to normalize A, B and C
   respectively.  In particular, this means that B and C are NOT normalized under the
   assumptions of A.  Thus x >= 0 /\ [ x <= 0 \/ x y < 0] would not be simplified at
   all.  Level "0" refers to whatever the current normalizer is.
 */
class RawNormalizer : public TFPolyFun
{
  TFormRef res;  
  Normalizer *normp;
 public:
  TFormRef getRes() { return res; }
  RawNormalizer(Normalizer &N) { normp = &N; }
  RawNormalizer(Normalizer* p) { normp = p; }
  void action(TConstObj* p) { res = p; }
  void action(TAtomObj* p) { res = normp->normalize(p); }
  void action(TAndObj* p) 
  { 
    // 1: split into two conjunctions - pure atomic formulas, and non-pure atomic formulas
    // 2: simplify each non-pure atomic formulas in isolation & collect
    // 3: simplify the pure-atomic formulas part
    TAndRef pureAnd = new TAndObj();
    TAndRef finalAnd = new TAndObj();
    for(TAndObj::conjunct_iterator itr = p->conjuncts.begin(); itr != p->conjuncts.end(); itr++)
    {
      this->actOn(*itr);
      if (res->constValue() == FALSE)
	return;
      if (!isAndAndAtoms(res)) 
	finalAnd->AND(res);
      else
	pureAnd->AND(res);
    }
    finalAnd->AND(normp->normalize(pureAnd));
    if (finalAnd->constValue() != -1)
      res = new TConstObj(finalAnd->constValue());
    else if (finalAnd->size() == 1)
      res = *(finalAnd->conjuncts.begin());
    else      
      res = finalAnd;
  }
  void action(TOrObj* p) 
  {
    TOrRef newor = new TOrObj();
    for(TOrObj::disjunct_iterator itr = p->disjuncts.begin(); itr != p->disjuncts.end(); itr++)
    {
      this->actOn(*itr);
      if (res->constValue() == TRUE){ res = new TConstObj(TRUE); return; }
      newor->OR(res);
    }
    if (newor->size() == 0)
      res = new TConstObj(FALSE);
    else if (newor->size() == 1)
      res = *(newor->disjuncts.begin());
    else
      res = newor;
  }
  void action(TQBObj* p) 
  { 
    // normalize formula part.  if constant, ignore quantifiers and return constant
    this->actOn(p->formulaPart);
    if (constValue(res) == TRUE || constValue(res) == FALSE) return;
    
    // get the free variables of the formula part.  simplify quantifier blocks by only including
    // variables that appear free in the formula part.
    VarSet V = getFreeVars(res); //-- this is pretty inefficient! 
    vector<VarSet> lblocks;
    int firsti = -1;
    for(unsigned int i = 0; i < p->blocks.size(); i++)
    {
      VarSet next = V & p->blocks[i];
      if (next.any())
      {
	if (lblocks.size() == 0) { lblocks.push_back(next); firsti = i; }
	else if ((i - firsti) % 2 == lblocks.size() % 2) { lblocks.push_back(next); }
	else { lblocks.back() |= next; }
      }
    }
    if (lblocks.size() == 0) { return; }
    
    int oldInnermostBlockType = p->blocks.size() % 2 == 0 
                                ? quantifierNegation(p->outermostBlockType) 
                                : p->outermostBlockType;

    int newInnermostBlockType = firsti % 2 == 0 ? oldInnermostBlockType : quantifierNegation(oldInnermostBlockType);

    TQBRef newQB = new TQBObj(lblocks[0],newInnermostBlockType,res,p->getPolyManagerPtr());
    for(unsigned int i = 1; i < lblocks.size(); ++i)
      newQB->add(lblocks[i],i%2 == 0 ? newInnermostBlockType : quantifierNegation(newInnermostBlockType));
    res = newQB;
  }
};
}//end namespace tarski
#endif
