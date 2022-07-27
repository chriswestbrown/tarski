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


// Here in 2021 trying to make this a bit ore sensible!
// if quantifier-free, then {# char's in printed form} else
// for equation eq, foo(eq) = min deg_x(eq) over all quantified variables x appearing in eq
// exp = {# of quantified variables} - sum over eq's of 1/(1 + foo(eq))
// 10*20^({# of quantified variables} - ) + {# char's in printed form}
  int factorGrade(IntPolyRef p) {
    VarSet V = p->getVars();
    auto itr = V.begin();
    int d = p->degree(*itr);
    while(++itr != V.end())
      d = min(d,p->degree(*itr));
    return d;
  }
int atomEQGrade(TAtomRef A) {
  if (A->relop != EQOP) return 0;
  FactRef F = A->getFactors();
  auto itr = F->factorBegin();
  auto end_itr = F->factorEnd();
  if (itr == end_itr) { throw TarskiException("Empty equation unexpected in grade!"); }
  int r = factorGrade(itr->first);
  while(++itr != end_itr)
    r = max(r,factorGrade(itr->first));
  //  cout << "atomEQGrade : " << toString(A) << " r = " << r << endl;
  return r;
}    

  /*
    Length of a conjunction:
    sum of the lengths of the atoms

    Length of atom:  
    0 if constant   else   sum of lengths of factors + C1*(#factors-1) 

    Length of factor:
    sum over terms c*x1^e1*...*xr^er of max(1,lglg|c|)*C2^e1*...C2^er
   */
  static double MYLENGTH(Word r, Word P)
  {
    const int C2 = 4;
    if (r == 0) {
      Word l1 = IMAX(2,ILOG2(IABSF(P)));
      Word l2 =  ILOG2(l1);
      return l2;
    }
    double sum = 0.0, prod = 1.0;
    int e = 0;
    for(Word L = CINV(P); L != NIL; L = RED2(L)) {
      for(int i = SECOND(L) - e; i > 0; i--, e++)
	prod *= C2;
      sum += prod*MYLENGTH(r-1,FIRST(L));
    }
    return sum;
  }
  
  double MYLENGTH(TAtomRef A)
  {
    const int C1 = 2;
    double sum = 0.0;
    int fcount = 0;
    for(auto itr = A->factorsBegin(); itr != A->factorsEnd(); ++itr, ++fcount)
      sum += MYLENGTH(itr->first->getLevel(),itr->first->getSaclibPoly());
    if (fcount > 0)
      sum += C1*(fcount - 1);
    return sum;
  }
 
  double MYLENGTH(TAndRef F)
  {
    double sum = 0.0;
    for(auto itr = F->begin(); itr != F->end(); ++itr) {
      TAtomRef A = asa<TAtomObj>(*itr);
      if (A.is_null()) { throw TarskiException("QFR Grade: Input must be conjunction of Atoms."); }
      sum += MYLENGTH(A);
    }
    return sum;
  }


  double FACTORLENGTH(TAtomRef A)
  {
    auto PMptr = A->getPolyManagerPtr();
    double sum = 0.0;
    for(auto itr = A->factorsBegin(); itr != A->factorsEnd(); ++itr) {
      IntPolyRef p = itr->first;
      ostringstream sout;
      PushOutputContext(sout);
      p->write(*PMptr);
      PopOutputContext();
      sum += sout.str().length();
    }
    return sum;
  }
  double FACTORLENGTH(TAndRef F)
  {
    double sum = 0.0;
    for(auto itr = F->begin(); itr != F->end(); ++itr) {
      TAtomRef A = asa<TAtomObj>(*itr);
      if (A.is_null()) { throw TarskiException("QFR Grade: Input must be conjunction of Atoms."); }
      sum += FACTORLENGTH(A);
    }
    return sum;
  }
  
  
  
