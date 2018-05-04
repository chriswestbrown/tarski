#include "mono-explain.h"
#include "../GC_System/GC_System.h"
#include "fern-poly-iter.h"
#include "../poly/polymanager.h"
#include "../poly/variable.h"
namespace tarski {

  /*
    A wrapper for the other variant of monoStrict which takes a FernPolyIter
  */
  VarKeyedMap<int> monoStrict(const  VarKeyedMap<int> &knownSigns, FernPolyIter &F, bool& success){
    std::vector<Variable> vars = F.getVars();
    std::vector<short> exponents = F.getExponents();
    short targetSign = F.getTermSign();
    return monoStrict(knownSigns, vars, exponents, targetSign, success);
  }

  /*
    Input:
    knownSigns - a VKM with the known sign information on the monomial
    vars - the list of vars in the monomial
    exponents - the list of exponents which corresponds to the vars in the monomial (if x^2  appears, and vars[0] = x, then exponents[0] = 2)
    targetSign - the sign we want to prove on the monomial
    success - a boolean we can manipulate freely

    Return success = false, or the weakest possible explanation for the chosen term being strict as LTOP or GTOP as a VarKeyedMap. Note that the VarKeyedMap defaults unkown entries to ALOP
  */
  VarKeyedMap<int> monoStrict(const VarKeyedMap<int> &knownSigns, const std::vector<Variable> &vars, const std::vector<short> &exponents, const short &targetSign,  bool& success) {
    VarKeyedMap<int> M(ALOP);


    if (targetSign == GTOP) {
      bool correct = true; //this value alternates between true and false
      std::vector<short>::const_iterator exIter = exponents.begin();
      for (std::vector<Variable>::const_iterator vIter = vars.begin(); vIter != vars.end(); ++vIter) {

        int currSign = knownSigns.get(*vIter);

        if (currSign == GEOP || currSign == LEOP || currSign == ALOP || currSign == EQOP) {
          success = false;
          return M;
        }
        if (*exIter % 2 == 0) { //even exponent means NEOP is possible
          M[*vIter] = NEOP;
        }
        else {
          M[*vIter] = currSign;
          if (currSign == LTOP) correct = !correct; //flip sign, now our signs prove LTOP
        }

        ++exIter;
      }
      if (correct) {
        success = true;
        return M;
      }
      else {
        success = false;
        return M;
      }
    }
    else if (targetSign == LTOP) {
      bool correct = false; //this value alternates between true and false
      std::vector<short>::const_iterator exIter = exponents.begin();
      for (std::vector<Variable>::const_iterator vIter = vars.begin(); vIter != vars.end(); ++vIter) {

        int currSign = knownSigns.get(*vIter);

        if (currSign == GEOP || currSign == LEOP || currSign == ALOP || currSign == EQOP) {
          success = false;
          return M;
        }
        if (*exIter % 2 == 0) { //even exponent means NEOP is possible
          M[*vIter] = NEOP;
        }
        else {
          M[*vIter] = currSign;
          if (currSign == LTOP) correct = !correct; //flip sign, now our signs prove LTOP
        }

        ++exIter;
      }
      if (correct) {
        success = true;
        return M;
      }
      else {
        success = false;
        return M;
      }
    }
    else {
      //Throw new TarskiException("monoStrict must be called on LTOP or GTOP")
      success = false;
      return M;
    }
  }


  VarKeyedMap<int> termStrict(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, const short &targetSign, bool& success) {
    std::vector<Variable> vars = F.getVars();
    std::vector<short> exponents = F.getExponents();
    int coefficient = F.getCoefficient();
    return termStrict(knownSigns, vars, exponents, targetSign, coefficient, success);
  }

