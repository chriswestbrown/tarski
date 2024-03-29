#include "poly-explain.h"
#include "mono-explain.h"
#include "fern-poly-iter.h"
#include "../../poly/variable.h"
#include "../../formrepconventions.h"

namespace tarski {

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
  VarKeyedMap<int> polyStrict(const VarKeyedMap<int> &knownSigns, const short &sign, FernPolyIter &F, bool& success)
  {
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
      int bestScore = 0; // we want the minimum score!
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
  VarKeyedMap<int> polyNonStrict(const VarKeyedMap<int> &knownSigns, const short &sign, FernPolyIter &F, bool& success)
  {
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
  VarKeyedMap<int> polyNotEqual(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, bool& success)
  {
    VarKeyedMap<int> tmp(ALOP);
    VarKeyedMap<int> res(ALOP);
    std::vector<VarKeyedMap <int> > candidates;
    std::vector<Variable> allVars = F.getAllVars();
    success = true;


    short tgtSgn = NOOP;
    do {
      tmp = termNotEqual(knownSigns, F, success);
      if (success) {
        tgtSgn = NEOP;
        break;
      }
      success = true;
      tmp = termStrict(knownSigns, F, GTOP, success);
      if (success) {
        tgtSgn = GTOP;
          break;
      }
      success = true;
      tmp = termStrict(knownSigns, F, LTOP, success);
      if (success) {
        tgtSgn = LTOP;
        break;
      }
      success = true;
      tmp = termEqual(knownSigns, F, success);
      if (!success) {
        return res;
      }
      res = combine(res, tmp, allVars);
    } while (tgtSgn == NOOP && F.next() != F.end());

    //cout << "tgtSgn is " << numToRelop(tgtSgn) << endl;
    if (tgtSgn == LTOP || tgtSgn == GTOP) {
      tmp = polyEqual(knownSigns, F, success);
      res = combine(res, tmp, allVars);
    }
    else
      while (F.next() != F.end()) {
        VarKeyedMap<int> tmp;
        tmp = termEqual(knownSigns, F, success);
        if (!success) return res;
        res = combine(res, tmp, allVars);
      }


    return res;
  }


  /*
    Requires a fresh FernPolyIter
  */
  VarKeyedMap<int> polyEqual(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, bool& success)
  {
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


  void writePolyExp(const VarKeyedMap<int> &signs, const short &sign, FernPolyIter &F, const std::vector<Variable> &vars,   VarContext &VC)
  {
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
      int sold = oldMap.get(*vIter);
      int snew = sigma_combine(sold,candidate.get(*vIter));
      score += signScores[snew] - signScores[sold];
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

