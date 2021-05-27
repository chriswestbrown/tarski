#ifndef _MAPLE_POLICY_
#define _MAPLE_POLICY_
#include "CAPolicy.h"
#include "MAPLE.h"


class MaplePolicy : public CAPolicy
{
public:
  void IPFAC(Word r, Word P, Word *s_, Word *c_, Word *L_)
  {
    GVSB["Maple"]->IPFAC(r,P,s_,c_,L_);
  }

  Word IPRES(Word r, Word A, Word B)
  {
    return GVSB["Maple"]->IPRES(r,A,B);
  }

  void IPFACRES(Word r, Word A, Word B, Word *s_, Word *c_, Word *L_)
  {
    GVSB["Maple"]->IPFACRES(r,A,B,s_,c_,L_);
  }
  
  Word IPDSCR(Word r, Word A)
  {
    return GVSB["Maple"]->IPDSCR(r,A);
  }

  void IPFACDSCR(Word r, Word P, Word *s_, Word *c_, Word *L_)
  {
    GVSB["Maple"]->IPFACDSCR(r,P,s_,c_,L_);
  }

  void IPFACLDCF(Word r, Word P, Word *s_, Word *c_, Word *L_)
  {
    GVSB["Maple"]->IPFACLDCF(r,P,s_,c_,L_);
  }

  Word IPFACTGB(Word r, Word I, Word N)  
  {
    return GVSB["Maple"]->IPFACTGB(r,I,N);
  }
  Word CONSTORDTEST(Word r, Word A, Word L) 
  {
    return GVSB["Maple"]->CONSTORDTEST(r,A,L);
  }
  const std::string name() { return "MaplePolicy"; }

  bool supports(const std::string &s)
  {
    return s == "IPFAC" || "IPRES" || "IPDSCR" || "IPFACTGB" || "CONSTORDTEST"; 
  }
};

#endif
