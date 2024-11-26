#ifndef _MULTINOMIAL_
#define _MULTINOMIAL_

#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <iostream>
#include "../poly/gcword.h"
#include "../poly/sacMod.h"
#include "../GC_System/GC_System.h"

namespace tarski {

  typedef GCWord integer;

  std::string integerToString(const integer& k);
  integer stringToInteger(const std::string &s);

  integer power(integer x, integer k);

  integer factorial(integer n);

  void pgen(integer n, integer k, std::vector< std::vector<integer> > &res);

  integer multinomial_coefficient(integer n, const std::vector<integer> &k);

}
#endif
