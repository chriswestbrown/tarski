#ifndef _REWRITE_
#define _REWRITE_

#include "search.h"

namespace tarski {
  
/******************************************************************
 * Rewrite operators
 ******************************************************************/
class BasicRewrite
{
  int M_leaveOut;

public:
  enum { M_linearSpolys = 1, M_uniqueAtomWithVar = 2, M_onlyNotEQ = 4 };	  
  QueueManager *globalQM;
  BasicRewrite(QueueManager *_globalQM, int _M_leaveOut) : 
    globalQM(_globalQM), M_leaveOut(_M_leaveOut)  { }
  TFormRef refine(QAndRef target, TFQueueRef Q);
  TFormRef linSubs(QAndRef target, TAtomRef A, TFQueueRef Q, TFormRef Fs);
  TFormRef splitOnAtom(QAndRef target, TAtomRef A, TFQueueRef Q);
  TFormRef linSubsXpowK(QAndRef target, TAtomRef A, TFQueueRef Q, TFormRef Fs);
  TFormRef linearSpolys(QAndRef target, TAtomRef A, TAtomRef B, TFQueueRef Q, TFormRef Fs);
  TFormRef uniqueAtomWithVar(QAndRef target, TAndRef Fs, TFQueueRef Q);
};

/******************************************************************
 * Parentage's for different rewrite operators
 ******************************************************************/
class PSubsObj : public ParentageObj
{
  QAndRef Fp; // Formula that was subsituted into
  TAtomRef A; // Atom the substitution came from 
  Variable x; // The variable the subs came from
  char scase; // 'G' for generic, 'D' for degenerate
  int k;      // The power of k, i.e. formula was linear in x^k
public:
  PSubsObj(QAndRef _Fp, TAtomRef _A, Variable _x, char _scase, int _k = 1) :
    Fp(_Fp), A(_A), x(_x), scase(_scase), k(_k) { }
  void write()
  {
    std::cout << (scase == 'G' ? "generic" : "degenerate") 
	 << "case subsituting for " 
	 << A->getPolyManagerPtr()->getName(x);
		
    if (k > 1)
      std::cout << "^" << k;
    std::cout << " in ";
    A->write();
  }
  QAndRef predecessor() { return Fp; }
};

class PSplitObj : public ParentageObj
{
  QAndRef Fp; // Formula that was split
  TAtomRef A; // Equality that was split
  IntPolyRef p; // The factor assumed zero in this branch
public:
  PSplitObj(QAndRef _Fp, TAtomRef _A, IntPolyRef _p) :
    Fp(_Fp), A(_A), p(_p) { }
  void write()
  {
    std::cout << "branch ";
    p->write(*(A->getPolyManagerPtr()));
    std::cout << " = 0 from ";
    A->write();
  }
  QAndRef predecessor() { return Fp; }
  IntPolyRef getZeroFactor() { return p; }
};

class PSPolyObj : public ParentageObj
{
  QAndRef Fp; // Formula that was split
  TAtomRef EQ1, EQ2, EQ2p;
  VarSet x;
public:
  PSPolyObj(QAndRef _Fp, TAtomRef _EQ1, TAtomRef _EQ2, TAtomRef _EQ2p, VarSet _x)
    : Fp(_Fp), EQ1(_EQ1), EQ2(_EQ2), EQ2p(_EQ2p), x(_x) { }
  void write()
  {
    std::cout << "replaced ";
    EQ2->write();
    std::cout << " with ";
    EQ2p->write();
    std::cout << " by linear-S-poly with ";
    EQ1->write();
    std::cout << " on variable '" << EQ1->getPolyManagerPtr()->getName(x)
	      << '\'';
  }
  QAndRef predecessor() { return Fp; }
};

class PSPolyDegenerateObj : public ParentageObj
{
  QAndRef Fp; // Formula that was split
  TAtomRef EQ1, EQ2, degEQ;
  VarSet x;
public:
  PSPolyDegenerateObj(QAndRef _Fp, TAtomRef _EQ1, TAtomRef _EQ2, TAtomRef _degEQ, VarSet _x)
    : Fp(_Fp), EQ1(_EQ1), EQ2(_EQ2), degEQ(_degEQ), x(_x) { }
  void write()
  {
    std::cout << "case ";
    degEQ->write();
    std::cout << " where S-poly construction degenerates reducing ";
    EQ2->write();
    std::cout << " by ";
    EQ1->write();
    std::cout << " on variable '";
    EQ1->getPolyManagerPtr()->getName(x);
    std::cout << '\'';
  }
  QAndRef predecessor() { return Fp; }
};
  
  
class PUniqueAtomObj : public ParentageObj
{
  QAndRef Fp; // Parent formula
  VarSet x;   // the quantified variable we eliminated
  TAtomRef A; // Unique atom in Fp containing x
  int kind; // 0 means always sat at x = 0, 1 means always sat (other), 
            // 2 means non-triv branch (ldcoeff non-zero), 
            // 3 means triv branch (x = 0)
public:
  PUniqueAtomObj(QAndRef _Fp, VarSet _x, TAtomRef _A, int _kind)
    : Fp(_Fp), x(_x), A(_A), kind(_kind) { }
  void write()
  {
    PolyManager *PM = A->getPolyManagerPtr();
    std::cout << "atom ";
    A->write();
    std::cout << " is the unique occurence of " << PM->getName(x) << ".  ";
    switch(kind)
    {
    case 0: std::cout << "Always sat at " << PM->getName(x) << " = 0."; break;
    case 1: std::cout << "Always sat. (ldcoeff is always non-zero!)"; break;
    case 2: std::cout << "ldcoeff non-zero branch."; break;
    case 3: std::cout << PM->getName(x) << " = 0 branch."; break;
    }
  }
  QAndRef predecessor() { return Fp; }
};

class PAllInequationsObj : public ParentageObj
{
  QAndRef Fp; // Parent formula
  VarSet X; // the quantified variables eliminated
public:
  PAllInequationsObj(QAndRef _Fp, VarSet _x) : Fp(_Fp), X(_x) { }
  void write()
  {
    std::cout << "existentially quantified conjunction of inequations is true when none of the LHS's are nullifiable over the free variables.";
  }
  QAndRef predecessor() { return Fp; }
};



// Parentage can be
// 0 - input
// 1 - generic case substituting for variable x in atom A of parent formula PF
// 2 - degenerate case substituting for variable x in atom A of parent formula PF
// 3 - splitting case for factor p of atom A in parent formula PF
// Only QAndObj's need a parentage.

// Prints derivation of F (assuming that F is actually a QNodeObj)
void printDerivation(TFormRef F);
void printDerivationIfAnd(TFormRef F);



  
}//end namespace tarski
#endif

