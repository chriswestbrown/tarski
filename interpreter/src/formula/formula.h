#ifndef _CWBFORMULA_
#define _CWBFORMULA_

#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>

#include "../formrepconventions.h"
#include "../poly/sacMod.h"
#include "../poly/factor.h"
#include "../tarski.h"

namespace tarski {

extern int developer;

typedef unsigned long long uint64;

/******************************************************
 * TForm
 ******************************************************/
const int TF_ERROR = 0, TF_CONST = 1, TF_ATOM = 2, TF_EXTATOM = 3,
  TF_AND = 4, TF_OR = 5, TF_QB = 6;

class TFPolyFun;

class TFormObj;
typedef GC_Hand<TFormObj> TFormRef;

class TFormObj : public GC_Obj
{
public:
  virtual ~TFormObj() { }
  virtual PolyManager* getPolyManagerPtr() { return 0; }
  virtual void write(bool flag = false) = 0; // writes to the current saclib context!
  virtual void apply(TFPolyFun &F) = 0;
  virtual VarSet getVars() = 0;
  virtual TFormRef negate() { return 0; } // returns a new object!
  virtual int constValue() { return -1; } // return TRUE or FALSE if an obvious synonym, e.g. empty AND
  virtual TFormRef renameVars(VarKeyedMap<VarSet> &M) { return NULL; }
  virtual int getTFType() const { return TF_ERROR; }
};

// asa:  makes casting easier.  Ex: asa<TAndObj>(p)
template<class T>
  T* asa(const TFormRef& p) { return p.is_null() ? NULL : dynamic_cast<T*>(&(*p)); }

 template<class T>
   T* TFasa(const TFormRef& p) { return p.is_null() ? NULL : dynamic_cast<T*>(&(*p)); }

/******************************************************
 * TConst
 ******************************************************/
class TConstObj : public TFormObj
{
public:
  int value;
  TConstObj(int x) : value(x) { }
  virtual void write(bool flag = false) 
  { 
    if (value == TRUE) SWRITE("true"); 
    else if (value == FALSE) SWRITE("false");
    else { SWRITE("BCONST("); IWRITE(value); SWRITE(")"); } 
  }
  void apply(TFPolyFun &F);
  VarSet getVars() { return VarSet(); }
  TFormRef negate() { return new TConstObj(logicalNegation(value)); }
  int constValue() { return value; }
  TFormRef renameVars(VarKeyedMap<VarSet> &M) { return new TConstObj(this->value); }
  virtual int getTFType() const { return TF_CONST; }
};
typedef GC_Hand<TConstObj> TConstRef;

/******************************************************
 * TAtom
 ******************************************************/
class TAtomObj;
typedef GC_Hand<TAtomObj> TAtomRef;

class TAtomObj : public TFormObj
{
public:
  FactRef F; // i.e. the left-hand side
  int relop; // so atom is: "F relop 0"
  
  TAtomObj(PolyManager &_M) { F = new FactObj(_M); }
  TAtomObj(FactRef _F, int _relop) { F = _F; relop = _relop; }
  virtual void write(bool flag = false) {
    if (flag) { SWRITE("["); }
    F->write(); SWRITE(" "); SWRITE(relopString(relop)); SWRITE(" 0");
    if (flag) { SWRITE("]"); }
  }
  void apply(TFPolyFun &F);

  std::map<IntPolyRef,int>::iterator factorsBegin() { return F->MultiplicityMap.begin(); }
  std::map<IntPolyRef,int>::iterator factorsEnd() { return F->MultiplicityMap.end(); }

  PolyManager* getPolyManagerPtr() { return F->M; }
  VarSet getVars() { return F->getVars(); }
  int getRelop() { return relop; }
  FactRef getFactors() { return F; }
  TFormRef negate() { return new TAtomObj(F,negateRelop(relop)); }
  TFormRef renameVars(VarKeyedMap<VarSet> &M) 
  { 
    FactRef Fp = new FactObj(*(getPolyManagerPtr()),F->getContent());
    for(std::map<IntPolyRef,int>::iterator itr = factorsBegin(); itr != factorsEnd(); ++itr)
    {
      IntPolyRef p = itr->first->renameVars(M);
      int e = itr->second;
      int sn = 1;
      IntPolyRef pp = p->makePositive(sn); // must make sure these factors are "positive" polynomials
      Fp->addFactor(pp,e);
      if (sn == -1 && e%2 == 1)
        Fp->negateContent();
    }
    int newRelop = relop;
    if (Fp->signOfContent() < 0) { Fp->negateContent(); newRelop = reverseRelop(relop); }
    return new TAtomObj(Fp,newRelop);
  }

