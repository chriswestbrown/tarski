#ifndef _SINGULAR_POLICY_
#define _SINGULAR_POLICY_
#include "CAPolicy.h"
#include "SINGULAR.h"


class SingularPolicy : public CAPolicy
{
public:
  void IPFAC(Word r, Word P, Word *s_, Word *c_, Word *L_)
  {
    GVSB["Singular"]->IPFAC(r,P,s_,c_,L_);
  }

  Word IPRES(Word r, Word A, Word B)
  {
    return GVSB["Singular"]->IPRES(r,A,B);
  }

  void IPFACRES(Word r, Word A, Word B, Word *s_, Word *c_, Word *L_)
  {
    return GVSB["Singular"]->IPFACRES(r,A,B,s_,c_,L_);
  }

  Word IPDSCR(Word r, Word A)
  {
    return GVSB["Singular"]->IPDSCR(r,A);
  }

  void IPFACDSCR(Word r, Word P, Word *s_, Word *c_, Word *L_)
  {
    return GVSB["Singular"]->IPFACDSCR(r,P,s_,c_,L_);
  }

  Word IPFACTGB(Word r, Word I, Word N)  
  {
    return GVSB["Singular"]->IPFACTGB(r,I,N);
  }
  Word CONSTORDTEST(Word r, Word A, Word L) 
  {
    return GVSB["Singular"]->CONSTORDTEST(r,A,L);
  }
  const std::string name() { return "SingularPolicy"; }

  bool supports(const std::string &s)
  {
    return s == "IPFAC" || "IPRES" || "IPDSCR" || "IPFACTGB" || "CONSTORDTEST"; 
  }
};

#endif
