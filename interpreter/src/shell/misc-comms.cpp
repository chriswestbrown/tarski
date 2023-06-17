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

  
}
