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
  
  string writeForQEPCADBQFF(TFormRef F, bool endWithQuit, char solFormType, VarOrderRef ord);
											    
 class EQConstRec {
   FactRef f;
   VarOrderRef ord;
   VarSet mainVar;
   int num, level;
   int degree;   // sum of squares of degrees of factors
   int tdegree;  // sum of squares of total degrees of factors
   int numTerms; // sum of squares of number of terms of factors
 public:
   EQConstRec(FactRef facts, VarOrderRef ord, int level) { // NOTE: require all facts of same level 'level'
     f = facts;
     num = f->numFactors();
     this->level = level;
     this->ord = ord;
     mainVar = ord->getMainVariable(f->factorBegin()->first);
     degree = tdegree = numTerms = 0;
     for(auto itr = f->factorBegin(); itr != f->factorEnd(); ++itr) {
       IntPolyRef p = itr->first;
       int d = p->degree(mainVar), td = p->totalDegree();
       degree += d*d;
       tdegree += td*td;
       int nt, sotd, maxlen;
       p->sizeStats(nt,sotd,maxlen);
       numTerms += nt*nt;
     }
   }
   FactRef getFactors() { return f; }
   bool operator<(const EQConstRec &r) const {
     if (level != r.level) return level < r.level;
     if (degree != r.degree) return degree < r.degree;
     if (tdegree != r.tdegree) return tdegree < r.tdegree;
     if (numTerms != r.numTerms) return numTerms < r.numTerms;
     return f->cmp(r.f) < 0;
   }
 };
   
void sortEqationalConstriantAtoms(VarOrderRef ord, TAndRef Ge, vector<FactRef> &orderedConstraints) {
  vector<EQConstRec> V;
  for(TAndObj::conjunct_iterator itr = Ge->conjuncts.begin(); itr != Ge->conjuncts.end(); ++itr) {
    TAtomRef a = asa<TAtomObj>(*itr);
    if (!a.is_null()) {
      FactRef f = new FactObj(a->getPolyManagerPtr());
      int level = -1;
      bool isMultiLevelConstraint = false;
      for(std::map<IntPolyRef,int>::iterator itr = a->factorsBegin(); itr != a->factorsEnd(); ++itr) {
	f->addFactor(itr->first,1);
	if (level == -1) level = ord->level(itr->first);
	else if (level != ord->level(itr->first)) isMultiLevelConstraint = true;
      }
      if (!isMultiLevelConstraint) {
	EQConstRec R(f,ord,level);
	V.push_back(R);
      }
    }
  }
  sort(V.begin(),V.end());
  for(int i = 0; i < V.size(); i++)
    orderedConstraints.push_back(V[i].getFactors());
}
  
// This function is really only intended for existentially quantified conjunctions
string writeForQEPCADB(TFormRef F, TFormRef &introducedAssumptions, bool endWithQuit,
		       bool trackUnsatCore, char solFormType, VarOrderRef ord)
{
  //-- STEP 1: Test that F is of the proper form!
  if (!isQuantifiedConjunctionOfAtoms(F)) 
    return writeForQEPCADBQFF(F,endWithQuit,solFormType,ord);

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
    if (!ta.is_null()) { ta->F->varStats(M); }
    else {
      // THROW ERROR 
      throw TarskiException("Error in writeForQEPCADB! Inputs in the language of extended Tarski formulas not yet handled.");
      asa<TExtAtomObj>(*itr)->F->varStats(M);
    }
  }

  // Get free var and quantified var orderings in vfv and vqv respectively
  VarSet Vs = F->getVars();
  VarSet FVars = Vs & ~(QVars);
  vector<VarSet> vqv;
  for(VarSet::iterator vitr = QVars.begin(); vitr != QVars.end(); ++vitr) vqv.push_back(*vitr);
  if (ord.is_null())
    sort(vqv.begin(),vqv.end(),M);
  else {
    ord->sort(vqv);
    std::reverse(vqv.begin(),vqv.end());
  }
  vector<VarSet> vfv;
  for(VarSet::iterator vitr = FVars.begin(); vitr != FVars.end(); ++vitr) vfv.push_back(*vitr);
  if (ord.is_null())
    sort(vfv.begin(),vfv.end(),M);
  else
    ord->sort(vfv);
    
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
  VarOrderRef finalOrder = new VarOrderObj(ptrPM);
  for(int i = vfv.size() - 1; i >= 0; --i)
    finalOrder->push_back(vfv[i]);
  for(int i = vqv.size() - 1; i >= 0; --i)
    finalOrder->push_back(vqv[i]);
    
  vector<FactRef> EQC;
  sortEqationalConstriantAtoms(finalOrder,Ge,EQC);

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
  if (EQC.size() > 0)
  {
    sout << "prop-eqn-const" << endl;
    sout << "go" << endl;
    for(int i = 0; i < EQC.size(); i++)
      sout << "eqn-const-poly " << EQC[i]->toString() << " ." << endl;
  }
  else
    sout << "go" << endl;
  // NOTE: at this point we should be finished with the normalization phase

  // for fully quantified input, we can ask for tracking unsat core
  if (vfv.size() == 0 && trackUnsatCore)
    sout << "track-unsat-core" << endl;
  
  sout << "go\ngo\n";
  if (solFormType != 0)
    sout << "sol " << solFormType << "\n";

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

  string writeForQEPCADBQFF(TFormRef F, bool endWithQuit, char solFormType, VarOrderRef ord)
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
    if (!ta.is_null()) { ta->F->varStats(M); }
    else {
      // THROW ERROR 
      throw TarskiException("Error in writeForQEPCADBQFF! Inputs in the language of extended Tarski formulas not yet handled.");
      asa<TExtAtomObj>(*itr)->F->varStats(M);
    }
  }

  // Get  var orderings in vqv respectively
  vector<VarSet> vqv;
  for(VarSet::iterator vitr = QVars.begin(); vitr != QVars.end(); ++vitr) vqv.push_back(*vitr);
  if (ord.is_null())
    sort(vqv.begin(),vqv.end(),M);
  else {
    ord->sort(vqv);
    std::reverse(vqv.begin(),vqv.end());
  }
  
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
  VarOrderRef finalOrder = new VarOrderObj(ptrPM);
  for(int i = vqv.size() - 1; i >= 0; --i)
    finalOrder->push_back(vqv[i]);
  vector<FactRef> EQC;
  sortEqationalConstriantAtoms(finalOrder,Ge,EQC);  

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
  if (EQC.size() > 0)
  {
    sout << "prop-eqn-const" << endl;
    sout << "go" << endl;
    for(int i = 0; i < EQC.size(); i++)
      sout << "eqn-const-poly " << EQC[i]->toString() << " ." << endl;
  }
  else
    sout << "go" << endl;

  sout << "go\ngo\n";
  if (solFormType != 0)
    sout << "sol " << solFormType << "\n";

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
  virtual void action(TExtAtomObj* p) 
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
  makeOrderProper(F,res);
  return res;
}

 
// Given a formula F in two variables, and two variables x and y
// return a vector of "features" we can compare on.
/*  F  values
    0  -1,1     alphabetical ... does variable x come before y
    1           degree in x of factor with max x-degree
    2           2nd place of above (or 0)
    3           3rd place of above (or 0)
    4           degree in y of factor with max y-degree
    5           2nd place of above (or 0)
    6           3rd place of above (or 0)
    7           max total degree of any term containing x in factor that's 1st-place in this cat
    8           2nd place of above (or 0)
    9           3rd place of above (or 0)
   10           max total degree of any term containing y in factor that's 1st-place in this cat
   11           2nd place of above (or 0)
   12           3rd place of above (or 0)
   13           num terms containing x in factor that's 1st place in this cat.
   14           2nd place of above (or 0)
   15           3rd place of above (or 0)
   16           num terms containing y in factor that's 1st place in this cat.
   17           2nd place of above (or 0)
   18           3rd place of above (or 0)
*/