  /*
    Input:
    knownSigns - a VKM with the known sign information on the monomial
    vars - the list of vars in the monomial
    exponents - the list of exponents which corresponds to the vars in the monomial (if x^2  appears, and vars[0] = x, then exponents[0] = 2)
    targetSign - the sign we want to prove on the monomial
    success - a boolean we can manipulate freely

    Return success = false, or the weakest possible explanation for the chosen term being strict as LTOP or GTOP as a VarKeyedMap. Note that the VarKeyedMap defaults unkown entries to ALOP
  */
  VarKeyedMap<int> termStrict(const VarKeyedMap<int> &knownSigns, const std::vector<Variable> &vars,  const std::vector<short> &exponents, const short &targetSign, const int &coefficient, bool& success) {

    short coeff = ISIGNF(coefficient);
    VarKeyedMap<int> M(ALOP);
    bool correct = true;
    if (targetSign == LTOP)
      correct = false;
    if (coeff < 0) correct = !correct;
    else if (coeff == 0) {
      success = false;
      return M;
    }
    std::vector<short>::const_iterator exIter = exponents.begin();
    for (std::vector<Variable>::const_iterator vIter = vars.begin(); vIter != vars.end(); ++vIter) {
      int currSign = knownSigns.get(*vIter);

      if (currSign == GEOP || currSign == LEOP || currSign == ALOP || currSign == EQOP) {
        success = false;
        return M;
      }
      if (*exIter % 2 == 0) { //even exponent means NEOP is possible
        M[*vIter] = NEOP;
      }
      else {
        M[*vIter] = currSign;
        if (currSign == LTOP) correct = !correct; //flip sign, now our signs prove LTOP
      }

      ++exIter;
    }
    if (correct) {
      success = true;
      return M;
    }
    else {
      success = false;
      return M;
    }
  }

  VarKeyedMap<int> termNonStrict(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, const short &targetSign, bool &success) {


    int coeff = ISIGNF(F.getCoefficient());


    std::vector<Variable> vars = F.getVars();
    std::vector<short> exponents = F.getExponents();

    VarKeyedMap<int> M(ALOP);


    //Setting correct
    bool correct = true;
    if (targetSign == GEOP || targetSign == GTOP) {
      bool correct = true; //this value alternates between true and false
    }
    else if (targetSign == LEOP || targetSign == LTOP) {
      correct = false;
    }
    else { //Invalid sign
      success = false;
      return M;
    }
    if (coeff < 0) correct = !correct;
    else if (coeff == 0) {
      success = false;
      return M;
    }

    std::vector<short>::const_iterator exIter = exponents.begin();
    for (std::vector<Variable>::const_iterator vIter = vars.begin(); vIter != vars.end(); ++vIter) {
      int currSign = knownSigns.get(*vIter);
      if (currSign == EQOP) {
        success = true;
        return termEqual(knownSigns, F, success);
      }
      else if (*exIter % 2 == 0) { //even exponent means NEOP is possible
        M[*vIter] = ALOP;
      }
      else if (currSign == ALOP || currSign == NEOP) {
        return termEqual(knownSigns, F, success); //Still a chance that this can be the solution
        //TODO: Fix Latex algorithm
      }
      else if (currSign == EQOP) {
        success = true;
        return termEqual(knownSigns, F, success);
      }
      else if (currSign == LTOP || currSign == LEOP){
        M[*vIter] = LEOP;
        correct = !correct;
      }
      else if (currSign == GTOP || currSign == GEOP){
        M[*vIter] = GEOP;
      }

      ++exIter;
    }
    if (correct) {
      success = true;
      return M;
    }
    else {
      success = false;
      return M;
    }
  }

  VarKeyedMap<int> monoNonStrict(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, const short &targetSign, bool &success) {

    std::vector<Variable> vars = F.getVars();
    std::vector<short> exponents = F.getExponents();


    VarKeyedMap<int> M(ALOP);

    //Setting correct
    bool correct = true;
    if (targetSign == GEOP) {
      bool correct = true; //this value alternates between true and false
    }
    else if (targetSign == LEOP) {
      correct = false;
    }
    else { //Invalid sign
      success = false;
      return M;
    }


    std::vector<short>::const_iterator exIter = exponents.begin();
    for (std::vector<Variable>::const_iterator vIter = vars.begin(); vIter != vars.end(); ++vIter) {
      int currSign = knownSigns.get(*vIter);

      if (*exIter % 2 == 0) { //even exponent means NEOP is possible
        M[*vIter] = ALOP;
      }
      else if (currSign == ALOP) {
        success = false;
        return M;
      }
      else if (currSign == EQOP) {
        success = true;
        return termEqual(knownSigns, F, success);
      }
      else if (currSign == LTOP){ //Get to weaken < to <=
        M[*vIter] = LEOP;
        correct = !correct;
      }
      else if (currSign == GTOP){ //Get to weaken > to >=
        M[*vIter] = GEOP;
      }
      ++exIter;
    }
    if (correct) {
      success = true;
      return M;
    }
    else {
      success = false;
      return M;
    }
  }


