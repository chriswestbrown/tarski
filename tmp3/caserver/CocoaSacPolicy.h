#ifndef _COCOASAC_POLICY_
#define _COCOASAC_POLICY_
#include "OriginalPolicy.h"
#include "CocoaPolicy.h"


class CocoaSacPolicy : public CAPolicy
{
  CAPolicy *cocoa, *sac;
public:
  CocoaSacPolicy() 
  {
    sac = new OriginalPolicy;
    cocoa = new CocoaPolicy;
  }
  void IPFAC(Word r, Word P, Word *s_, Word *c_, Word *L_)
  {
    if (r > 1)
      cocoa->IPFAC(r,P,s_,c_,L_);
    else
      sac ->IPFAC(r,P,s_,c_,L_);
  }

  Word IPRES(Word r, Word A, Word B)
  {
    if (r > 2)
      return cocoa->IPRES(r,A,B);
    else
      return sac ->IPRES(r,A,B);
  }

  Word IPDSCR(Word r, Word A)
  {
    if (r > 2)
      return cocoa->IPDSCR(r,A);
    else
      return sac ->IPDSCR(r,A);
  }

  Word IPFACTGB(Word r, Word I, Word N)  
  {
    return cocoa->IPFACTGB(r,I,N);
  }

  Word CONSTORDTEST(Word r, Word A, Word L) 
  {
    return cocoa->CONSTORDTEST(r,A,L);
  }

  const std::string name() { return "CocoaSacPolicy"; }
  bool supports(const std::string &s)
  {
      return s == "IPFAC" || "IPRES" || "IPDSCR" || "IPFACTGB" || "CONSTORDTEST"; 
  }
  
};

#endif
