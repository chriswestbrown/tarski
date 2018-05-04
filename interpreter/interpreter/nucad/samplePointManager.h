#ifndef _SAMPLE_POINT_MANAGER_
#define _SAMPLE_POINT_MANAGER_
#include "../onecell/varorder.h"

class SamplePointManagerObj;
typedef GC_Hand<SamplePointManagerObj> SamplePointManagerRef;

typedef int SamplePointId; // note: 0 is the SamplePointId of the origin

class SamplePointManagerObj : public GC_Obj
{
private:
  VarOrderRef varOrder;
  
public:
  SamplePointManagerObj(VarOrderRef varOrder) { this->varOrder = varOrder; }
  VarOrderRef getVarOrder() const { return varOrder; }
  PolyManager* getPolyManager() const { return varOrder->getPolyManager(); }
  // register's point (s_1,...,s_{k},P_1,...,P_n)
  SamplePointId addRationalOver(SamplePointId s, GCWord P, int k)
  {
    this->tmpP = P;
    return 1;
  }

  int polynomialSignAt(SamplePointId s, IntPolyRef p)
  {
    VarOrderRef X = getVarOrder();
    int k = X->level(p);
    return X->partialEval(p,tmpP,X->level(p))->signIfConstant();
  }

  bool isNullifiedAt(SamplePointId s, IntPolyRef p)
  {
    VarOrderRef X = getVarOrder();
    return X->isNullified(p,tmpP,X->level(p));
  }

  
private: // This is just for early test purposes!
  GCWord tmpP;  
};



#endif

