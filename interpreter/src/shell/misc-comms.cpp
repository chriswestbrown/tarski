#include "misc-comms.h"
#include "writeForQE.h"
#include "utils.h"

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


}