  virtual int getTFType() const { return TF_ATOM; }

  /******************************************************************
   * OrderComp is a comparator object that compares TAtomRefs
   * in a way that is reasonably efficient for use in maps, sets, etc.
   * NOTE: To function properly, this requires that all IntPolyRefs
   *       in the atoms A and B are references to canonical copies.
   ******************************************************************/
  class OrderComp
  {
  public:  
    bool operator()(const TAtomRef &A, const TAtomRef &B) const;
  };
};

bool equals(TAtomRef T, TAtomRef A);

/******************************************************
 * TExtAtom
 ******************************************************/
class TExtAtomObj;
typedef GC_Hand<TExtAtomObj> TExtAtomRef;
class TExtAtomObj : public TFormObj
{
public:
  /* represents var relop _root_rootIndex F, where (for now) we assume F has a single factor */
  VarSet var;
  int rootIndex;
  FactRef F;
  int relop;
  
  TExtAtomObj(PolyManager &_M) { var = 0; rootIndex = 0; relop = ALOP; F = new FactObj(_M); }
  TExtAtomObj(VarSet var, int relop, int rootIndex, FactRef F) 
  { 
    this->var = var;
    this->relop = relop;
    this->rootIndex = rootIndex;
    this->F = F;
  }

  virtual void write(bool flag = false) 
  { 
    if (flag) { SWRITE("["); }
    PolyManager *pM = F->getPolyManagerPtr();
    SWRITE(pM->getName(var).c_str());
    SWRITE(" ");
    SWRITE(relopString(relop));
    SWRITE(" _root_");
    IWRITE(rootIndex);
    SWRITE(" "); 
    F->write(); 
    if (flag) { SWRITE("["); }
  }

  std::map<IntPolyRef,int>::iterator factorsBegin() { return F->MultiplicityMap.begin(); }
  std::map<IntPolyRef,int>::iterator factorsEnd() { return F->MultiplicityMap.end(); }

  void apply(TFPolyFun &F);

  PolyManager* getPolyManagerPtr() { return F->M; }
  VarSet getVars() { return var + F->getVars(); }
  VarSet getLHSVar() { return var; }
  int getRootIndex() { return rootIndex; }
  int getRelop() { return relop; }
  TFormRef negate() { throw TarskiException("Negate not yet implemented for TExtAtom!"); }
  
  // truth of this ExtAtom assuming the RHS has n_less roots less than var, n_equal roots
  // equal to var (note: must be 0 or 1) and n_greater roots greater than var.
  bool detTruth(int n_less, int n_equal, int n_greater)
  {
    int index = this->getRootIndex(), relop = NOOP;
    if (index < 0) { index = n_less + n_equal + n_greater + index + 1; }
    if (n_equal > 0 && n_less == index - 1)
      relop = EQOP;
    else if (index <= n_less)
      relop = GTOP;
    else
      relop = LTOP;

    // determine truth value!
    int truth = consistentWith(relop,this->getRelop());
    return truth;
  }

