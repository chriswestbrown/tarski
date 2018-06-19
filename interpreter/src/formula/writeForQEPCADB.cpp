#include <sys/types.h>
#include <time.h>
#include <iostream>
#include <stack>
#include "../algparse/algparse.h"
#include "formmanip.h"
#include "formula.h"
#include "normalize.h"
#include "linearSubs.h"
#include "writeForQE.h"
#include <algorithm>
#include "../search/qfr.h"

using namespace std;

namespace tarski {

/********************************************************
 * We assume we're given a formula F that is
 * - is in strong prenex form, i.e. ex x1...xk[G]
 * - has a single existentially quantified block
 * - formula is already normalized (say to level 4).
 * This function will give a somewhat down-and-dirty
 * QEPCADB formulation.  It will try to
 * 1) split out assumptions
 * 2) declare equational constraints.
 ********************************************************/
class VarStatsCmp : public VarKeyedMap< vector<int> >
{
  // See poly/factor.h for info on how varStats populates 
  // populates the vector of stats
  PolyManager *ptrPM;
public:
  VarStatsCmp(vector<int> &nullvalue, PolyManager *pPM) :  
    VarKeyedMap< vector<int> >(nullvalue) { ptrPM = pPM; }
  bool operator()(const VarSet &a, const VarSet &b)
  {
    vector<int> &v = (*this)[a];
    vector<int> &w = (*this)[b];
    for(int i = 0; i < 3; ++i)
      if (v[i] < w[i]) return true;
      else if (v[i] > w[i]) return false;
    return ptrPM->getName(a) < ptrPM->getName(b);
  }
};

