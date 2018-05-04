/***************************************************************
 * This is Hoon's original policy, which uses his "database" and
 * the PCRESALG variable.
 ***************************************************************/
#ifndef _ORIGINAL_POLICY_
#define _ORIGINAL_POLICY_
#include "CAPolicy.h"


class OriginalPolicy : public CAPolicy
{
public:
  virtual void IPFAC(Word r, Word P, Word *s_, Word *c_, Word *L_)
  {
    ::IPFAC(r,P,s_,c_,L_);
    return;
  }

  virtual Word IPRES(Word r, Word A, Word B)
  {
    return ::IPRES(r,A,B);
    //    case SUBRES:  R = IPRESPRS(r,A,B); break; 
    //    case BEZOUT:  R = IPRESBEZ(r,A,B); break; }
  }

  virtual Word IPDSCR(Word r, Word A)
  {
    return ::IPDSCR(r,A);
    //    case SUBRES:  D = IPDSCRPRS(r,A); break; 
    //    case BEZOUT:  D = IPDSCRBEZ(r,A); break; }
  }

  virtual Word IPLDCF(Word r, Word A)
  {
    return ::PLDCF(A);
  }
  
  virtual void IPFACRES(Word r, Word A, Word B, Word *s_, Word *c_, Word *L_)
  {
    Word R = IPRES(r,A,B);
    if (R == 0) { *s_ = 0; *c_ = 0; *L_ = NIL; }
    else if (r == 1) { *s_ = ISIGNF(R); *c_ = IABSF(R); *L_ = NIL; }
    else IPFAC(r-1,R,s_,c_,L_);
  }
  virtual void IPFACDSCR(Word r, Word P, Word *s_, Word *c_, Word *L_)
  { 
    if (PDEG(P) == 1) { *s_ = 1; *c_ = 1; *L_ = NIL; return; }
    Word R = IPDSCR(r,P);
    if (R == 0) { *s_ = 0; *c_ = 0; *L_ = NIL; }
    else if (r == 1) { *s_ = ISIGNF(R); *c_ = IABSF(R); *L_ = NIL; }
    else IPFAC(r-1,R,s_,c_,L_);
  }
  virtual void IPFACLDCF(Word r, Word P, Word *s_, Word *c_, Word *L_)
  { 
    Word R = IPLDCF(r,P);
    if (R == 0) { *s_ = 0; *c_ = 0; *L_ = NIL; }
    else if (r == 1) { *s_ = ISIGNF(R); *c_ = IABSF(R); *L_ = NIL; }
    else IPFAC(r-1,R,s_,c_,L_);
  }

  virtual const std::string name() { return "OriginalPolicy"; }
  bool supports(const std::string &s)
  {
    return s == "IPFAC" ||  s == "IPRES" ||  s == "IPDSCR" ||  s == "IPLDCF" ||
      s == "IPFACRES" ||  s == "IPFACDSCR" ||  s == "IPFACLDCF";
  }

};

#endif