double SmartGradeForQEPCADv1::grade(TFormRef F, VarSet QVars)
{
  // Wrap atom in AND if needed
  TAndRef Fp = new TAndObj();
  TAtomRef A = asa<TAtomObj>(F);
  if (!A.is_null()) { Fp->AND(A); }
  else { Fp = asa<TAndObj>(F); }

  // Iterate over atoms and process
  double equalityProductPenalty = 1.0;
  double sum = 0;
  VarSet V;
  for(set<TFormRef>::iterator itr = Fp->conjuncts.begin(); itr != Fp->conjuncts.end(); ++itr)
  {
    TAtomRef A = asa<TAtomObj>(*itr);
    if (A.is_null()) { cerr << "Bug in grade!  this should be an atom!" << endl; exit(1); }
    V = V + varsInAtom(A);
    if (A->relop == EQOP) {
      sum += 1.0/(1 + atomEQGrade(A));
      int len = A->getFactors()->numFactors();
      if (len > 1)
	equalityProductPenalty *= len/2.0*1.5;
    }
  }
  VarSet R = V & QVars;
  int N = R.numElements();

  //cout << toString(F) << " : " << "N = " << N << " sum = " << sum << endl;
  
  double grd =  R.numElements() == 0 ? toString(F).length() : 1000*pow(10,N - sum)
    //    + MYLENGTH(Fp)
    + FACTORLENGTH(Fp);
    + toString(F).length()
    ;
    //cout << toString(F) << " : " << grd << endl;
    return grd*equalityProductPenalty;    
}
  /*
(qfr [ex v18,v19,v20,v5,v6[v18 > 0 /\ v19 > 0 /\ v20 > 0 /\ v6^2 + v5^2 - v19^2 = 0 /\ v6 /= 0 /\ 2 v18 m - v20 - v19 - 1 = 0 /\ 2 v5 + v20^2 - v19^2 - 1 = 0 /\ 4 v18^2 v6^2 + 2 v19^2 v5 - v19^4 - v19^2 = 0]])
  */


pair<TFormRef,double> minFormAndGrade(TFQueueRef Q, VarSet QVars, FormulaGraderRef FG)
{
  TFQueueObj *nq = Q->globalQM->currentQueue(Q->tag);
  if (nq == NULL) { 
    cerr << "Problem in minFormGrade ... this queue is FALSE!" << endl;
    exit(1);
  }
  Q = nq;

  QNodeRef minn;
  TFormRef minf = Q->getOriginal()->F;
  double ming = FG->grade(minf,QVars);
  
  for(set<QNodeRef>::iterator itr = Q->nodes.begin(); itr != Q->nodes.end(); ++itr)
  {
    QAndRef AN = asa<QAndObj>(*itr);
    if (!AN.is_null())
    {
      double mingAN = FG->grade(AN->F,QVars);
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





void MinFormFinder::process(TFQueueRef Q, FormulaGraderRef FG) { searchForMin(Q,FG); }
double MinFormFinder::getMinGrade(TFQueueRef Q) { return minGrade[Q->tag]; }
int MinFormFinder::getMinLength(TFQueueRef Q) { return minLength[Q->tag]; }
TFormRef MinFormFinder::getMinFormula(TFQueueRef Q) { return minFormula[Q->tag]; }

double MinFormFinder::getMinGrade(QAndRef A) { return minGrade[A->tag]; }

void MinFormFinder::searchForMin(QAndRef A, FormulaGraderRef FG)
{
  //  cout << "In searchForMin (AND) (tag = " << A->tag << "): "; A->write(1); cout << endl;
  if (minGrade.find(A->tag) != minGrade.end())
  {
    //cout << "value already in map!" << endl;
    return;
  } //else       cout << "value NOT already in map!" << endl;
  
  minGrade[A->tag] = FG->grade(A->F, A->QVars);
  minLength[A->tag] = A->getPrintedLength();
  minFormula[A->tag] = A->F;
  fromTtoQ[A->F->tag] = A;
}
  
  void MinFormFinder::searchForMin(QOrRef A, FormulaGraderRef FG)
  {
    //cout << "In searchForMin (OR) (tag = " << A->tag << "): "; A->write(1); cout << endl;
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
  
  void MinFormFinder::searchForMin(TFQueueRef Q, FormulaGraderRef FG)
  {
    //cout << "In searchForMin (BASE) (tag = " << Q->tag << "): "; cout << endl;
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

