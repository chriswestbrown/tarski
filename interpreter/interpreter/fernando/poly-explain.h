#ifndef POLY_EXPLAIN_H
#define POLY_EXPLAIN_H


#include <vector>
#include "../formula/formula.h"

namespace tarski {
  class FernPolyIter;

  static short signScores[8] = {9999, 7, 7, 5, 7, 4, 5, 0};
  template<class T> class VarKeyedMap;
  //This table dictates how to combine two signs to get the stricter of the two
  //It works for VarKeyedMaps whose defaults are ALOP or NOOP


  /* PolySign algorithms from the paper */
  VarKeyedMap<int> polyStrict(const VarKeyedMap<int> &knownSigns, const short &sign, FernPolyIter &F, bool& success);
  VarKeyedMap<int> polyNonStrict(const VarKeyedMap<int> &knownSigns, const short &sign, FernPolyIter &F, bool& success);
  VarKeyedMap<int> polyNotEqual(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, bool& success);
  VarKeyedMap<int> polyEqual(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, bool& success);

  /* Combines the VarKeyedMaps so that all entries are the stricter of the two according to T_combine */
  VarKeyedMap<int> combine(const VarKeyedMap<int> &v1, const VarKeyedMap<int> &v2, const std::vector<Variable> &vars);

  /* Given a sign, calls the appropriate function*/
  VarKeyedMap<int> select(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, const short &sign, bool &success);

  void writePolyExp(const VarKeyedMap<int> &signs, const short &sign, FernPolyIter &F, const std::vector<Variable> &vars,   VarContext &VC);
  int  polyScoreFun(const VarKeyedMap<int> &oldMap, const VarKeyedMap<int> &candidate, const std::vector<Variable> &vars);

}
#endif