  TFormRef renameVars(VarKeyedMap<VarSet> &M) 
  { 
    throw TarskiException("renameVars() not yet implemented for TExtAtom!");
    /* FactRef Fp = new FactObj(*(getPolyManagerPtr()),F->getContent()); */
    /* for(std::map<IntPolyRef,int>::iterator itr = factorsBegin(); itr != factorsEnd(); ++itr) */
    /* { */
    /*   IntPolyRef p = itr->first->renameVars(M); */
    /*   int e = itr->second; */
    /*   int sn = 1; */
    /*   IntPolyRef pp = p->makePositive(sn); // must make sure these factors are "positive" polynomials */
    /*   Fp->addFactor(pp,e); */
    /*   if (sn == -1 && e%2 == 1) */
    /* 	Fp->negateContent(); */
    /* } */
    /* int newRelop = relop; */
    /* if (Fp->signOfContent() < 0) { Fp->negateContent(); newRelop = reverseRelop(relop); } */
    /* return new TExtAtomObj(Fp,newRelop); */
  }
  virtual int getTFType() const { return TF_EXTATOM; }
};



/******************************************************
 * A few important standalones
 ******************************************************/
inline int constValue(TFormRef r) { TConstObj *p = asa<TConstObj>(r); if (p) return p->value; else return -1; }
TAtomObj* makeAtom(PolyManager &_M, IntPolyRef A, int sigma);
TAtomObj* makeAtom(FactRef A, int sigma);
inline TFormRef atom(const FactRef &A, const int &sigma) { return makeAtom(A,sigma); }
std::string toString(TFormRef t);
class ConjunctOrder { public: 
  bool operator()(TFormRef a, TFormRef b) const;
};


/******************************************************
 * TAnd
 ******************************************************/
class TAndObj;
typedef GC_Hand<TAndObj> TAndRef;
class TAndObj : public TFormObj
{
public:
  std::set<TFormRef,ConjunctOrder> conjuncts;
  typedef std::set<TFormRef,ConjunctOrder>::iterator conjunct_iterator;
  conjunct_iterator begin() { return conjuncts.begin(); }
  conjunct_iterator end() { return conjuncts.end(); }
  virtual void write(bool flag = false);
  void apply(TFPolyFun &F);
  void AND(TFormRef a);
  int size() const { return conjuncts.size(); }
  PolyManager* getPolyManagerPtr()
  {
    return conjuncts.empty() ? NULL : (*conjuncts.begin())->getPolyManagerPtr();
  }
  VarSet getVars();
  TFormRef negate();
  int constValue()
  { 
    return size() == 0 ? TRUE : -1; 
  }
  TFormRef renameVars(VarKeyedMap<VarSet> &M)
  {
    TAndRef res = new TAndObj();
    for(conjunct_iterator itr = conjuncts.begin(); itr != conjuncts.end(); ++itr)
      res->AND((*itr)->renameVars(M));
    return res;
  }
  virtual int getTFType() const { return TF_AND; }
};

/******************************************************
 * TOr
 ******************************************************/
class TOrObj;
typedef GC_Hand<TOrObj> TOrRef;

static ConjunctOrder order;

class TOrObj : public TFormObj
{
public:
  std::set<TFormRef,ConjunctOrder> disjuncts;
 TOrObj() : disjuncts(order) { }
  typedef std::set<TFormRef>::iterator disjunct_iterator;
  disjunct_iterator begin() { return disjuncts.begin(); }
  disjunct_iterator end() { return disjuncts.end(); }
  virtual void write(bool flag = false); 
  void apply(TFPolyFun &F);
  void OR(TFormRef a);
  int size() const { return disjuncts.size(); }
  VarSet getVars();
  PolyManager* getPolyManagerPtr()
  {
    return disjuncts.empty() ? NULL : (*disjuncts.begin())->getPolyManagerPtr();
  }
  TFormRef negate();
  int constValue() { return size() == 0 ? FALSE : -1; }
  TFormRef renameVars(VarKeyedMap<VarSet> &M)
  {    
    TOrRef res = new TOrObj();
    for(disjunct_iterator itr = disjuncts.begin(); itr != disjuncts.end(); ++itr)
      res->OR((*itr)->renameVars(M));
    return res;
  }
  virtual int getTFType() const { return TF_OR; }
};


/******************************************************
 * TQB - Quantifier Blocks
 ******************************************************/
class TQBObj;
typedef GC_Hand<TQBObj> TQBRef;

class TQBObj : public TFormObj
{
public:
  PolyManager *PMptr;
  int outermostBlockType;
  std::vector<VarSet> blocks; // blocks[0] is the *innermost* quantifier!
  TFormRef formulaPart;

  int numBlocks() { return blocks.size(); }
  int blockType(int i) { return (numBlocks() - i) % 2 == 1 ? outermostBlockType : quantifierNegation(outermostBlockType); }
  VarSet blockVars(int i) { return blocks[i]; }
  TFormRef getFormulaPart() { return formulaPart; }
  VarSet getFullQuantifiedVarSet()
  {
    VarSet V;
    for(int i = 0; i < numBlocks(); i++)
      V = V + blockVars(i);
    return V;
  }

