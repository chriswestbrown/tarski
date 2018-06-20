#ifndef _STORE_
#define _STORE_

#include <iostream>
#include <sstream>
#include "../tarskisysdep.h" /* defines pathToMaple variable */
#include "../poly/sacMod.h"
#include "../formula/normalize.h"
#include "../onecell/memopolymanager.h"
#include "../smtlib/readSMTRetFormula.h"
#include "../shell/qepcad-inter/qepcad-inter.h"
#include "../poly/factor.h"

namespace tarskiapi {

  
  class Store
  {
  private:
    std::vector<tarski::FactRef> IdxToFactor;
    std::map<tarski::FactRef,int,tarski::FactObj::lt> FactorTpIdx;
    tarski::Normalizer *NORM;
    tarski::PolyManager *PM;

  public:
    Store(tarski::Normalizer *NORM, tarski::PolyManager *PM) :
      NORM(NORM), PM(PM) { IdxToFactor.push_back(NULL); }

    /************************************************************
     * Input:  smtlibPoly - a string with a let-free smtlib representation of a polynomial.
     *         sigma - an int representing a relop int the standard tarski way.
     * Output: either a v vector of length 1, or a vector of length 2k+1, k > 0.
     *         if v has length 1, then v[0] = 0 represents that the input atom is equivalent
     *         to false (e.g. [ x^2 < 0 ]) and v[0] = 1 represents that the input atom is
     *         equivalent to true (e.g. [ x^2 >= 0 ]).  When v has length greater than 1, 
     *         then consecutive values encode an atom, where v[2*i] is the index of the
     *         polynomial that is the LHS, and v[2*i+1] is the relop (the RHS is always 0).
     *************************************************************/
    std::vector<int> registerAtom(const std::string &smtlibPoly, int sigma);

    std::string getString(const std::vector<int> &q);    
    int isSAT(const std::vector<int> &q, std::string &witness);    
    int isSATUC(const std::vector<int> &q, std::string &witness, std::vector<int> &unsatCore);    

    string relopStringSMTLIB(int r);
    int SMTLIBRelopInt(const string& str);
    
    
    tarski::PolyManager* getPolyManagerPtr() { return PM; }
    tarski::TAtomRef getAtom(int id, int sigma);
  };

} // END namespace tarskiapi
  
#endif
