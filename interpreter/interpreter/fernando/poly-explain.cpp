#include "poly-explain.h"
#include "mono-explain.h"
#include "fern-poly-iter.h"
#include "../poly/variable.h"
#include "../formrepconventions.h"

namespace tarski {


  static short T_combine[8][8] = {
    //______|ZERO_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
    /*ZERO*/{ZERO,LTOP,EQOP,LEOP,GTOP,NEOP,GEOP,ALOP},
    /*LTOP*/{LTOP,LTOP,ZERO,LTOP,ZERO,LTOP,ZERO,LTOP},
    /*EQOP*/{EQOP,ZERO,EQOP,EQOP,ZERO,ZERO,EQOP,EQOP},
    /*LEOP*/{LEOP,LTOP,EQOP,ZERO,ZERO,ZERO,ZERO,LEOP},
    /*GTOP*/{GTOP,ZERO,ZERO,ZERO,GTOP,GTOP,GTOP,GTOP},
    /*NEOP*/{NEOP,LTOP,ZERO,ZERO,GTOP,NEOP,ZERO,NEOP},
    /*GEOP*/{GEOP,ZERO,EQOP,ZERO,GTOP,ZERO,GEOP,GEOP},
    /*ALOP*/{ALOP,LTOP,EQOP,LEOP,GTOP,NEOP,GEOP,ALOP}
  };



  VarKeyedMap<int> combine(const VarKeyedMap<int> &v1, const VarKeyedMap<int> &v2, const std::vector<Variable> &vars) {
    VarKeyedMap<int> M(ALOP);
    for (std::vector<Variable>::const_iterator vIter = vars.begin(); vIter != vars.end(); ++vIter){
      Variable v = *vIter;
      short s1 = v1.get(v);
      short s2 = v2.get(v);
      short sFin = T_combine[s1][s2];
      M[v] = sFin;
    }
    return M;
  }

  /*
    Requires a fresh FernPolyIter
  */
  VarKeyedMap<int> polyStrict(const VarKeyedMap<int> &knownSigns, const short &sign, FernPolyIter &F, bool& success){
    VarKeyedMap<int> M(ALOP);
    std::vector<VarKeyedMap <int> > candidates;
    std::vector<Variable> vars = F.getAllVars();
    success = true;
    bool selected = false;
    do {
      VarKeyedMap<int> tmp;
      tmp = termStrict(knownSigns, F, sign, success);
      if (success) {
        candidates.push_back(tmp);
        selected = true;
        tmp = termNonStrict(knownSigns, F, sign, success); //In this case no need to check success since termStrict succeeded!
      }
      if (!success) {
        success = true;
        tmp = termNonStrict(knownSigns, F, sign, success);
        if (success == false) {
          return M;
        }
      }

      M = combine(M, tmp, vars);

    }  while (F.next() != F.end());

    if (!selected) {
      success = false;
      return M;
    }
    else {

      int bestIdx = -1;
      int bestScore = 0;
      int currIdx = 0;
      for (std::vector<VarKeyedMap<int>>::iterator iter = candidates.begin(); iter != candidates.end(); ++iter) {
        int tmpScore = polyScoreFun(M, *iter, vars);
        if (bestScore > tmpScore || bestIdx == -1  ) {
          bestScore = tmpScore;
          bestIdx = currIdx;
        }
        currIdx++;
      }

      VarKeyedMap<int>  strengthen = candidates[bestIdx];
      M = combine(M, strengthen, vars);
    }
    return M;
  }

  /*
    Requires a fresh FernPolyIter
  */
  VarKeyedMap<int> polyNonStrict(const VarKeyedMap<int> &knownSigns, const short &sign, FernPolyIter &F, bool& success){
    VarKeyedMap<int> M(ALOP);
    success = true;
    std::vector<Variable> vars = F.getAllVars();
    do {
      VarKeyedMap<int> tmp;
      tmp = termNonStrict(knownSigns, F, sign, success);
      if (!success) {
        return M;
      }
      M = combine(M, tmp, vars);
    } while (F.next() != F.end());
    return M;
  }



