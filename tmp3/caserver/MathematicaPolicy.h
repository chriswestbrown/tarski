#ifndef _MATHEMATICA_POLICY_
#define _MATHEMATICA_POLICY_
#include "CAPolicy.h"
#include "MATHEMATICA.h"


class MathematicaPolicy : public CAPolicy
{
public:
  void IPFAC(Word r, Word P, Word *s_, Word *c_, Word *L_)
  {
    GVSB["Mathematica"]->IPFAC(r,P,s_,c_,L_);
  }

  Word IPRES(Word r, Word A, Word B)
  {
    return GVSB["Mathematica"]->IPRES(r,A,B);
  }

  Word IPDSCR(Word r, Word A)
  {
    return GVSB["Mathematica"]->IPDSCR(r,A);
  }

  Word IPFACTGB(Word r, Word I, Word N)  
  {
    return GVSB["Mathematica"]->IPFACTGB(r,I,N);
  }
  Word CONSTORDTEST(Word r, Word A, Word L) 
  {
    return GVSB["Mathematica"]->CONSTORDTEST(r,A,L);
  }
  const std::string name() { return "MathematicaPolicy"; }

  bool supports(const std::string &s)
  {
    return s == "IPFAC" || "IPRES" || "IPDSCR" || "IPFACTGB" || "CONSTORDTEST"; 
  }
};

#endif
