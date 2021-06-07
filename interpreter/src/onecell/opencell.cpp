#include "opencell.h"

extern Word CSSP(Word a, Word b);

using namespace std;

namespace tarski {
// This is a down & dirty version ... should be correct but definitely
// not super-efficient!
bool OpenCellObj::rationalPointInCellQuery(Word PT, int level)
{
  RealAlgNumRef Q = rationalToRealAlgNum(LELTI(PT,level));

  // Base Case: level == 1
  if (level == 1)
  {
    return get(1)->a->compareToRobust(Q) < 0 && get(1)->b->compareToRobust(Q) > 0;
  }

  // Recursive Case: check that point in induced cell of level level-1.
  if (!rationalPointInCellQuery(PT,level-1)) return false;
  

  // Check lower bound
  bool lbflag = get(level)->a->compareToNegInfty() == 0;
  if (!lbflag)
  {
    RealRootIUPRef zlow = asa<RealRootIUPObj>(get(level)->a);
    int j = zlow->rootIndex();
    IntPolyRef lbp = X->partialEval(get(level)->l,PT,level-1);
    vector<RealRootIUPRef> VL = RealRootIsolateSquarefree(lbp->getSaclibPoly());
    RealAlgNumRef intervalLowerBound = VL[j-1];
    if (intervalLowerBound->compareToRobust(Q) >= 0) //!!!!!!! Changed to robust 4/23/15
      return false;
  }
 
  // Check upper bound
  bool ubflag = get(level)->b->compareToPosInfty() == 0;
  if (!ubflag)
  {
    RealRootIUPRef zhigh = asa<RealRootIUPObj>(get(level)->b);
    int j = zhigh->rootIndex();
    IntPolyRef ubp = X->partialEval(get(level)->u,PT,level-1);
    vector<RealRootIUPRef> VL = RealRootIsolateSquarefree(ubp->getSaclibPoly());
    RealAlgNumRef intervalUpperBound = VL[j-1];
    if (intervalUpperBound->compareToRobust(Q) <= 0) //!!!!!!! Changed to robust 4/23/15
      return false;
  }
  return true;
}


void OpenCellObj::debugPrint2() { debugPrint(cout,-1); }

bool OpenCellObj::rationalPointInCellQuery(Word PT)
{
  int levelCell = dimension();
  int levelPoint = LENGTH(PT);
  int k = std::min(levelCell,levelPoint);
  return rationalPointInCellQuery(PT,k);
}

Word OpenCellObj::choosePointCompletion(Word PT_)
{
  Word PT = LCOPY(PT_);
  if (PT != NIL && !rationalPointInCellQuery(PT)) return NIL;
  int levelCell = dimension();
  int levelPoint = LENGTH(PT);
  if (levelCell <= levelPoint) return PT;

  int level = levelPoint + 1;
  Word PTnew = PT;
  do {
    RealAlgNumRef intervalLowerBound = get(level)->a;
    RealAlgNumRef intervalUpperBound = get(level)->b;
    bool lbflag = get(level)->a->compareToNegInfty() == 0;
    if (!lbflag)
    {
      RealRootIUPRef zlow = asa<RealRootIUPObj>(get(level)->a);
      int j = zlow->rootIndex();
      IntPolyRef lbp = level == 1 ? get(level)->l : X->partialEval(get(level)->l,PTnew,level-1);
      vector<RealRootIUPRef> VL = RealRootIsolateSquarefree(lbp->getSaclibPoly());
      intervalLowerBound = VL[j-1];
    }
    bool ubflag = get(level)->b->compareToPosInfty() == 0;
    if (!ubflag)
    {
      RealRootIUPRef zhigh = asa<RealRootIUPObj>(get(level)->b);
      int j = zhigh->rootIndex();
      IntPolyRef ubp = level == 1 ? get(level)->u : X->partialEval(get(level)->u,PTnew,level-1);
      vector<RealRootIUPRef> VL = RealRootIsolateSquarefree(ubp->getSaclibPoly());
      intervalUpperBound = VL[j-1];
    }
    Word x = rationalPointInInterval(intervalLowerBound,intervalUpperBound);
    PTnew = CONC(PTnew,LIST1(x));
    ++level;
  }while(level <= levelCell);
  return PTnew;
}


Word OpenCellObj::moveSamplePoint(Word PT)
{
  Word PT_new = choosePointCompletion(PT);
  if (PT_new == NIL) return NIL;
  int levelCell = dimension();
  this->alpha = PT_new;

  for(int k = 1; k <= levelCell; ++k)
  {
    IntPolyRef l = get(k)->l;
    RealAlgNumRef a = get(k)->a;
    int j_l = a->compareToNegInfty() == 0 ? 0 : asa<RealRootIUPObj>(a)->rootIndex();
    IntPolyRef u = get(k)->u;
    RealAlgNumRef b = get(k)->b;
    int j_u = b->compareToPosInfty() == 0 ? 0 : asa<RealRootIUPObj>(b)->rootIndex();
    CellBoundRef B = new CellBoundObj(getVarOrder(),PT_new,k,l,j_l,u,j_u);
    setBound(k,B);
  }
  return PT_new;
}


OpenCellObj::OpenCellObj(VarOrderRef _x, Word _alpha, int dim) : CB(dim+1)
{
  X = _x;
  alpha = _alpha;
  for(int i = 1; i <= dim; ++i) CB[i] = new CellBoundObj(LELTI(alpha,i));
}

  void OpenCellObj::debugPrint(ostream &out, int n)
  {
    if (n == -1) { n = dimension(); }
    cout << "Cell bounds:" << endl;
    for(int i = n; i > 0; --i) { cout << CB[i]->toStr(getPolyManager()) << endl; }
    cout << "Defining formula:" << endl << definingFormula() << endl;
  }
}//end namespace tarski
