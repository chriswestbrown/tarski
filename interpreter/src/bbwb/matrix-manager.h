#ifndef _MATRIX_MAN_H
#define _MATRIX_MAN_H
#include <vector>
#include <iostream>
#include <map>
#include <list>
#include <forward_list>
#include "dmatrix.h"
#include "formula.h"
#include "poly.h"

namespace tarski {

  struct IntPolyComp {
    bool operator() (const IntPolyRef& l, const IntPolyRef& r) const {
      return IntPolyObj::ipcmp(l, r);
    }
  };

  class MatrixManager {
  private:
    
    DMatrix strict;
    DMatrix all;
    PolyManager * PM;
    bool needUpdate;
    std::vector<IntPolyRef> cIdxToPoly;
    std::vector<TAtomRef> rIdxToAtom;
    std::vector<size_t> tB;
    //tB = traceBack. Not all rows in strict have 1:1 correspondences
    //with rows in all, because strict doesn't store rows which correspond
    //to nonStrict atoms. tB[rowIndex] lets us use the rIdxToAtom vector
    //as a hashmap for both strict and nonStrict.

    
    std::map<IntPolyRef, int>  allPolys;
    std::map<IntPolyRef, TAtomRef> strongMap;
    void addNewStrict(IntPolyRef);
    void swapToStrict(IntPolyRef);

    bool isStrictAtom(TAtomRef t);

    void strictUpdate();
    inline int getNumStrictCols() {return strict.getNumCols(); }
  public:
    MatrixManager(TAndRef);
    MatrixManager(const MatrixManager&, const vector<int>&);

    //-- return true if the polynomial p is "strict", i.e. if it is present in a strict atom
    bool isStrictPoly(IntPolyRef p);

    void addAtom(TAtomRef t);
    TAtomRef getMeaning(int row);
    forward_list<TAtomRef> explainMeaning(int row);
    void write() const;
    TAtomRef mkNonStrictAtom(const vector<char>& row, bool& res);

    /* INLINE  METHODS */
    inline IntPolyRef getPoly(int idx) const {
      return cIdxToPoly[idx];
    }
    inline int getIdx(IntPolyRef p) const {
      return (allPolys.find(p) != allPolys.end()) ? allPolys.find(p)->second
        : -1;
    }
    inline TAtomRef getAtom(int idx, bool strict) const {
      return (strict) ? rIdxToAtom[tB[idx]] : rIdxToAtom[idx];
    }
    inline TAtomRef getStrictAtom(int idx)       const {
      return rIdxToAtom[tB[idx]];
    }
    inline TAtomRef strengthenPoly(IntPolyRef p) const {
      return strongMap.at(p);
    }
    inline const DMatrix& getStrict() const { return strict; }
    inline const DMatrix& getAll() const { return all; }
    static inline bool isStrictRelop(TAtomRef t)  {
      return (t->getRelop() == LTOP || t->getRelop() == GTOP
              || t->getRelop() == NEOP);
    }

    inline void strictElim() {
      strictUpdate();
      strict.doElim();
    }
    inline void allElim(){
      all.doElim();
    }

    //Returns a list of the indices of
    //all the rows in the all matrix
    //which are in both matrix
    inline const vector<size_t>& dualRows() {
      return tB;
    }


 
  };


}//end namespace tarski
#endif
