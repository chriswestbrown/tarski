#ifndef _COCOA_POLICY_
#define _COCOA_POLICY_
#include "CAPolicy.h"
#include "COCOA.h"


class CocoaPolicy : public CAPolicy
{
public:
  void IPFAC(Word r, Word P, Word *s_, Word *c_, Word *L_)
  {
    GVSB["Cocoa"]->IPFAC(r,P,s_,c_,L_);
  }

  Word IPRES(Word r, Word A, Word B)
  {
    return GVSB["Cocoa"]->IPRES(r,A,B);
  }

  Word IPDSCR(Word r, Word A)
  {
    return GVSB["Cocoa"]->IPDSCR(r,A);
  }

  Word IPFACTGB(Word r, Word I, Word N)  
  {
    return GVSB["Cocoa"]->IPFACTGB(r,I,N);
  }
  Word CONSTORDTEST(Word r, Word A, Word L) 
  {
    return GVSB["Cocoa"]->CONSTORDTEST(r,A,L);
  }
  const std::string name() { return "CocoaPolicy"; }

  bool supports(const std::string &s)
  {
    return s == "IPFAC" || "IPRES" || "IPDSCR" || "IPFACTGB" || "CONSTORDTEST"; 
  }
};

#endif
