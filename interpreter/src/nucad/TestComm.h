#ifndef _TEST_COMM_
#define _TEST_COMM_
#include "property.h"
#include "goal.h"
#include "termbase.h"
#include "termqueue.h"
#include "../poly/factor.h"
#include "nucadcell.h"
#include "testsamplepointmanager.h"
#include "treesamplepointmanager.h"
#include "NuCADCellType.h"
#include "../shell/einterpreter.h"
#include <algorithm>
//#include "rule.h"

namespace tarski {


  class TestComm : public EICommand
  {
  public:
  TestComm(NewEInterpreter* ptr) : EICommand(ptr) { }

    VarOrderRef argToVarOrder(SRef obj, PolyManager* PMptr);

    GCWord argToRationalPoint(SRef obj);

    SRef execute(SRef input, vector<SRef> &args); 

    string testArgs(vector<SRef> &args) { return ""; }
    string doc() { return "TO APPEAR"; }
    string usage() { return "TO APPEAR"; }
    string name() { return "test-comm"; }
  };
  
}

#endif

// (test-comm 'level1 [ x - 1 ])
