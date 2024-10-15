#include "misc-comms.h"
#include "writeForQE.h"
#include "utils.h"
#include "qepcad-inter/qepcad-api.h"  

using namespace std;

namespace tarski {

  SRef CommGetFreeVars::execute(SRef input, vector<SRef> &args) 
  {
    TFormRef F = args[0]->tar()->getValue();
    VarSet V = getFreeVars(F);
    LisRef L = new LisObj();
    if (V.isEmpty())
      return L;
    for(auto itr = V.begin(); itr != V.end(); ++itr)
      L->push_back(new SymObj(getPolyManagerPtr()->getName(*itr)));
    return L;
  }


  class SolutionDimensionCallback : public QepcadAPICallback {
    int Norig;
  public:
    SolutionDimensionCallback(int Norig) { this->Norig = Norig; }
    SRef operator()(QepcadCls &Q) {
      int k = SOLSETDIM(Q.GVPC,Q.GVNFV);
      NumRef res = new NumObj(Norig == Q.GVNFV ? k : k + (Norig - Q.GVNFV));
      return res;
    }
  };
  
  SRef CommSolutionDimension::execute(SRef input, vector<SRef> &args) 
  {
    TFormRef F = args[0]->tar()->getValue();
    VarSet V = getFreeVars(F);
    int N = V.numElements();

    SRef res;
    try {
      // Do basic normalization to get rid of boolean constants, which qepcad
      // doesn't understand.
      RawNormalizer R(defaultNormalizer);
      R(F);
      TFormRef T = R.getRes();

      // Bail out if this is already a constant
      int tmp = T->constValue();
      if (tmp != -1) {
	res = new NumObj(tmp == 0 ? -1 : N);
      }
      else {
	SolutionDimensionCallback f(N);
	TFormRef assumptions;
	std::string script = naiveButCompleteWriteForQepcad(T,assumptions);
	res = qepcadAPICall(script,f);
      }
      return res;
    }
    catch(TarskiException e)
    {
      return new ErrObj(e.what());
    }
    return new SObj();
  }


  SRef CommDiscriminant::execute(SRef input, vector<SRef> &args) {
    AlgRef A = args[0]->alg();
    IntPolyRef p = A->getVal();
    SymRef sx = args[1]->sym();
    Variable x = this->getPolyManagerPtr()->getVar(sx->getVal());
    int deg = p->degree(x);
    if (deg <= 0)
      return new ErrObj("Command " + name() + " requires polynomial of positive degree in given variable.");

    IntPolyRef d = deg == 1 ? new IntPolyObj(1) : this->getPolyManagerPtr()->discriminant(p,x);
    AlgRef D = new AlgObj(d,*(this->getPolyManagerPtr()));
    return D;
  }
  
    SRef CommSubDiscSeq::execute(SRef input, vector<SRef> &args) {
    AlgRef A = args[0]->alg();
    IntPolyRef p = A->getVal();
    SymRef sx = args[1]->sym();
    Variable x = this->getPolyManagerPtr()->getVar(sx->getVal());
    int deg = p->degree(x);
    LisRef res = new LisObj();
    if (deg <= 0)
      return new ErrObj("Command " + name() + " requires polynomial of positive degree in given variable.");
    else if (deg == 0) {
      res->push_back(new AlgObj(new IntPolyObj(1),*(this->getPolyManagerPtr())));
    }
    else {
      VarSet V = p->getVars();
      Word r = V.numElements();
      Word i = V.positionInOrder(x);
      Word A = PMMVPO(r,p->getSaclibPoly(),i);
      Word Ap = IPDMV(r,A);
      Word L = IPPSC(r,A,Ap);
      for(Word Lp = L; Lp != NIL; Lp = RED(Lp)) {
	Word PSC = IPQ(r-1,FIRST(Lp),PLDCF(A));
	IntPolyRef sdc = new IntPolyObj(r-1,PSC,V - x);
	res->push_back(new AlgObj(sdc,*(this->getPolyManagerPtr())));
      }
    }
    return res;
  }

  SRef CommNullifySys::execute(SRef input, vector<SRef> &args) {
    // Get polynomial p and varset S
    AlgRef A = args[0]->alg();
    IntPolyRef p = A->getVal();
    LisRef vars = args[1]->lis();
    VarSet S;
    for(int i = 0; i < vars->length(); ++i)  {
      if (vars->get(i)->type() != _sym) { 
	return new ErrObj("Function 'nullify-sys' received as 2nd argument a list that contains non-symbol element '" + vars->get(i)->toStr() + "'."); 
      }
      S = S + interp->PM->getVar(vars->get(i)->sym()->val);
    }

    // (nullify-sys [ a x y^2 + y x - (a + y)] '(x))
    
    return new TarObj(nullifySys(p,S,interp->PM));
  }
  
}
