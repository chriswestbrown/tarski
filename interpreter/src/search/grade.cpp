#include "grade.h"
using namespace std;

namespace tarski {
VarSet varsInAtom(TAtomRef A)
{
  VarSet V;
  for(map<IntPolyRef,int>::iterator ftr = A->F->MultiplicityMap.begin(); ftr != A->F->MultiplicityMap.end(); ++ftr)
    V = V + ftr->first->getVars();
  return V;
}

double SimpleGradeForQEPCAD::grade(TFormRef F, VarSet QVars)
{
  int ne = 0;
  VarSet V;
  TAtomRef A = asa<TAtomObj>(F);
  if (!A.is_null())
  {
    V = varsInAtom(A);
    if (A->relop == EQOP) ++ne;
  }
  else
  {
    TAndRef Fp = asa<TAndObj>(F);
    for(set<TFormRef>::iterator itr = Fp->conjuncts.begin(); itr != Fp->conjuncts.end(); ++itr)
    {
      TAtomRef A = asa<TAtomObj>(*itr);
      if (A.is_null()) { cerr << "Bug in grade!  this should be an atom!" << endl; exit(1); }
      V = V + varsInAtom(A);
      if (A->relop == EQOP) ++ne;
    }
  }
  VarSet R = V & QVars;
  double gv = 1;
  for(VarSet::iterator vtr = R.begin(); vtr != R.end(); ++vtr)
    gv *= 20;
  return R.numElements() == 0 ? toString(F).length() : (1000*(10*gv - ne) + toString(F).length()); 
}



pair<TFormRef,double> minFormAndGrade(TFQueueRef Q, VarSet QVars, FormulaGrader &FG)
{
  TFQueueObj *nq = Q->globalQM->currentQueue(Q->tag);
  if (nq == NULL) { 
    cerr << "Problem in minFormGrade ... this queue is FALSE!" << endl;
    exit(1);
  }
  Q = nq;

  QNodeRef minn;
  TFormRef minf = Q->getOriginal()->F;
  double ming = FG.grade(minf,QVars);
  
  for(set<QNodeRef>::iterator itr = Q->nodes.begin(); itr != Q->nodes.end(); ++itr)
  {
    QAndRef AN = asa<QAndObj>(*itr);
    if (!AN.is_null())
    {
      double mingAN = FG.grade(AN->F,QVars);
      if (mingAN < ming) { ming = mingAN; minf = AN; minn = AN; }
    }
    else
    {
      // I guess I need to start grading in a way that's sensitive to the DAG structure
      // ... though I don't understand why it would really matter!
      QOrRef ON = asa<QOrObj>(*itr);
      double gON = 0;
      TOrRef D = new TOrObj;
      for(set<TFQueueRef>::iterator qtr = ON->disjuncts.begin(); qtr != ON->disjuncts.end(); ++qtr)
      {
	pair<TFormRef,double> rres = minFormAndGrade(*qtr,QVars,FG);
	D->OR(rres.first);
	gON = max(rres.second,gON);
      }
      if (gON < ming) { ming = gON; minf = D; minn = ON; }
    }
  }

  pair<TFormRef,double> res;
  res.first = minf;
  res.second = ming;
  return res;
}





void MinFormFinder::process(TFQueueRef Q, FormulaGrader &FG) { searchForMin(Q,FG); }
double MinFormFinder::getMinGrade(TFQueueRef Q) { return minGrade[Q->tag]; }
int MinFormFinder::getMinLength(TFQueueRef Q) { return minLength[Q->tag]; }
TFormRef MinFormFinder::getMinFormula(TFQueueRef Q) { return minFormula[Q->tag]; }

double MinFormFinder::getMinGrade(QAndRef A) { return minGrade[A->tag]; }

void MinFormFinder::searchForMin(QAndRef A, FormulaGrader &FG)
{
  //cout << "In searchForMin (tag = " << A->tag << "): "; A->write(1); cout << endl;
  if (minGrade.find(A->tag) != minGrade.end())
  {
    //cout << "value already in map!" << endl;
    return;
  } //else       cout << "value NOT already in map!" << endl;
  
  minGrade[A->tag] = FG.grade(A->F, A->QVars);
  minLength[A->tag] = A->getPrintedLength();
  minFormula[A->tag] = A->F;
  fromTtoQ[A->F->tag] = A;
}
  
  void MinFormFinder::searchForMin(QOrRef A, FormulaGrader &FG)
  {
    TOrRef sf = new TOrObj;
    int length = 0;
    double grd = 0;
    for(set<TFQueueRef>::iterator itr = A->disjuncts.begin(); itr != A->disjuncts.end(); ++itr)
    {
      searchForMin(*itr,FG);
      grd = max(grd,minGrade[(*itr)->tag]);
      length += 2 + minLength[(*itr)->tag];
      TFormRef minF = minFormula[(*itr)->tag];
      sf->OR(minF);
    }
    minGrade[A->tag] = grd;
    minLength[A->tag] = length;
    minFormula[A->tag] = sf;
  }
  
  void MinFormFinder::searchForMin(TFQueueRef Q, FormulaGrader &FG)
  {
    double grd = -1;
    int length = 0;
    TFormRef sf;
    for(set<QNodeRef>::iterator itr = Q->nodes.begin(); itr != Q->nodes.end(); ++itr)
    {
      QAndRef fA = asa<QAndObj>(*itr);
      if (!fA.is_null())
	searchForMin(fA,FG);
      else
	searchForMin(asa<QOrObj>(*itr),FG);
      
      if (grd == -1 || grd > minGrade[(*itr)->tag])
      {
	grd = minGrade[(*itr)->tag];
	length = minLength[(*itr)->tag];
	sf = minFormula[(*itr)->tag];
      }
    }
    minGrade[Q->tag] = grd;
    minLength[Q->tag] = length;
    minFormula[Q->tag] = sf;
  }
}//end namespace tarski

