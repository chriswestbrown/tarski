#include "../formula/linearSubs.h"
#include "../formula/formmanip.h"
#include "../formula/writeForQE.h"
#include "../formula/formula.h"
#include "rewrite.h"
#include "qfr.h"
#include "degreereduce.h"

using namespace std;

namespace tarski {
  
  static void recon(TAtomRef A, VarKeyedMap<int> &M, VarKeyedMap<int> &PP) {
    for(auto itr = A->factorsBegin(); itr != A->factorsEnd(); ++itr) {
      IntPolyRef p = itr->first;
      if (p->isVar()) {
	switch(A->getRelop()) {
	case LTOP: case GTOP: case LEOP: case GEOP: 
	  if (itr->second % 2 == 1)
	    PP[p->getVars()] += 1;
	  break;
	default:
	  ;
	}}
      else
	p->gcdOfVariableExponentsAll(M);
    }
  }

  static void recon(TAndRef F, VarKeyedMap<int> &M, VarKeyedMap<int> &PP) {
    for(auto itr = F->begin(); itr != F->end(); ++itr) {
      TAtomRef A = asa<TAtomObj>(*itr);
      if (A.is_null()) {
	cout << endl;
	A->write();
	cout << endl;
	throw TarskiException("Only conjunctions of atoms handled right now!");
      }      
      recon(A,M,PP);
    }
  }

  // rewrite the saclib polynomial
  static Word rewrite(Word r, Word A, vector<VarSet> &levelToVar, VarKeyedMap<int> &M, VarKeyedMap<int> &PP)
  {
    if (r == 0 || A == 0) return A;
    Word Brev = NIL;
    VarSet X = levelToVar[r];
    int m = M[X] > 1 && PP[X] <= 1 ? M[X] : 1;
    for(Word Ap = A; Ap != NIL; Ap = RED2(Ap)) {
      Word e = FIRST(Ap);
      Word C = SECOND(Ap);
      Brev = COMP2(rewrite(r-1,C,levelToVar,M,PP),e/m,Brev); 
    }
    return INV(Brev);
  }
  
  static IntPolyRef rewrite(IntPolyRef p, VarKeyedMap<int> &M, VarKeyedMap<int> &PP)
  {
    vector<VarSet> levelToVar(1);  // levelToVar[0] is never used.
    VarSet V = p->getVars();
    for(auto itr = V.begin(); itr != V.end(); ++itr)
      levelToVar.push_back(*itr);
    Word B = rewrite(p->getLevel(),p->getSaclibPoly(),levelToVar,M,PP);
    return new IntPolyObj(p->getLevel(),B,V);
  }
  
  static TFormRef rewrite(TAtomRef A, VarKeyedMap<int> &M, VarKeyedMap<int> &PP)
  {
    auto PMptr = A->getPolyManagerPtr();
    FactRef F = new FactObj(PMptr);
    int sigma = A->getRelop();
    
    for(auto itr = A->factorsBegin(); itr != A->factorsEnd(); ++itr) {
      IntPolyRef p = itr->first;
      int e = itr->second;
      if (p->isVar()) {
	VarSet X = p->getVars();
	if (M[X] > 1 && PP[X] == 0) {	  
	  switch(sigma) {
	  case LTOP: case GTOP: case LEOP: case GEOP: 
	    F->addFactor(p,e);
	    break;
	  default:
	    F->addFactor(p,e); // unchanged
	  }}
	else if (M[X] > 1 && PP[X] == 1) {
	  //BEGIN subtle rewriting for equisatisfiability!
	  switch(sigma) {
	  case LTOP: case GTOP:
	    if (e % 2 == 0 || M[X] % 2 == 1)
	      F->addFactor(p,e); // unchanged
	    else {
	      sigma = NEOP;
	      F->addFactor(p,e); // unchanged
	    }
	    break;
	  case LEOP: case GEOP: 
	    if (e % 2 == 0 || M[X] % 2 == 1)
	      F->addFactor(p,e); // unchanged
	    else
	      return new TConstObj(TRUE);
	    break;
	  default:
	    F->addFactor(p,e); // unchanged
	  }
	  //END subtle rewriting for equisatisfiability!
	}
	else {
	  F->addFactor(p,e); // unchanged
	}}
      else
	F->addMultiple(rewrite(p,M,PP),e);
    }
    return makeAtom(F,sigma);
  }
  
