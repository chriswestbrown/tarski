#ifndef _MATRIX_MAN_H
#define _MATRIX_MAN_H
#include <vector>
#include <iostream>
#include <map>
#include "dmatrix.h"
#include "formula.h"

namespace tarski {
  class MatrixManager {
  private:
    DMatrix strict;
    DMatrix all;
    PolyManager * PM;
    std::vector<IntPolyRef> cIdxToPoly;
    std::vector<TAtomRef> rIdxToAtom;
    std::vector<int> tB;
    std::map<IntPolyRef, int>  allPolys;
    std::map<IntPolyRef, TAtomRef> strongMap;
    void addNewStrict(IntPolyRef);
    void swapToStrict(IntPolyRef);
  public:
    MatrixManager(TAndRef);
    void addAtom(TAtomRef t);
    void write();

    /* INLINE CONST METHODS */
    inline IntPolyRef getPoly(int idx)           const { return cIdxToPoly[idx];      }
    inline TAtomRef getAtom(int idx)             const { return rIdxToAtom[idx];      }
    inline TAtomRef getStrictAtom(int idx)       const { return rIdxToAtom[tB[idx]];  }
    inline TAtomRef strengthenPoly(IntPolyRef p) const {
      return strongMap.at(p);
    }
    inline const DMatrix& getStrict()            const { return strict;               }
    inline const DMatrix& getAll()               const { return all;                  }
    static inline bool isStrictRelop(TAtomRef t)  {
      return (t->getRelop() == LTOP || t->getRelop() == GTOP || t->getRelop() == NEOP);
    }

    /* INLINE METHODS*/
    inline void strictElim()                     { strict.doElim(); }
    inline void allElim()                        { all.doElim();    }


 
  };


}//end namespace tarski
#endif
