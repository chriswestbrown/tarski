#include "sample-comm.h"

namespace tarski {
  SRef SampleComm::execute(SRef input, std::vector<SRef> &args)
  {
    // must have arguments
    int N = args.size();
    if (N < 1) { return new ErrObj("sample-command requires <num> argument."); }
  
    // Get argumetn, add one to it, return
    NumRef num = args[0]->num(), one = new NumObj(1);
    if (num.is_null()) { return new ErrObj("sample-command argument not a <num>."); }
    one->add(num);
  
    return one;
  }

}
