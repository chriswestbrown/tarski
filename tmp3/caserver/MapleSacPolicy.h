#ifndef _MAPLESAC_POLICY_
#define _MAPLESAC_POLICY_
#include "OriginalPolicy.h"
#include "MaplePolicy.h"


class MapleSacPolicy : public CAPolicy
{
  CAPolicy *maple, *sac;
public:
  MapleSacPolicy() 
  {
    sac = new OriginalPolicy;
    maple = new MaplePolicy;
  }
  ~MapleSacPolicy() 
  {
    //std::cerr << "In ~MapleSacPolicy " << std::endl;
    delete sac;
    delete maple;
  }
  void IPFAC(Word r, Word P, Word *s_, Word *c_, Word *L_)
  {
    if (r > 1)
      maple->IPFAC(r,P,s_,c_,L_);
    else
      sac ->IPFAC(r,P,s_,c_,L_);
  }

  Word IPRES(Word r, Word A, Word B)
  {
    // Maple only has IPFACRES
    //if (r > 2)
    //  maple->IPRES(r,A,B);
    //else
      return sac ->IPRES(r,A,B);
  }

  void IPFACRES(Word r, Word A, Word B, Word *s_, Word *c_, Word *L_)
  {
    maple->IPFACRES(r,A,B,s_,c_,L_);
  }

  Word IPDSCR(Word r, Word A)
  {
    if (r > 2)
      return maple->IPDSCR(r,A);
    else
      return sac ->IPDSCR(r,A);
  }

  void IPFACDSCR(Word r, Word P, Word *s_, Word *c_, Word *L_)
  {
    maple->IPFACDSCR(r,P,s_,c_,L_);
  }

  Word IPLDCF(Word r, Word A)
  {
    return sac ->IPLDCF(r,A);
  }

  void IPFACLDCF(Word r, Word P, Word *s_, Word *c_, Word *L_)
  {
    maple->IPFACLDCF(r,P,s_,c_,L_);
  }

  Word IPFACTGB(Word r, Word I, Word N)  
  {
    return maple->IPFACTGB(r,I,N);
  }

  Word CONSTORDTEST(Word r, Word A, Word L) 
  {
    return maple->CONSTORDTEST(r,A,L);
  }

  const std::string name() { return "MapleSacPolicy"; }
  bool supports(const std::string &s)
  {
      return s == "IPFAC" || "IPRES" || "IPDSCR" || "IPFACTGB" || "CONSTORDTEST"; 
  }
  
};

#endif
