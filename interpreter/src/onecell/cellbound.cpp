#include "cellbound.h"

using namespace std;

namespace tarski {
CellBoundObj::CellBoundObj(Word rat) 
{ 
  alphaAsRN = rat;
  alpha = rationalToRealAlgNum(rat); 
  l = u = NULL;
  a = new NegInftyObj();
  b = new PosInftyObj();
}

CellBoundObj::CellBoundObj(Word rat, Word pb, IntPolyRef p) 
{ 
  alphaAsRN = rat;
  alpha = rationalToRealAlgNum(rat); 
  l = u = p;
  
  vector<RealRootIUPRef> L = RealRootIsolateSquarefree(pb);
  
  int k = 0, t; while(k < L.size() && (t = L[k]->compareToRobust(alpha)) < 0) { ++k; }
  if (k > 0) { a = L[k-1]; } else { a = new NegInftyObj(); l = NULL; }
  if (k < L.size())
  {
    if (t == 0) { cerr << "CellBoundObj(rat,p,pb) Error!  This case not covered yet!" << endl; }
    b = L[k];
  }
  else
  { b = new PosInftyObj(); u = NULL; }
}

CellBoundObj::CellBoundObj(VarOrderRef X, Word alpha, int k, IntPolyRef l, Word j_l, IntPolyRef u, Word j_u)
{
  Word rat = LELTI(alpha,k);
  this->alphaAsRN = rat;
  this->alpha = rationalToRealAlgNum(rat); 
  this->l = l;
  this->u = u;
  
  if (!l.is_null()) {
    IntPolyRef ap = X->partialEval(l,alpha,k-1);
    vector<RealRootIUPRef> L_a = RealRootIsolateSquarefree(ap->sP);
    if (L_a.size() < j_l) {
      cerr << "Error in CellBoundObj Constructor (lower)!" << endl;
    }
    RealAlgNumRef av = L_a[j_l-1];
    this->a = av;
  }
  else { this->a = new NegInftyObj(); }

  if (!u.is_null()) {
    IntPolyRef bp = X->partialEval(u,alpha,k-1);
    vector<RealRootIUPRef> L_b = RealRootIsolateSquarefree(bp->sP);
    if (L_b.size() < j_u) {
      cerr << "Error in CellBoundObj Constructor! (upper)" << endl;
    }
    RealAlgNumRef bv = L_b[j_u-1];
    this->b = bv;
  }
  else { this->b = new PosInftyObj(); }
  
}


CellBoundObj::CellBoundObj(CellBoundRef b1, CellBoundRef b2) // these *must* share the same "alpha" value!
{
  alphaAsRN = b1->alphaAsRN;
  alpha = b1->alpha;
  if (b1->a->compareToBrittle(b2->a) < 0) { l = b2->l; a = b2->a; } else { l = b1->l; a = b1->a; }
  if (b2->b->compareToBrittle(b1->b) < 0) { u = b2->u; b = b2->b; } else { u = b1->u; b = b1->b; }
}

string CellBoundObj::toStr()
{
  ostringstream sout;
  PushOutputContext(sout);
  RNWRITE(alphaAsRN);
  PopOutputContext();
  return a->toStr() + " <= " + sout.str() + " <= " + b->toStr();
}

string CellBoundObj::toStr(PolyManager* ptrPM)
{
  string lbp = "", ubp = "";
  if (!l.is_null()) { 
    ostringstream sout;
    PushOutputContext(sout); 
    l->write(*ptrPM);
    PopOutputContext();
    lbp = sout.str() + ":";
  }
  if (!u.is_null()) { 
    ostringstream sout;
    PushOutputContext(sout); 
    u->write(*ptrPM);
    PopOutputContext();
    ubp = sout.str() + ":";
  }
  ostringstream sout;
  PushOutputContext(sout);
  RNWRITE(alphaAsRN);
  PopOutputContext();
  return lbp + a->toStr() + " <= " + sout.str() + " <= " + ubp + b->toStr();
}

string CellBoundObj::toStrFormula(PolyManager* ptrPM, VarSet mainVariable)
{
  string lbp = "", ubp = "";
  if (!l.is_null()) { 
    int index = asa<RealRootIUPObj>(a)->rootIndex();
    ostringstream sout;
    PushOutputContext(sout); 
    SWRITE("_root_"); IWRITE(index); SWRITE(" ");
    l->write(*ptrPM);
    PopOutputContext();
    lbp = ptrPM->getName(mainVariable) + " > " + sout.str();
  }
  if (!u.is_null()) { 
    int index = asa<RealRootIUPObj>(b)->rootIndex();
    ostringstream sout;
    PushOutputContext(sout); 
    SWRITE("_root_"); IWRITE(index); SWRITE(" ");
    u->write(*ptrPM);
    PopOutputContext();
    ubp = ptrPM->getName(mainVariable) + " < " +sout.str();
  }
  if (lbp == "" && ubp == "") return "";
  if (lbp == "") return ubp;
  if (ubp == "") return lbp;
  return lbp + " /\\ " + ubp;
}

}//end namespace tarski
