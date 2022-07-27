
//#include <md5.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <exception>

#include "formula.h"

namespace tarski {

TAtomObj* makeAtom(PolyManager &_M, IntPolyRef A, int sigma) 
{ 
  TAtomObj *p = new TAtomObj(_M); 
  p->F->addMultiple(A,1);
  p->relop = sigma;
  return p;
}

TAtomObj* makeAtom(FactRef A, int sigma)
{
  TAtomObj *p = new TAtomObj(*(A->M)); 
  p->F = A;
  p->relop = sigma;
  return p;

}


bool TAtomObj::OrderComp::operator()(const TAtomRef &A, const TAtomRef &B) const
{
  if (A->getRelop() < B->getRelop()) return true;
  if (A->getRelop() > B->getRelop()) return false;
  if (A->getFactors()->numFactors() < B->getFactors()->numFactors()) return true;
  if (A->getFactors()->numFactors() > B->getFactors()->numFactors()) return false;
  int t = OCOMP(A->getFactors()->getContent(),B->getFactors()->getContent());
  if (t != 0) { if (t < 0) return -1; else return 1; }
  FactObj::factorIterator itrA= A->getFactors()->factorBegin();
  FactObj::factorIterator itrB= B->getFactors()->factorBegin();
  while(itrA != A->getFactors()->factorEnd())
  {
    if (itrA->second < itrB->second) return true;
    if (itrA->second > itrB->second) return false;
    if (itrA->first < itrB->first) return true;
    if (itrB->first < itrA->first) return false;
    ++itrA;
    ++itrB;
  }
  return false;
}


/******************************************************
 * I'm going to make the assumption - unencoded, mind
 * you - that all conjuncts are inequalities or constants.
 ******************************************************/

int compareTF(TFormRef a, TFormRef b);

inline int cmp(int a, int b) { return a < b ? -1 : (b < a ? 1 : 0); }
class TFHierarchy : public TFPolyFun
{
public:
  int res; TFHierarchy() { res = -1; }
  void action(TConstObj* p) { res = 0;  }
  void action(TAtomObj* p) { res = 1; }
  void action(TExtAtomObj* p) { res = 2; }
  void action(TAndObj* p) { res = 3; }
  void action(TOrObj* p) { res = 4; }
  void action(TQBObj* p) { res = 5; }
  static int level(TFormRef a) { TFHierarchy Ha; Ha(a); return Ha.res; }
};

int cmpTF(TConstRef c_a, TConstRef c_b) { return ICOMP(c_a->value,c_b->value); }
int cmpTF(TAtomRef a_a, TAtomRef a_b)
{  
  map<IntPolyRef,int>::iterator itr_a = a_a->factorsBegin();
  map<IntPolyRef,int>::iterator itr_b = a_b->factorsBegin();
  while(itr_a != a_a->factorsEnd() && itr_b != a_b->factorsEnd())
  {
    if (itr_a->first < itr_b->first) return -1;
    if (itr_b->first < itr_a->first) return 1;
    if (itr_a->second < itr_b->second) return -1;
    if (itr_b->second < itr_a->second) return 1;	
    ++itr_a; ++itr_b;
  }
  if (itr_a == a_a->factorsEnd() && itr_b == a_b->factorsEnd())
  {
    int t = ICOMP(a_a->F->content,a_b->F->content);
    if (t == 0) { t = ICOMP(a_a->relop,a_b->relop); }
    return t;
  }
  return (itr_a == a_a->factorsEnd()) ? -1 : 1;
}

int cmpTF(TExtAtomRef a_a, TExtAtomRef a_b)
{  
  VarSet v_a = a_a->getLHSVar(), v_b = a_b->getLHSVar();
  if (v_a < v_b) return -1;
  if (v_b < v_a) return 1;
  map<IntPolyRef,int>::iterator itr_a = a_a->factorsBegin();
  map<IntPolyRef,int>::iterator itr_b = a_b->factorsBegin();
  while(itr_a != a_a->factorsEnd() && itr_b != a_b->factorsEnd())
  {
    if (itr_a->first < itr_b->first) return -1;
    if (itr_b->first < itr_a->first) return 1;
    if (itr_a->second < itr_b->second) return -1;
    if (itr_b->second < itr_a->second) return 1;	
    ++itr_a; ++itr_b;
  }
  if (itr_a == a_a->factorsEnd() && itr_b == a_b->factorsEnd())
  {
    int t = ICOMP(a_a->F->content,a_b->F->content);
    if (t == 0) { t = ICOMP(a_a->relop,a_b->relop); }
    return t;
  }
  return (itr_a == a_a->factorsEnd()) ? -1 : 1;
}

int cmpTF(TAndRef a, TAndRef b)
{
  TAndObj::conjunct_iterator aitr = a->conjuncts.begin();
  TAndObj::conjunct_iterator bitr = b->conjuncts.begin();
  while(aitr != a->conjuncts.end() && bitr != b->conjuncts.end())
  {
    int t = compareTF(*aitr,*bitr);
    if (t != 0) return t;
    ++aitr;
    ++bitr;
  }
  if (aitr == a->conjuncts.end() && bitr == b->conjuncts.end()) return 0;
  if (aitr == a->conjuncts.end()) return -1;
  return 1;
}

int cmpTF(TOrRef a, TOrRef b)
{
  TOrObj::disjunct_iterator aitr = a->disjuncts.begin();
  TOrObj::disjunct_iterator bitr = b->disjuncts.begin();
  while(aitr != a->disjuncts.end() && bitr != b->disjuncts.end())
  {
    int t = compareTF(*aitr,*bitr);
    if (t != 0) return t;
    ++aitr;
    ++bitr;
  }
  if (aitr == a->disjuncts.end() && bitr == b->disjuncts.end()) return 0;
  if (aitr == a->disjuncts.end()) return -1;
  return 1;
}

int compareTF(TFormRef a, TFormRef b)
{
  int la = TFHierarchy::level(a);
  int lb = TFHierarchy::level(b);
  if (la == -1) { cerr << "la == -1!" << endl; }
  if (lb == -1) { cerr << "lb == -1!" << endl; }
  if (la != lb) { return ICOMP(la,lb); }
  switch (la)
  {
  case 0: return cmpTF(asa<TConstObj>(a), asa<TConstObj>(b));
  case 1: return cmpTF(asa<TAtomObj>(a), asa<TAtomObj>(b));
  case 2: return cmpTF(asa<TExtAtomObj>(a), asa<TExtAtomObj>(b));
  case 3: return cmpTF(asa<TAndObj>(a), asa<TAndObj>(b));
  case 4: return cmpTF(asa<TOrObj>(a), asa<TOrObj>(b));
  default:
    return cmp(a->tag,b->tag);
  }
}



int conjunctCompare(TFormRef a, TFormRef b)
{
  static TFHierarchy Ha, Hb;
  Ha(a); Hb(b);
  if (Ha.res != Hb.res) { return cmp(Ha.res,Hb.res); }
  TConstRef c_a = asa<TConstObj>(a);
  TConstRef c_b = asa<TConstObj>(b);
  if (!c_a.is_null() && !c_b.is_null()) return ICOMP(c_a->value,c_b->value);
  TAtomRef a_a = asa<TAtomObj>(a);
  TAtomRef a_b = asa<TAtomObj>(b);
  if (!a_a.is_null() && !a_b.is_null())
  {
    map<IntPolyRef,int>::iterator itr_a = a_a->factorsBegin();
    map<IntPolyRef,int>::iterator itr_b = a_b->factorsBegin();
    while(itr_a != a_a->factorsEnd() && itr_b != a_b->factorsEnd())
    {
      if (itr_a->first < itr_b->first) return -1;
      if (itr_b->first < itr_a->first) return 1;
      if (itr_a->second < itr_b->second) return -1;
      if (itr_b->second < itr_a->second) return 1;	
      ++itr_a; ++itr_b;
    }
    if (itr_a == a_a->factorsEnd() && itr_b == a_b->factorsEnd())
    {
      int t = ICOMP(a_a->F->content,a_b->F->content);
      if (t == 0) { t = ICOMP(a_a->relop,a_b->relop); }
      return t;
    }
    return (itr_a == a_a->factorsEnd()) ? -1 : 1;
  }
  return cmp(a->tag,b->tag);
}

//bool ConjunctOrder::operator()(TFormRef a, TFormRef b) const { return conjunctCompare(a,b) < 0; }
bool ConjunctOrder::operator()(TFormRef a, TFormRef b) const { return compareTF(a,b) < 0; }

string toString(TFormRef t)
{
  ostringstream sout;
  PushOutputContext(sout);
  try { t->write(); } catch(exception &e) { PopOutputContext(); throw e; }
  PopOutputContext();
  return sout.str();
}

void TAndObj::write(bool flag) 
{ 
  if (conjuncts.size() == 0) { SWRITE("true"); return; }
  flag = false;
  SWRITE("["); 
  for(set<TFormRef>::iterator i = conjuncts.begin(); i != conjuncts.end(); ++i)
  {  
    if (i != conjuncts.begin()) 
      SWRITE(" /\\ "); 
    (*i)->write(); 
  }
  SWRITE("]");     
}

void TAndObj::AND(TFormRef a) 
{ 
  if (a->constValue() == TRUE)  return;
  if (a->constValue() == FALSE) { conjuncts.clear(); conjuncts.insert(a); return; }
  TAndRef aa = asa<TAndObj>(a); 
  if (aa.is_null())
    conjuncts.insert(a); 
  else
    for(set<TFormRef>::iterator itr = aa->conjuncts.begin(); itr != aa->conjuncts.end(); ++itr)
      AND(*itr);
}

VarSet TAndObj::getVars()
{
  VarSet res;
  for(set<TFormRef>::iterator itr = conjuncts.begin(); itr != conjuncts.end(); ++itr)
    res |= (*itr)->getVars();
  return res;
}

TFormRef TAndObj::negate()
{
  TOrObj* p = new TOrObj;
  for(set<TFormRef>::iterator itr = conjuncts.begin(); itr != conjuncts.end(); ++itr)
    p->disjuncts.insert((*itr)->negate());
  return p;
}


void TOrObj::write(bool flag) 
{ 
  if (disjuncts.size() == 0) { SWRITE("false"); return; }
  flag = false;
  SWRITE("["); 
  for(set<TFormRef>::iterator i = disjuncts.begin(); i != disjuncts.end(); ++i)
  {  
    if (i != disjuncts.begin()) 
      SWRITE(" \\/ "); 
    (*i)->write(flag); 
    }
  SWRITE("]");     
}

void TOrObj::OR(TFormRef a) 
{ 
  if (a->constValue() == FALSE)  return;
  if (a->constValue() == TRUE) { disjuncts.clear(); disjuncts.insert(a); return; }
  TOrRef aa = asa<TOrObj>(a); 
  if (aa.is_null())
    disjuncts.insert(a); 
  else
    for(set<TFormRef>::iterator itr = aa->disjuncts.begin(); itr != aa->disjuncts.end(); ++itr)
      OR(*itr);
}

VarSet TOrObj::getVars()
{
  VarSet res;
  for(set<TFormRef>::iterator itr = disjuncts.begin(); itr != disjuncts.end(); ++itr)
    res |= (*itr)->getVars();
  return res;
}

TFormRef TOrObj::negate()
{
  TAndObj* p = new TAndObj;
  for(set<TFormRef>::iterator itr = disjuncts.begin(); itr != disjuncts.end(); ++itr)
    p->conjuncts.insert((*itr)->negate());
  return p;
}


void FindEquations::action(TAtomObj* p) { if (p->relop == EQOP && !p->F->isConstant()) res.insert(p); }
void FindEquations::action(TAndObj* p) 
{ 
  for(set<TFormRef>::iterator i = p->conjuncts.begin(); i != p->conjuncts.end(); ++i)
    this->actOn(*i); 
}


TConstRef constInEqToBoolConst(int LHSsign, int relop)
{
  int lt = 1, et = 2, gt = 4;
  if (((lt & relop) && LHSsign < 0) || ((et & relop) && LHSsign == 0) || ((gt & relop) && LHSsign > 0))
    return new TConstObj(TRUE);
  else
    return new TConstObj(FALSE);
}
// Polymorphic function object for making copies of formula objects (removing occurences of trueAtom)
class CopyFormula : public TFPolyFun
{
  TAtomRef trueAtom;
public:
  TFormRef res;
  CopyFormula(TAtomRef _ta = NULL) { trueAtom = _ta; }
  virtual void action(TConstObj* p) { res = p;  }
  virtual void action(TAtomObj* p) 
  {
    if (p == trueAtom.vpval()) { res =  new TConstObj(TRUE); }
    else { TAtomObj *q = new TAtomObj(*(trueAtom->F->M)); q->relop = p->relop; *(q->F) = *(p->F); res = q; }
  }
  virtual void action(TAndObj* p) 
  { 
    TAndRef C = new TAndObj;
    for(set<TFormRef>::iterator i = p->conjuncts.begin(); i != p->conjuncts.end(); ++i)
    { 
      this->actOn(*i); 
      TConstRef tf = asa<TConstObj>(res);
      if (tf.is_null() || tf->value != TRUE)
	C->AND(res); 
    }
    if (C->size() == 0) { res =  new TConstObj(TRUE); }
    else res = C;
  }
};

TFormRef copyAndRemove(TFormRef F, TAtomRef A) { CopyFormula CF(A); CF(F); return CF.res; }

TQBRef TQBObj::deconflict(VarSet V)
{
  return NULL;
}

bool equals(TAtomRef A, TAtomRef B)
{
  if (A->getRelop() != B->getRelop()) return false;
  if (A->getFactors()->numFactors() != B->getFactors()->numFactors()) return false;
  int t = OCOMP(A->getFactors()->getContent(),B->getFactors()->getContent());
  if (t != 0) { return false; }
  FactObj::factorIterator itrA= A->getFactors()->factorBegin();
  FactObj::factorIterator itrB= B->getFactors()->factorBegin();
  while(itrA != A->getFactors()->factorEnd())
  {
    if (itrA->second != itrB->second) return false;
    if (itrA->first < itrB->first || itrB->first < itrA->first) return false;
    ++itrA;
    ++itrB;
  }
  return true;

}
}//end namespace tarski