  VarKeyedMap<int> termNotEqual(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, bool& success) {


    std::vector<Variable> vars = F.getVars();
    VarKeyedMap<int> M(ALOP);


    for (std::vector<Variable>::const_iterator vIter = vars.begin(); vIter != vars.end(); ++vIter) {
      int currSign = knownSigns.get(*vIter);
      //Reject all which could allow = -
      if (currSign == ALOP || currSign == LEOP || currSign == GEOP || currSign == EQOP) {
        success = false;
        return M;
      }
      else { //All others are NEOP
        M[*vIter] = NEOP;
      }

    }
    success = true;
    return M;

  }


  VarKeyedMap<int> termEqual(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, bool& success) {


    std::vector<Variable> vars = F.getVars();
    VarKeyedMap<int> M(ALOP);
    if (ISIGNF(F.getCoefficient()) == 0) {success = true; return M;}

    success = false;
    for (std::vector<Variable>::const_iterator vIter = vars.begin(); vIter != vars.end(); ++vIter) {
      int currSign = knownSigns.get(*vIter);
      //Select one to be =. Note I still need an algorithm for scoring here! 
      if (currSign == EQOP && success == false){
        success = true;
        M[*vIter] = EQOP;
      }

      else { //All others are ALOP
        M[*vIter] = ALOP;
      }
    }

    return M;

  }

  /*
    Simple function to demonstrate all these algorithms appropriately (the term variants)
  */
  void selectAndWriteTerm(const VarKeyedMap<int> &signs, FernPolyIter &F, VarContext &VC){
    VarKeyedMap<int> M(ALOP);
    bool success = true;
    if (F.getTermSign() == LTOP || F.getTermSign() == GTOP)
      M = termStrict(signs, F, F.getTermSign(), success);
    else if (F.getTermSign() == LEOP || F.getTermSign() == GEOP)
      M = termNonStrict(signs, F, F.getTermSign(), success);
    else if (F.getTermSign() == NEOP) 
      M = termNotEqual(signs, F, success);
    else if (F.getTermSign() == EQOP)
      M = termEqual(signs, F, success);
    else if (F.getTermSign () == ALOP) {
      //std::cout << "ALOP detected - nothing to do" << std::endl;
      return;
    }
    else if (!success) {
      std::cout << "Failed to prove a sign" << std::endl;
      throw TarskiException("Mono Sign Failure");
      return;
    }
    else {
      std::cout << "NOOP detected - what are you doing?" << std::endl;
      return;
    }

    writeExp(M, F.getVars(), VC);

  }



  void writeExp(const VarKeyedMap<int> &signs, const std::vector<Variable> &vars, VarContext &VC){
    for (std::vector<Variable>::const_iterator vIter = vars.begin(); vIter != vars.end(); ++vIter) {
      std::cout << VC.getName(*vIter) << ": " << FernPolyIter::numToRelop(signs.get(*vIter)) << "   ";
    }
    std::cout << std::endl;

  }

  void writeExp(const VarKeyedMap<int> &signs, const VarSet &vars, VarContext &VC) {
    for (VarSet::iterator itr = vars.begin(); itr != vars.end(); ++itr) {
      std::cout << VC.getName(*itr) << ": " << FernPolyIter::numToRelop(signs.get(*itr)) << "    ";
    }
    std::cout << std::endl;

  }
}
