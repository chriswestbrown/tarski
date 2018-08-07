#include "deduction.h"


namespace tarski {

  class Simplifier {
  private:
    PolyManager * PM;
    const std::vector<Deduction *> * deds;
    const std::vector<std::vector<int> > * depIdxs;
    const std::map<TAtomRef, int, managerComp> * atomToDed;
  public:
  };

}
