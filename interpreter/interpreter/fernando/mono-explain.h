#ifndef MONO_EXPLAIN_H
#define MONO_EXPLAIN_H


#include "../poly/gcword.h"
#include "../GC_System/GC_System.h"
#include "../formula/monomialinequality.h"
#include "../formula/formula.h"
#include <stack>
#include <vector>

namespace tarski {
  class FernPolyIter;
  class PolyManager;
  class VarSet;
  template<class T> class VarKeyedMap;
  typedef GC_Hand<IntPolyObj> IntPolyRef;

  VarKeyedMap<int> monoStrict(const VarKeyedMap<int> &knownSigns, FernPolyIter& F, const short &targetSign, bool& success);
  VarKeyedMap<int> monoStrict(const VarKeyedMap<int> &knownSigns, const std::vector<Variable> &vars, const std::vector<short> &exponents, const short &targetSign,  bool& success);
  VarKeyedMap<int> termStrict(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, const short &targetSign, bool& success);
  VarKeyedMap<int> termStrict(const VarKeyedMap<int> &knownSigns, const std::vector<Variable> &vars,  const std::vector<short> &exponents, const short &targetSign, const int &coefficient, bool& success);
  VarKeyedMap<int> monoNonStrict(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, const short &targetSign, bool &success);
  VarKeyedMap<int> termNonStrict(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, const short &targetSign, bool &success);
  VarKeyedMap<int> termNotEqual(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, bool& success);
  VarKeyedMap<int> termEqual(const VarKeyedMap<int> &knownSigns, FernPolyIter &F, bool& success);
  void selectAndWriteTerm(const VarKeyedMap<int> &signs, FernPolyIter &F,  VarContext &VC);
  void writeExp(const VarKeyedMap<int> &signs, const std::vector<Variable> &vars, VarContext &VC);
  void writeExp(const VarKeyedMap<int> &signs, const VarSet &vars, VarContext &VC);
}
#endif
