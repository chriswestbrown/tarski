#ifndef _MATRIX_MAN_H
#define _MATRIX_MAN_H
#include <vector>
#include <iostream>
#include <map>
#include <list>
#include "dmatrix.h"
#include "formula.h"
#include "poly.h"

namespace tarski {

  struct IntPolyComp {
    bool operator() (const IntPolyRef& l, const IntPolyRef& r) {
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
    std::vector<int> tB;
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
    void addAtom(TAtomRef t);
    void write() const;

    /* INLINE  METHODS */
    inline IntPolyRef getPoly(int idx) const {
      return cIdxToPoly[idx];
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
      return (t->getRelop() == LTOP || t->getRelop() == GTOP || t->getRelop() == NEOP);
    }


    /* INLINE METHODS*/
    inline void strictElim() {
      strictUpdate();
      strict.doElim();
    }
    inline void allElim(){
      all.doElim();
    }


 
  };


}//end namespace tarski
#endif
