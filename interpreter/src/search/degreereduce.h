#ifndef _DEGREEREDUCE_
#define _DEGREEREDUCE_

#include "rewrite.h"

namespace tarski {


/******************************************************************
 * Parentage's for different rewrite operators
 ******************************************************************/
class DegreeReduceObj : public ParentageObj
{
  QAndRef Fp; // Formula that was subsituted into
  VarSet V;
  VarKeyedMap<int> M;  
  VarKeyedMap<int> PP; 
public:
  DegreeReduceObj(QAndRef _Fp, VarSet _V, VarKeyedMap<int> &_M, VarKeyedMap<int> &_PP)
    : Fp(_Fp), V(_V), M(_M), PP(_PP) { }
  void write();
  QAndRef predecessor() { return Fp; }
};


  
class GeneralDegreeReduce
{
private:
  // These are filled in by a call to exec.
  VarKeyedMap<int> M;  // M[xi] = m, means we can sub x^m --> x
  VarKeyedMap<int> PP; // PP[xi] = s, s /= 0 means xi^e appears as a factor in a <,>,<=,>= atom with e odd
                         // s times.  This is a problem if M[xi] ends up being even.  
  VarSet V,QVars;
  
public:
  /* IMPORTANT! It is assumed that T has been *normalized*.  Specifically that
     1. P /= 0 means P is a single polynomial and the exponent is 1
     2. in an "=" each factor has multiplicity 1 
     3. the content in each atom is one
     4. multiplicities on factors are all 1s or 2s.

     sub(Y,F) where Y = {xi^mi --> xi,...}

     * sub(Y, A1 /\ ... /\ Ak) = sub(Y,A1) /\ ... /\ sub(Y,Ak) /\ {xi >= 0 | xi^mi-->xi \in Y and mi even}
     * sub(Y,A) = ...
          if A is xi /= 0 then xi /= 0   [note: see #1 above]
          if A is xi^e * p1 * ... * pr = 0 then xi * sub(Y,p1) *...* sub(Y,pr) = 0 [note: see #2 above]
          if A is xi^e * p1^e1 * ... * pr^er sigma 0 then
             if e and mi have the same parity (where xi^mi-->xi \in Y) or e even and mi odd
               then xi^e * sub(Y,p1)^e1 * ... * sub(Y,pr)^er sigma 0
             else [This is the e odd and mi even case]
               if F ==> xi >= 0 (resp xi <= 0) 
                  return xi (resp -1*xi) * sub(Y,p1)^e1 * ... * sub(Y,pr)^er sigma 0

           OTHERWISE ... we would have to split into xi >= 0 and xi <= 0 cases.  It will
           take more thought to figure out when/if we would want to do that!                   
  */             
  TFormRef exec(QAndRef T);
  ParentageRef makeParentage(QAndRef target)
  {
    return new DegreeReduceObj(target,V,M,PP);
  }
};
}
#endif