  TQBObj(PolyManager* pM) { PMptr = pM; }  

  TQBObj(TQBRef QF) 
  { 
    PMptr = QF->getPolyManagerPtr();
    outermostBlockType = QF->outermostBlockType; 
    blocks = QF->blocks; 
    formulaPart = QF->formulaPart; 
  }

  TQBObj(VarSet S, int Qtype, TFormRef fp)
  {
    PMptr = fp->getPolyManagerPtr();
    formulaPart = fp;
    add(S,Qtype);
  }

  TQBObj(VarSet S, int Qtype, TFormRef fp, PolyManager *pm)
  {
    PMptr = pm;
    formulaPart = fp;
    add(S,Qtype);
  }

  void add(VarSet S, int Qtype)
  {
    if (S.none()) return;
    if (blocks.size() != 0 && outermostBlockType == Qtype) 
      blocks[blocks.size()-1] = blocks[blocks.size()-1] + (formulaPart->getVars() & S);
    else
    {
      VarSet newVars = formulaPart->getVars() & S;
      if (!newVars.isEmpty())
      {
	blocks.push_back(newVars);
	outermostBlockType = Qtype;
      }
    }
  }

  inline static VarSet renameVars(VarSet V, VarKeyedMap<VarSet> &M)
  {
    VarSet R;
    for(VarSet::iterator itr = V.begin(); itr != V.end(); ++itr)
      R = R + M[*itr];
    return R;
  }

  TQBRef renameQuantifiedVars(VarKeyedMap<VarSet> &M)
  {
    TFormRef newFormulaPart = getFormulaPart()->renameVars(M);
    TQBRef res = new TQBObj(getPolyManagerPtr());
    res->formulaPart = newFormulaPart;
    for(int i = 0; i < numBlocks(); i++)
      res->add(renameVars(blockVars(i),M),blockType(i));
    return res;
  }


  TFormRef renameVars(VarKeyedMap<VarSet> &M)
  {
    VarSet V;
    for(int i = 0; i < numBlocks(); ++i)
      V = V + blockVars(i);
    VarKeyedMap<VarSet> Mp(M);
    for(VarSet::iterator itr = V.begin(); itr != V.end(); ++itr)
      Mp[*itr] = *itr;
    TFormRef newFormulaPart = formulaPart->renameVars(Mp);
    TQBRef res = new TQBObj(getPolyManagerPtr());
    res->formulaPart = newFormulaPart;
    for(int i = 0; i < numBlocks(); ++i)
      res->add(blockVars(i),blockType(i));
    return res;
  }

  // F.deconflict(V) takes quantified formula 
  TQBRef deconflict(VarSet V);

