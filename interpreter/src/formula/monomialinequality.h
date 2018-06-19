#ifndef _monomial_inequality_H_
#define _monomial_inequality_H_


#include "normalize.h"
#include <vector>
#include <set>
#include <queue>
#include <map>
#include "DynamicBitVector.h"
#include "../shell/einterpreter.h"

namespace tarski {
class MIVectRep 
{
public:
  DBV strictPart;
  FunnyVec nonstrictPart;
  MIVectRep() { }
  MIVectRep(int n, int r) { strictPart.resize(r+1); nonstrictPart.resize(n-r); }
  bool isStrict() const { return nonstrictPart.isZero(); }
  int get(int i) const { return  i < strictPart.size() ? strictPart.get(i) : nonstrictPart.get(i - strictPart.size());}
  int operator+=(const MIVectRep &V);
  bool operator==(const MIVectRep &V) const 
  { 
    return strictPart == V.strictPart  && nonstrictPart == V.nonstrictPart;
  } 
  bool operator<(const MIVectRep &V) const // purely lexicographic
  { 
    return strictPart < V.strictPart || (strictPart == V.strictPart  && nonstrictPart < V.nonstrictPart);
  } 
  void write() const
  {
    strictPart.write();
    std::cout << " : ";
    nonstrictPart.write();
  }
  inline int strictSize() {
    return strictPart.size();
  }
  inline int size() {
    return strictPart.size() + nonstrictPart.size()/2;
  }
};

class MonoIneqRep
{
public:
  int numVars() const { return RefToIndex.size(); };
  int numStrict() const { return B[0].strictPart.size() - 1; }
  void writeMI(MIVectRep &);
  void write(MIVectRep &);
  int processAtom(TAtomRef A) ;
  int processConjunction(TAndRef C);
  MonoIneqRep() { nextIndex = 0; }
  const std::map<IntPolyRef, std::vector < TAtomRef > >& getCandidates() const {return RefToSIneq;}
  // Input:  i - the index of a row in B
  // Output: a - the atom in C corresponding to row i (see processConjunction)
  TAtomRef rowIndexToAtom(int i)
  {
    return BtoC[i];
  }

  std::vector<MIVectRep> B; // Basis
  std::vector<std::map<IntPolyRef,int>::iterator> reverseMap;
  void printCandidates(TFormRef F);

  Matrix genStrictMatrix(std::vector<int>& traceRow) {
    // Pull out the strict part
    int r = numStrict();
    int m = 0; for(unsigned int i = 0; i < B.size(); ++i) if (B[i].nonstrictPart.isZero()) ++m;
    //  if (r == 0) return; // No strict part!
    Matrix M(m,1+r);
    for(unsigned int i = 0, j = 0; i < B.size(); ++i) {
      if (B[i].nonstrictPart.isZero()) {
        M[j++] = B[i].strictPart;
        traceRow.push_back(i);
      }
    }
    return M;
  }

 private:
  int nextIndex;
  std::map< IntPolyRef,int> RefToIndex;
  std::map< IntPolyRef, std::vector < TAtomRef > > RefToSIneq; //Maps factors to each of the strong inequalities it appears in

  std::vector<bool> indexToType; // 0 = strict, 1 = nonstrict
  std::vector<TAtomRef> BtoC; // BtoC[i] = j means row i of B corresponds to atom C[i] in conjunction C

};

}//end namespace tarski
#endif
