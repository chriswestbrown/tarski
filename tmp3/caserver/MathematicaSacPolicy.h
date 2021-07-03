#ifndef _MATHEMATICASAC_POLICY_
#define _MATHEMATICASAC_POLICY_
#include "OriginalPolicy.h"
#include "MathematicaPolicy.h"


class MathematicaSacPolicy : public CAPolicy
{
  CAPolicy *mathematica, *sac;
public:
  MathematicaSacPolicy() 
  {
    sac = new OriginalPolicy;
    mathematica = new MathematicaPolicy;
  }
  void IPFAC(Word r, Word P, Word *s_, Word *c_, Word *L_)
  {
    if (r > 1)
      mathematica->IPFAC(r,P,s_,c_,L_);
    else
      sac ->IPFAC(r,P,s_,c_,L_);
  }

  Word IPRES(Word r, Word A, Word B)
  {
    if (r > 2)
      return mathematica->IPRES(r,A,B);
    else
      return sac ->IPRES(r,A,B);
  }

  Word IPDSCR(Word r, Word A)
  {
    if (r > 2)
      return mathematica->IPDSCR(r,A);
    else
      return sac ->IPDSCR(r,A);
  }

  Word IPFACTGB(Word r, Word I, Word N)  
  {
    return mathematica->IPFACTGB(r,I,N);
  }

  Word CONSTORDTEST(Word r, Word A, Word L) 
  {
    return mathematica->CONSTORDTEST(r,A,L);
  }

  const std::string name() { return "MathematicaSacPolicy"; }
  bool supports(const std::string &s)
  {
      return s == "IPFAC" || "IPRES" || "IPDSCR" || "IPFACTGB" || "CONSTORDTEST"; 
  }
  
};

#endif
