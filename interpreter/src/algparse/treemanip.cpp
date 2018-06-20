#include "treemanip.h"

namespace tarski {
void Traverse::operator()(QuantifierBlock *p) 
{ 
  for(unsigned int i = 0; i < p->vars.size(); ++i) 
    p->vars[i]->apply(*this);
  p->arg->apply(*this);
}
}

