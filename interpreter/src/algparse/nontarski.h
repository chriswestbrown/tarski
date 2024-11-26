#ifndef _NON_TARSKI__
#define _NON_TARSKI__
#include "treemanip.h"
#include "../poly/polymanager.h"
#include "bexpr.h"

namespace tarski {
  bool hasNonConstDenom(TarskiRef T, PolyManager* pM);
  BExpRef toBExp(TarskiRef T, PolyManager* pM);      
}
#endif
