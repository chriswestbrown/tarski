#include "multinomial.h"
#include "../poly/sacMod.h"
#include "../poly/poly.h"
#include "../tarski.h"

namespace tarski {

  std::string integerToString(const integer& k) {
    ostringstream sout;
    PushOutputContext(sout);
    IWRITE(k);
    PopOutputContext();
    return sout.str();
    }


  integer stringToInteger(const std::string &s) { // This is the most stpid ass thing ever
    istringstream sin(s + ".");
    PushInputContext(sin);
    IntPolyRef p = new IntPolyObj();
    VarContext VC; VC.addVar("x"); VarSet S = VC.getVar("x");
    p->read(VC,S);
    integer res = p->valueIfConstant();
    PopInputContext();
    return res;
  }

  
  // WARNING! I play a bit fast and lose bewteen int and integer
  //          here.  It is OK, because I only treat int's as integers
  //          when values larger than BETA would run out of memory anyway.


  integer power(integer x, integer k) {
    if (k == 0) return 1;
    if (k == 1) return x;
    integer z = power(x,IDP2(k,1));
    return IEVEN(k) ? IPROD(z,z) : IPROD(IPROD(z,z),x);
  }

  integer factorial(integer n) {
    if (ISLIST(n)) { throw TarskiException("In factorial ... argument is way too big!"); }
    integer f = 1;
    for(int i = 2; i <= n; ++i)
      f = IPROD(f,i);
    return f;
  }

  void pgenh(std::vector<integer> &A, int i, integer n,
	     std::vector< std::vector<integer> > &res) {
    if (ISLIST(n)) { throw TarskiException("In pgenh ... argument is way too big!"); }
    if (i == A.size() - 1) {
      A[i] = n;
      res.push_back(A);
    }
    else {
      for(int j = 0; j <= n; j++) {
	A[i] = IDIF(n,j);
	pgenh(A,i+1,j,res);
      }
    }
  }

  void pgen(integer n, integer k, std::vector< std::vector<integer> > &res) {
    // generate all partitions of n into k parts 
    std::vector<integer> A(k,0);
    pgenh(A,0,n,res);
  }

  integer multinomial_coefficient(integer n, const std::vector<integer> &k) {
    integer result = factorial(n);
    for(int i = 0; i < k.size(); ++i)
      result = IQ(result,factorial(k[i]));
    return result;
  }

}