  void writeHelper(int i, int j, int qt)
  {
    std::ostream &out = TopOutputContext(); // std::set out to Saclib's current output context
    PolyManager *pM = getPolyManagerPtr();
    if (qt == EXIST) out << "ex "; else out << "all ";
    VarSet::iterator itr = blocks[j].begin();
    out << pM->getName(*itr);
    for(++itr; itr != blocks[j].end(); ++itr) out << "," << pM->getName(*itr);
    if (i < j) { out << "["; writeHelper(i,j-1,qt == EXIST ? UNIVER : EXIST); out << "]"; }
    else
    {
      TAndRef ta = asa<TAndObj>(formulaPart); 
      TOrRef tb = asa<TOrObj>(formulaPart); 
      if ((ta.is_null() || ta->size() == 1) && (tb.is_null() || tb->size() == 1))
      {	out << "["; formulaPart->write(); out << "]"; }
      else
	formulaPart->write();
    }
  }
  void write(bool flag = false)
  {
    flag = false;
    writeHelper(0,size()-1,outermostBlockType);
  }
  void apply(TFPolyFun &F);
  int size() const { return blocks.size(); }
  VarSet getVars() { return formulaPart.is_null() ? 0 : formulaPart->getVars(); }
  PolyManager* getPolyManagerPtr()
  { 
    return PMptr;
    //  return formulaPart.is_null() ? 0 : formulaPart->getPolyManagerPtr(); 
  }
  TFormRef negate()
  {
    TQBObj* p = new TQBObj(getPolyManagerPtr());
    p->formulaPart = formulaPart->negate();
    p->blocks = blocks;
    p->outermostBlockType = quantifierNegation(outermostBlockType);
    return p;
  }
  int constValue() { return formulaPart->constValue(); }
  virtual int getTFType() const { return TF_QB; }
};


inline TAndRef operator&&(TFormRef a, TFormRef b)
{
  TAndObj *p = new TAndObj(); p->AND(a);
  p->AND(b);
  return p;
}

inline TOrRef operator||(TFormRef a, TFormRef b)
{
  TOrObj *p = new TOrObj();
  p->OR(a);
  p->OR(b);
  return p;
}

inline TOrRef mkOR(TFormRef a, TFormRef b)
{
  TOrObj *p = new TOrObj();
  p->OR(a);
  p->OR(b);
  return p;
}

inline TAndRef mkAND(TFormRef a, TFormRef b)
{
  TAndObj *p = new TAndObj();
  p->AND(a);
  p->AND(b);
  return p;
}

/******************************************************
 * Polymorphic Function Objects 
 ******************************************************/
class TFPolyFun
{
public:
  void operator()(TFormRef f) { f->apply(*this); }
  void actOn(TFormRef f) { f->apply(*this); }

  virtual void action(TConstObj* p) { throw TarskiException("TFPolyFun: action unimplemented for TConstObj!"); }
  virtual void action(TAtomObj* p) { throw TarskiException("TFPolyFun: action unimplemented for TAtomObj!"); }
  virtual void action(TExtAtomObj* p) { throw TarskiException("TFPolyFun: action unimplemented for TExtAtomObj!"); }
  virtual void action(TAndObj* p) { throw TarskiException("TFPolyFun: action unimplemented for TAndObj!"); }
  virtual void action(TOrObj* p) { throw TarskiException("TFPolyFun: action unimplemented for TOrObj!"); }
  virtual void action(TQBObj* p) { throw TarskiException("TFPolyFun: action unimplemented for TQBObj!"); }
};
inline void TConstObj::apply(TFPolyFun &F) { F.action(this); }
inline void TAtomObj::apply(TFPolyFun &F) { F.action(this); }
inline void TExtAtomObj::apply(TFPolyFun &F) { F.action(this); }
inline void TAndObj::apply(TFPolyFun &F) { F.action(this); }
inline void TOrObj::apply(TFPolyFun &F) { F.action(this); }
inline void TQBObj::apply(TFPolyFun &F) { F.action(this); }


class FindEquations : public TFPolyFun // Finds the non-constant equations in a formula
{
public:
  std::set<TAtomRef> res;
  virtual void action(TAtomObj* p);
  virtual void action(TConstObj* p) {  }
  virtual void action(TAndObj* p);
};

class GetSubFormula : public TFPolyFun // Gets subformula based on index
{
  TFormRef res;
  int failCode;
  int index;
 public:
 GetSubFormula(int i): failCode(0), index(i) { }
  int getFailCode() { return failCode; }
  TFormRef getRes() { return res; }
  virtual void action(TConstObj* p) { failCode = 1; }
  virtual void action(TAtomObj* p) { failCode = 1;}
  virtual void action(TAndObj* p) 
  { 
    if (index >= p->size()) failCode = 2; 
    else {
      TAndObj::conjunct_iterator itr = p->conjuncts.begin();
      for(int i = 0; i < index; ++i) ++itr;
      res = *itr;
    }
  }
  virtual void action(TOrObj* p) 
  {        
    if (index >= p->size()) failCode = 2; 
    else {
      TOrObj::disjunct_iterator itr = p->disjuncts.begin();
      for(int i = 0; i < index; ++i) ++itr;
      res = *itr;
    }
  }
  virtual void action(TQBObj* p) 
  { 
    if (index != 0) failCode = 2;
    else {
      res = p->formulaPart;
    }    
  }
};


TFormRef copyAndRemove(TFormRef F, TAtomRef A = NULL);
TConstRef constInEqToBoolConst(int LHSsign, int relop);
}//end namespace tarski

#endif