  static TFormRef rewrite(TAndRef F, VarSet V, VarKeyedMap<int> &M, VarKeyedMap<int> &PP)
  {
    TAndRef res = new TAndObj();
    for(auto itr = F->begin(); itr != F->end(); ++itr) {
      TAtomRef A = asa<TAtomObj>(*itr);
      if (A.is_null()) {
	cout << endl;
	A->write();
	cout << endl;
	throw TarskiException("Only conjunctions of atoms handled right now!");
      }      
      res->AND(rewrite(A,M,PP));
    }
    auto PMptr = F->getPolyManagerPtr();
    for(auto itr = V.begin(); itr != V.end(); ++itr) {
      if (M[*itr] > 1 && PP[*itr] <= 1 && M[*itr] % 2 == 0)
	res->AND(makeAtom(*(PMptr),PMptr->getCannonicalCopy(new IntPolyObj(*itr)),GEOP));
    }
    return res;
  }

  /* IMPORTANT! It is assumed that T has been *normalized*.  Specifically that
     1. P /= 0 means P is a single polynomial and the exponent is 1
     2. in an "=" each factor has multiplicity 1 
     3. the content in each atom is one
     4. multiplicities on factors are all 1s or 2s.

     sub(Y,F) where Y = {xi^mi --> xi,...}

     * sub(Y, A1 /\ ... /\ Ak) = sub(Y,A1) /\ ... /\ sub(Y,Ak) /\ {xi >= 0 | xi^mi-->xi \in Y and mi even}
     * sub(Y,A) = ...
          if A is xi /= 0 then xi /= 0   [note: see #1 above]
          if A is xi^e * p1 * ... * pr = 0 then xi * sub(Y,p1) *...* sub(Y,pr) = 0 [note: see #2 above]
          if A is xi^e * p1^e1 * ... * pr^er sigma 0 then
             if e and mi have the same parity (where xi^mi-->xi \in Y) or e even and mi odd
               then xi^e * sub(Y,p1)^e1 * ... * sub(Y,pr)^er sigma 0
             else [This is the e odd and mi even case]
               if F ==> xi >= 0 (resp xi <= 0) 
                  return xi (resp -1*xi) * sub(Y,p1)^e1 * ... * sub(Y,pr)^er sigma 0

           OTHERWISE ... we would have to split into xi >= 0 and xi <= 0 cases.  It will
           take more thought to figure out when/if we would want to do that!                   
  */             
 TFormRef GeneralDegreeReduce::exec(QAndRef T) {
    // Recon to find eligible variables
    TAtomRef A = asa<TAtomObj>(T->F);
    TAndRef F = asa<TAndObj>(T->F);
    if (!A.is_null())
      recon(A,M,PP);
    else if (!F.is_null())
      recon(F,M,PP);
    else {
	cout << endl;
	T->F->write();
	cout << endl;
      throw TarskiException("Only atoms and conjunctions of atoms handled right now!");
    }
    auto PMptr = T->getPolyManagerPtr();
    V = T->getVars(); // V is VarSet instance variable
    QVars = T->QVars;
    VarSet FVars = V - QVars;

    // If we are using this for rewriting, we can only do this for quantified variables!
    // So set M[X] = 1 for any free variable X.
    for(auto vitr = FVars.begin(); vitr != FVars.end(); ++vitr)
      M[*vitr] = 1;

    int count = 0;
    for(auto vitr = QVars.begin(); vitr != QVars.end(); ++vitr)
      if (M[*vitr] > 1 && PP[*vitr] <= 1)
	count++;

    if (count == 0) { return T->F; }
    
    // for debugging
    if (verbose && count > 0) {
      cout << "--------------" << endl;
      for(auto vitr = V.begin(); vitr != V.end(); ++vitr)
	cout << PMptr->getName(*vitr) << " : " << M[*vitr] << " " << PP[*vitr] << endl;
      cout << "--------------" << endl;
    }
    
    // analyze variables with M[x] > 1 but PP[x] /= 0
    
    if (F.is_null()) { F = new TAndObj(); F->AND(A); }
    TFormRef res =  rewrite(F,V,M,PP);
    TFormRef resn = defaultNormalizer->normalize(res);

    //debugging
    if (verbose && count > 0) {
      cout << "Produced: ";
      resn->write();
      cout << endl;
    }
    
    return resn;
  }

  void DegreeReduceObj::write() 
  {
    auto PMptr = Fp->getPolyManagerPtr();
    cout << "Substitute:";
    for(auto vitr = V.begin(); vitr != V.end(); ++vitr)
      if (M[*vitr] > 1 && PP[*vitr] <= 1) {
	const string &name = PMptr->getName(*vitr);
	cout << name << "^" << M[*vitr]  << "-->" << name;
	if (PP[*vitr] == 1)
	  cout << "[equisat]";
      }
  }


} //tarski