  /*
    Requires a fresh FernPolyIter
    Try to prove polyStrict
    If not, try to find at exactly one term which is NEOP, and verify all others are EQOP
  */
  VarKeyedMap<int> polyNotEqual(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, bool& success){
    VarKeyedMap<int> tmp(ALOP);
    VarKeyedMap<int> res(ALOP);
    std::vector<VarKeyedMap <int> > candidates;
    std::vector<Variable> allVars = F.getAllVars();
    success = true;


    short tgtSgn = NOOP;
    do {
      success = true;
      tmp = termStrict(knownSigns, F, GTOP, success);
      if (success) {
        F.next();
        tgtSgn = GTOP;
        res = combine(res, tmp, allVars);
        break;
      }
      success = true;
      tmp = termStrict(knownSigns, F, LTOP, success);
      if (success) {
        F.next();
        tgtSgn = LTOP;
        res = combine(res, tmp, allVars);
        break;
      }
      success = true;
      tmp = termEqual(knownSigns, F, success);
      if (!success) {
        return res;
      }
      res = combine(res, tmp, allVars);
    } while (tgtSgn == NOOP && F.next() != F.end());

    while (F.next() != F.end()) {
      VarKeyedMap<int> tmp(ALOP);
      tmp = termStrict(knownSigns, F, tgtSgn, success);
      if (!success) tmp = termEqual(knownSigns, F, success);
      if (!success) return res;
      res = combine(res, tmp, allVars);
    }


    return res;



  }


  /*
    Requires a fresh FernPolyIter
  */
  VarKeyedMap<int> polyEqual(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, bool& success){
    VarKeyedMap<int> M(ALOP);
    success = true;
    std::vector<Variable> vars = F.getAllVars();

    do {
      VarKeyedMap<int> tmp;
      tmp = termEqual(knownSigns, F, success);
      if (!success) {
        return M;
      }
      M = combine(M, tmp, vars);
    } while (F.next() != F.end());
    return M;
  }


  void writePolyExp(const VarKeyedMap<int> &signs, const short &sign, FernPolyIter &F, const std::vector<Variable> &vars,   VarContext &VC){
    bool success = true;
    VarKeyedMap<int> M(ALOP);
    if (sign == LTOP || sign == GTOP) {
      M = polyStrict(signs, sign, F, success);
    }
    else if (sign == LEOP || sign == GEOP) {
      M = polyNonStrict(signs, sign, F, success);
    }
    else if (sign == NEOP) {
      M = polyNotEqual(signs, F, success);
    }
    else if (sign == EQOP) {
      M = polyEqual(signs, F, success);
    }
    else if (sign == ALOP) {
      //std::cout << "ALOP detected - nothing to do" << std::endl;
      return;
    }
    else {
      //std::cout << "NOOP detected - what are you doing?" << std::endl;
      return;
    }
    if (!success) {
      //std::cout << "Not succesful!\n";
    }
    writeExp(M, vars, VC);
  }


  int  polyScoreFun(const VarKeyedMap<int> &oldMap, const VarKeyedMap<int> &candidate, const std::vector<Variable> &vars) {

    int score = 0;

    //For all the variables in an explanation
    for (std::vector<Variable>::const_iterator vIter = vars.begin(); vIter != vars.end(); ++vIter) {

      int oneScore = 0;
      //If the oldMap has LEOP, and the candidate has LTOP, its not that bad. If the oldMap has ALOP and the candidate has LTOP, that's pretty bad. This line of code takes into account the score of the oldMap for the current variable.
      oneScore += signScores[candidate.get(*vIter)] - signScores[oldMap.get(*vIter)];

      //If the oldMap has LTOP, and the candidate has ALOP, that ALOP doesn't give us anything. This takes that into occurence.
      if (oneScore < 0) oneScore = 0;

      score += oneScore;
    }
    return score;
  }

  /*
    Given a sign, calls the correct polyExplain function
  */
  VarKeyedMap<int> select(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, const short &sign, bool &success) {
    if (sign == LTOP || sign == GTOP) {

      return polyStrict(knownSigns, sign, F, success);
    }
    else if (sign == GEOP || sign == LEOP) {

      return polyNonStrict(knownSigns, sign, F, success);
    }
    else if (sign == EQOP) {

      return polyEqual(knownSigns, F, success);
    }
    else {

      return polyNotEqual(knownSigns, F, success);
    }

  }


}

