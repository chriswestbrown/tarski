#ifndef MIN_WT_EXP_H
#define MIN_WT_EXP_H

#include "../formula/monomialinequality.h"
#include <vector>
#include <unordered_set>
#include <list>

namespace tarski {
  //Forward Declares
  class BBDed;
  class Deduction;
  DBV trans(const MIVectRep &w, int ns, int nn);
  int sc(const MIVectRep &a, const MIVectRep &b);
  int wt(const MIVectRep &w);

  //Function definitions
  DBV reduceExplain(Matrix &M, vector<int> &piv, vector<DBV>& traceMatrix, vector<int>& traceRow, const DBV &v, unordered_set<int>& explain);
  DBV reduceExplain2(Matrix &M, vector<int> &piv, vector<DBV>& traceMatrix, const vector<unordered_set<int> >& trace, const DBV &v, unordered_set<int>& explain);
  TAtomRef scoreFun2(MonoIneqRep &MIR, IntPolyRef p, std::vector<short> pureStrict);
  bool getDed(MIVectRep& mi, MonoIneqRep * MIR, TAtomRef& t, PolyManager * PM);
  std::vector<Deduction * > minWtBasisDeds(MonoIneqRep &MIR, std::vector<short> pureStrict);
  void minWtBasisExp(MonoIneqRep &MIR, vector<MIVectRep> &Bf, std::vector<std::vector<TAtomRef> >& finExp);
  inline void addExp(std::unordered_set<int>& exp, vector<vector<TAtomRef > >& finExp, MonoIneqRep& MIR);



  //helper methods
  inline void eraseFromList(MIVectRep,  list<MIVectRep>&);
  inline bool isIn(MIVectRep, std::list<MIVectRep>);
  inline void merge(unordered_set<int>& s1, const unordered_set<int>& s2);

}
#endif