  string writeForQEPCADBQFF(TFormRef F, bool endWithQuit);

// This function is really only intended for existentially quantified conjunctions
string writeForQEPCADB(TFormRef F, TFormRef &introducedAssumptions, bool endWithQuit, bool trackUnsatCore)
{
  //-- STEP 1: Test that F is of the proper form!
  if (!isQuantifiedConjunctionOfAtoms(F)) 
    return writeForQEPCADBQFF(F,endWithQuit);

  TQBRef Fb = asa<TQBObj>(F);
  if (Fb.is_null() || Fb->size() != 1)
    throw TarskiException("Error in writeForQEPCADB: input is not existentially quantified prenex!");

  TFormRef Fp = Fb->formulaPart;
  PolyManager *ptrPM = F->getPolyManagerPtr();
  TAndRef Fs = asa<TAndObj>(Fp);
  if (Fs.is_null() && asa<TAtomObj>(Fp)) { Fs = new TAndObj();  Fs->AND(Fp); }
  if (Fs.is_null())
    throw TarskiException("Error in writeForQEPCADB: input must be an existentially quantified conjunction.");

  //-- STEP 2a: determine which variables from F appear in non-strict inequalities (i.e. =,<=,>=)
  VarSet Vns = appearsInNonStrict(F);

  
  //-- STEP 2: Get the quantified variable set & write F = A /\ ex ... [G]
  VarSet QVars = Fb->blocks[0];
  vector<TFormRef> V = factorDisjointVars(Fs,QVars);
  TAndRef A = new TAndObj(), G = new TAndObj();
  for(unsigned int i = 0; i < V.size(); ++i)
    if ((QVars & V[i]->getVars()).any())
      G->AND(V[i]);
    else
      A->AND(V[i]);

  //-- STEP 3: Choose a "promising" variable order
  // Gather stats ov variables
  vector<int> NV(3); NV[0]=NV[1]=NV[2]=0;
  VarStatsCmp M(NV,ptrPM);
  TAndRef tmpA = new TAndObj; tmpA->AND(G); tmpA->AND(A); 
  for(TAndObj::conjunct_iterator itr = tmpA->conjuncts.begin(); itr != tmpA->conjuncts.end(); ++itr)
  {
    TAtomRef ta = asa<TAtomObj>(*itr);
    ta->F->varStats(M);
  }

  // Get free var and quantified var orderings in vfv and vqv respectively
  VarSet Vs = F->getVars();
  VarSet FVars = Vs & ~(QVars);
  vector<VarSet> vqv;
  for(VarSet::iterator vitr = QVars.begin(); vitr != QVars.end(); ++vitr) vqv.push_back(*vitr);
  sort(vqv.begin(),vqv.end(),M);
  vector<VarSet> vfv;
  for(VarSet::iterator vitr = FVars.begin(); vitr != FVars.end(); ++vitr) vfv.push_back(*vitr);
  sort(vfv.begin(),vfv.end(),M);

  //-- STEP 4: Split G into Ge (equations) and Gi (inequalities and inequations)
  TAndRef Ge = new TAndObj();
  TAndRef Gi = new TAndObj();
  bool allStrict = true; // if strict inequalities, we can do full dimensional only!
  for(TAndObj::conjunct_iterator itr = G->conjuncts.begin(); itr != G->conjuncts.end(); ++itr)
  {
    int relop;
    if (asa<TAtomObj>(*itr) && (relop = asa<TAtomObj>(*itr)->relop) == EQOP)
      Ge->AND(*itr), allStrict = false;
    else
      Gi->AND(*itr), (allStrict = allStrict && !relopIsNonStrict(relop));
  }

  //-- STEP 5: Sort Ge elements by 1) increasing level, 2) increasing degree in "level" variable,
  //           3) increasing total degree, 4) increasing number of terms


  // Set countNonStrict to the number of quantified variables *in the current order* that
  // must be treated as non-strict.  This means that the the first countNonStrict variables
  // must be treated as non-strict, but the remaining variables can be treated as strict.
  bool allStrictSoFar = true;
  int countNonStrict = 0;
  for(int i = 0; i < vqv.size(); i++)
  {
    VarSet x = vqv[i];
      allStrictSoFar = allStrictSoFar && !(Vns & x).any();
      if (!allStrictSoFar)
	countNonStrict++;
  }

  // Write QEPCADB input
  ostringstream sout;
  sout << "[]" << endl;
  sout << '(';
  for(int i = vfv.size() - 1; i >= 0; --i)
    sout << ptrPM->getName(vfv[i]) << ',';
  for(int i = vqv.size() - 1; i >= 0; --i)
    sout << ptrPM->getName(vqv[i]) << (i == 0 ? "" : ",");
  sout << ')' << endl;
  sout << vfv.size() << endl;
  for(int i = vqv.size() - 1; i >= 0; --i, --countNonStrict)
    sout << "(" << (countNonStrict > 0 ? 'E' : 'F') << " " << ptrPM->getName(vqv[i]) << ")";
  PushOutputContext(sout); G->write(); PopOutputContext();
  sout << "." << endl;
  if (A->size() > 0)
  {
    introducedAssumptions = A;
    sout << "assume ";
      PushOutputContext(sout); A->write(); PopOutputContext();
    sout << endl;
  }
  //  cout << "measure-zero-error" << endl;
  //   sout << "cell-choice-bound (SR,LD,HL)\n"; // T E S T ! ! !
  if (Ge->size() > 0)
  {
    sout << "prop-eqn-const" << endl;
    sout << "go" << endl;
    for(TAndObj::conjunct_iterator itr = Ge->conjuncts.begin(); itr != Ge->conjuncts.end(); ++itr)
    {
      TAtomRef a = asa<TAtomObj>(*itr);
      if (a.is_null() || a->F->numFactors() != 1) continue;
      sout << "eqn-const-poly ";
      PushOutputContext(sout); a->F->MultiplicityMap.begin()->first->write(*(a->F->M)); PopOutputContext();
      sout << "." << endl;
    }
  }
  else
    sout << "go" << endl;
  // NOTE: at this point we should be finished with the normalization phase

  // for fully quantified input, we can ask for tracking unsat core
  if (vfv.size() == 0 && trackUnsatCore)
    sout << "track-unsat-core" << endl;
  
  if (false)
    sout << "finish" << endl;
  else
    sout << "\ngo\ngo\nsol T\n";
  if (false) // Just for some debugging
  {
    VarSet Vs = F->getVars();
    for(VarSet::iterator vitr = Vs.begin(); vitr != Vs.end(); ++vitr)
    {
      sout << ptrPM->getName(*vitr) << ' '
	   << M[*vitr][0] << ' '
	   << M[*vitr][1] << ' '
	   << M[*vitr][2] << endl;
    }
  }

  if (endWithQuit) { sout << "quit" << endl; }
  return sout.str();
}

