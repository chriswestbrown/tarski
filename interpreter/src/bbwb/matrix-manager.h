#ifndef _MATRIX_MAN_H
#define _MATRIX_MAN_H
#include <vector>
#include <iostream>
#include <map>
#include "dmatrix.h"
#include "formula.h"


//TODO: Handle case where an atom which is only in the NS matrix
//      Becomes eligible for the strict matrix
//TODO: Handle NEOP/EQOP atoms
namespace tarski {
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
    std::map<TAtomRef, set<IntPolyRef> > nSR;
    void addNewStrict(IntPolyRef);
    void swapToStrict(IntPolyRef);


    void strictUpdate();
    inline int getNumStrictCols() {return strict.getNumCols(); }
  public:
    MatrixManager(TAndRef);
    void addAtom(TAtomRef t);
    void write();

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
      std::cerr << "Strengthening: "; p->write(*PM); std::cerr << endl;
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
    inline void allElim()                        { all.doElim();    }


 
  };


}//end namespace tarski
#endif