class FactorCollector : public TFPolyFun
{
  std::set<IntPolyRef> factors;
public:
  std::set<IntPolyRef>& getFactorSet() { return factors; }
  virtual void action(TAtomObj* p)
  {
    for(auto itr = p->getFactors()->factorBegin(), eitr = p->getFactors()->factorEnd(); itr != eitr; ++itr)
      factors.insert(itr->first);
  }  
  virtual void action(TExtAtomObj* p)
  {
    for(auto itr = p->getFactors()->factorBegin(), eitr = p->getFactors()->factorEnd(); itr != eitr; ++itr)
      factors.insert(itr->first);
  }  
  virtual void action(TConstObj* p) {  }
  virtual void action(TAndObj* p) {
    for(auto itr = p->conjuncts.begin(), eitr = p->conjuncts.end(); itr != eitr; ++itr)
      this->actOn(*itr);
  }
  virtual void action(TOrObj* p) {
    for(auto itr = p->disjuncts.begin(), eitr = p->disjuncts.end(); itr != eitr; ++itr)
      this->actOn(*itr);
  }
  virtual void action(TQBObj* p) {
    this->actOn(p->getFormulaPart());    
  }
};

static vector<int> FDempty{0,0,0};
class FeatDatum
{
public:
  IntPolyRef p;
  VarKeyedMap< vector<int> > M;
  FeatDatum() : M(FDempty) { }
};

class TopThree
{
  int A[4];
  int N;
public:
  TopThree() { N = 0; }
  void add(int x) {
    A[N] = x;
    for(int i = N; i > 0 && A[i-1] < A[i]; --i)
      swap(A[i-1],A[i]);
    if (N < 3)
      N++;
  }
  int get(int i) { return i < N ? A[i] : 0; }
};


/*
(def F [ x^2 - y > 0 /\ x y - 1 < 0 /\ x^2 - y^2 - x y < 2 ])
(get-2var-features F 'x 'y) 
 */

vector<float> getFeatures2Vars(TFormRef F, VarSet x, VarSet y)
{
  FactorCollector FC;
  FC.actOn(F);
  vector<FeatDatum> A(FC.getFactorSet().size());
  int i = 0;
  for(auto itr = FC.getFactorSet().begin(), eitr = FC.getFactorSet().end(); itr != eitr; ++itr)
  {
    A[i].p = *itr;
    A[i].p->varStats(A[i].M);
    i++;
  }
  TopThree degx[3], degy[3];
  for(int i = 0; i < A.size(); i++)
    for(int j = 0; j < 3; j++)
    {
      degx[j].add(A[i].M[x][j]);
      degy[j].add(A[i].M[y][j]);
    }
  
  vector<float> res;    
  res.push_back(F->getPolyManagerPtr()->getName(x) < F->getPolyManagerPtr()->getName(y) ? -1 : +1);
  for(int j = 0; j < 3; j++)
  {
    for(int i = 0; i < 3; i++)
      res.push_back(degx[j].get(i));
    for(int i = 0; i < 3; i++)
      res.push_back(degy[j].get(i));
  }
  
  return res;
}

}//end namespace tarski