  string writeForQEPCADBQFF(TFormRef F, bool endWithQuit)
{
  //-- STEP 1: Test that F is of the proper form!
  if (!isAndAndAtoms(F))
    throw TarskiException("Error in writeForQEPCADBQFF: input must be a quantifier-free conjunction.");
  
  TAndRef Fs = asa<TAndObj>(F);
  if (Fs.is_null()) (Fs = new TAndObj())->AND(F);
  PolyManager *ptrPM = F->getPolyManagerPtr();
  
  //-- STEP 2: Get the quantified variable set & write F = A /\ ex ... [G]
  VarSet QVars = Fs->getVars();
  vector<TFormRef> V = factorDisjointVars(Fs,QVars);
  TAndRef G = Fs;
  
  //-- STEP 3: Choose a "promising" variable order
  // Gather stats ov variables
  vector<int> NV(3); NV[0]=NV[1]=NV[2]=0;
  VarStatsCmp M(NV,ptrPM);
  TAndRef tmpA = new TAndObj; tmpA->AND(G);
  for(TAndObj::conjunct_iterator itr = tmpA->conjuncts.begin(); itr != tmpA->conjuncts.end(); ++itr)
  {
    TAtomRef ta = asa<TAtomObj>(*itr);
    ta->F->varStats(M);
  }

  // Get  var orderings in vqv respectively
  vector<VarSet> vqv;
  for(VarSet::iterator vitr = QVars.begin(); vitr != QVars.end(); ++vitr) vqv.push_back(*vitr);
  sort(vqv.begin(),vqv.end(),M);

  //-- STEP 4: Split G into Ge (equations) and Gi (inequalities and inequations)
  TAndRef Ge = new TAndObj();
  TAndRef Gi = new TAndObj();
  for(TAndObj::conjunct_iterator itr = G->conjuncts.begin(); itr != G->conjuncts.end(); ++itr)
  {
    if (asa<TAtomObj>(*itr) && asa<TAtomObj>(*itr)->relop == EQOP)
      Ge->AND(*itr);
    else
      Gi->AND(*itr);
  }

  //-- STEP 5: Sort Ge elements by 1) increasing level, 2) increasing degree in "level" variable,
  //           3) increasing total degree, 4) increasing number of terms
  

  // Write QEPCADB input
  ostringstream sout;
  sout << "[]" << endl;
  sout << '(';
  for(int i = vqv.size() - 1; i >= 0; --i)
    sout << ptrPM->getName(vqv[i]) << (i == 0 ? "" : ",");
  sout << ')' << endl;
  sout << vqv.size() << endl;
  PushOutputContext(sout); G->write(); PopOutputContext();
  sout << "." << endl;
  if (Ge->size() > 0)
  {
    sout << "prop-eqn-const" << endl;
    sout << "go" << endl;
    for(TAndObj::conjunct_iterator itr = Ge->conjuncts.begin(); itr != Ge->conjuncts.end(); ++itr)
    {
      TAtomRef a = asa<TAtomObj>(*itr);
      if (a.is_null() || a->F->numFactors() != 1) continue;
      sout << "eqn-const-poly ";
      PushOutputContext(sout); a->F->MultiplicityMap.begin()->first->write(*(a->F->M)); PopOutputContext();
      sout << "." << endl;
    }
  }
  else
    sout << "go" << endl;


  if (false)
    sout << "finish" << endl;
  else
    sout << "\ngo\ngo\nsol T\n";

  if (endWithQuit) { sout << "quit" << endl; }
  return sout.str();
}


class CollectFactors : public TFPolyFun
{
private:
  set<IntPolyRef>* ppolys;
public:
  CollectFactors(set<IntPolyRef> &polys) { ppolys = &polys; }
  virtual void action(TConstObj* p) { }
  virtual void action(TAtomObj* p) 
  {
    for(map<IntPolyRef,int>::iterator itrF = p->factorsBegin(); itrF != p->factorsEnd(); ++itrF)
      ppolys->insert(itrF->first);
  }
  virtual void action(TAndObj* p) 
  {
    for(TAndObj::conjunct_iterator itr = p->conjuncts.begin(); itr != p->conjuncts.end(); ++itr)
      this->actOn(*itr);
  }
  virtual void action(TOrObj* p)
  {
    for(TOrObj::disjunct_iterator itr = p->disjuncts.begin(); itr != p->disjuncts.end(); ++itr)
      this->actOn(*itr);
  }
  virtual void action(TQBObj* p) { this->actOn(p->formulaPart); }
};

set<IntPolyRef> collectFactors(TFormRef F)
{
  set<IntPolyRef> res; 
  CollectFactors CF(res);
  CF(F);
  return res;
}

vector<VarSet> getBrownVariableOrder(TFormRef F)
{
  if (!isPrenex(F)) { throw TarskiException("getBrownVariable ordering requires a prenex formula as input."); }

  vector<int> NV(3); NV[0]=NV[1]=NV[2]=0;
  VarStatsCmp M(NV,F->getPolyManagerPtr());
  set<IntPolyRef> polys = collectFactors(F);
  for(set<IntPolyRef>::iterator itr = polys.begin(); itr != polys.end(); ++itr)
    (*itr)->varStats(M);

  // Get  var orderings in vqv respectively
  VarSet vars = F->getVars();
  vector< VarSet > blocks;
  TQBRef Fq = asa<TQBObj>(F);
  if (!Fq.is_null())
  {
    for(int i = 0; i < Fq->numBlocks(); i++)
    {
      VarSet v = Fq->blockVars(i);
      blocks.push_back(v);
      vars = vars & ~v;
    }
  }
  blocks.push_back(vars);
  vector<VarSet> res;
  for(unsigned int i = 0; i < blocks.size(); i++)
  {
    vector<VarSet> vqv;
    for(VarSet::iterator vitr = blocks[i].begin(); vitr != blocks[i].end(); ++vitr) vqv.push_back(*vitr);
    sort(vqv.begin(),vqv.end(),M);
    for(unsigned int j = 0; j < vqv.size(); j++)
      res.push_back(vqv[j]);
  }
  reverse(res.begin(),res.end());
  return res;
}


}//end namespace tarski
