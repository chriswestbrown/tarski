#include "matrix-manager.h"
#include <assert.h>
namespace tarski {


  //This constructor is intended to be able to create a MatrixManager as
  //a subset of the rows of another
  //the vector rows indicates which rows we desire in the new matrix manager
  MatrixManager::MatrixManager(const MatrixManager& m, const vector<int>& rows) : cIdxToPoly(m.cIdxToPoly) {
    size_t j = 0;
    int tbRow;
    if (m.tB.size() > j) tbRow = m.tB[j];
    else tbRow = -1; 
    for (size_t i = 0; i < rows.size(); i++) {
      int row = rows[i];
      rIdxToAtom.push_back(m.rIdxToAtom[row]);
      all.addRow(m.getAll().getRow(row));
      if (tbRow == row) {
        strict.addRow(m.getStrict().getRow(j));
        tB.push_back(i);
        j++;
        if (m.tB.size() <= j) tbRow = -1;
        else tbRow = m.tB[j]; 
      }
    }
  }

  MatrixManager::MatrixManager(TAndRef t) : strict(), needUpdate(false), cIdxToPoly(1) {
    PM = t->getPolyManagerPtr();
    std::set<IntPolyRef> strongPolys;
    std::set<IntPolyRef> weakPolys;
    //The first loop determines which polynomials are strong, and makes a vector of all atoms
    for (TAndObj::conjunct_iterator itr = t->conjuncts.begin(); itr != t->conjuncts.end(); ++itr) {
      TAtomRef tf = asa<TAtomObj>(*itr);
      if (tf->getRelop() != NEOP)
        rIdxToAtom.push_back(tf);

      if (isStrictRelop(tf)) {
        for (map<IntPolyRef, int>::iterator fitr = tf->factorsBegin(); fitr != tf->factorsEnd(); ++fitr) {
          strongPolys.insert(fitr->first);
          strongMap[fitr->first] = tf;
        }
      }
      else {
        for (map<IntPolyRef, int>::iterator fitr = tf->factorsBegin(); fitr != tf->factorsEnd(); ++fitr) {
          weakPolys.insert(fitr->first);
        }
      }
    }

    //These two loops give each polynomial its index
    for (std::set<IntPolyRef>::iterator itr = strongPolys.begin(), end = strongPolys.end(); itr != end; ++itr) {
      IntPolyRef p = *itr;
      allPolys[p] = cIdxToPoly.size();
      cIdxToPoly.push_back(p);
      weakPolys.erase(p);
    }
    for (std::set<IntPolyRef>::iterator itr = weakPolys.begin(), end = weakPolys.end(); itr != end; ++itr) {
      IntPolyRef p = *itr;
      allPolys[p] = cIdxToPoly.size();
      cIdxToPoly.push_back(p);
    }

    //This loop gives each Atom in the formula a row, and populates that row
    DMatrix d(rIdxToAtom.size(), cIdxToPoly.size());
    for (size_t i = 0; i < rIdxToAtom.size(); i++) {
      TAtomRef tf = rIdxToAtom[i];
      if (tf->getRelop() == LTOP || tf->getRelop() == LEOP ||
          tf->getRelop() == EQOP) {
        d.set(i, 0, true);
      }
      bool eqMod = (tf->getRelop() == EQOP) ? true : false;
      bool noStrict = false;
      for (map<IntPolyRef, int>::iterator fitr = tf->factorsBegin();
           fitr != tf->factorsEnd(); ++fitr) {
        assert(allPolys.find(fitr->first) != allPolys.end());
        size_t j = allPolys[fitr->first];
        if (fitr->second % 2 == 0 && fitr->second > 0
            && j > strongPolys.size()) d.set(i, j, 2);
        else if (fitr->second % 2 == 1 && j > strongPolys.size()
                 && eqMod) d.set(i, j, 2);
        else if (fitr->second % 2 == 1 && !eqMod) d.set(i, j, 1);
        if (j >= strongPolys.size()+1)  {
          noStrict = true; 
        }
      }
      if (!noStrict) {
        tB.push_back(i);
        vector<char> strictRow(d.getRow(i).begin(),
                               d.getRow(i).begin()+strongPolys.size()+1);
        strict.addRow(strictRow);
      }
    }
    all = d;
  }

  void MatrixManager::addAtom(TAtomRef tf) {
    bool isRow = true;
    if (tf->getRelop() == NEOP || tf->getRelop() == EQOP) {
      isRow = false;
    }
    else {
      rIdxToAtom.push_back(tf);
    }
    if (isStrictRelop(tf)) {
      std::vector<char> newRowVec(getNumStrictCols(), 0);
      if (getNumStrictCols() == 0) newRowVec.resize(1, 0);
      if (tf->getRelop() == LTOP) {
        newRowVec[0] = 1;
      }
      for (map<IntPolyRef, int>::iterator fitr = tf->factorsBegin();
           fitr != tf->factorsEnd(); ++fitr) {
        IntPolyRef p = fitr->first;
        short val = fitr->second % 2;
        std::map<IntPolyRef, int>::iterator pItr = allPolys.find(p);
        //unknown and learned strict
        if (pItr == allPolys.end()) {
          addNewStrict(p);
          newRowVec.push_back(false);
          newRowVec[allPolys[p]] = val;
          strongMap[p] = tf;
        }
        //known and becomes strict
        else if (pItr->second >= getNumStrictCols()){
          needUpdate = true;
          swapToStrict(p);
          newRowVec.push_back(val);
          strongMap[p] = tf;
        }
        //known and was already strict
        else {
          newRowVec[allPolys[p]] = val;
        }
      }
      if (isRow) {
        strict.addRow(newRowVec);
        all.addRow(newRowVec);
        tB.push_back(rIdxToAtom.size()-1);
      }
      return;
    }


    else {
      bool isStrict = true;
      vector<char> newRowVec(allPolys.size());
      for (map<IntPolyRef, int>::iterator fitr = tf->factorsBegin();
           fitr != tf->factorsEnd(); ++fitr) {
        IntPolyRef p = fitr->first;
        if (allPolys.find(p) == allPolys.end() ) {
          //unknown and learned not strict
          isStrict = false;
          
          all.addCol();
          allPolys[p] = cIdxToPoly.size();
          cIdxToPoly.push_back(p);
          newRowVec.push_back(false);
        }
        //not strict and known not strict
        else {
          int j = allPolys[p];
          if (isStrict && j >= getNumStrictCols())  {
            isStrict = false;
            
          }
          if (fitr->second % 2 == 0 && fitr->second > 0
              && j >= getNumStrictCols()) newRowVec[j] = 2;
          else if (fitr->second % 2 == 0) newRowVec[j] = 0;
          else newRowVec[j] = 1;
        }
      }
      if (isStrict && isRow) {
        if (getNumStrictCols() == 0) {
          vector<char> sRowVec(newRowVec.begin(), newRowVec.begin()+1+strongMap.size());
          strict.addRow(sRowVec);
        }
        tB.push_back(rIdxToAtom.size()-1);
      }
      if (isRow) all.addRow(newRowVec);
    }
  }


  //increments all non strict idxs
  //sets p to last strict idx
  //changes matrix column sizes
  void MatrixManager::addNewStrict(IntPolyRef p) {
    if (strict.getNumCols() == all.getNumCols()) {
      allPolys[p] = cIdxToPoly.size();
      cIdxToPoly.push_back(p);
      all.addCol();
      strict.addCol();

    }
    else {
      short oldIdx = (getNumStrictCols() != 0) ? getNumStrictCols() : 1;
      IntPolyRef oldP = cIdxToPoly[oldIdx];
      cIdxToPoly.push_back(oldP);
      cIdxToPoly[oldIdx] = p;
      allPolys[oldP] = all.getNumCols();
      allPolys[p] = oldIdx;
      all.addCol();
      all.swapBack(strict.getNumCols());
      strict.addCol();
    }
  }

  void MatrixManager::swapToStrict(IntPolyRef p) {
    //We learned a polynomial is now strict that was nonstrict!
    short oldIdx = (getNumStrictCols() != 0) ? getNumStrictCols() : 1;
    IntPolyRef oldP = cIdxToPoly[oldIdx];
    short nuIdx = allPolys[p];
    //in this case, no swap necessary since it is already in proper place
    if (nuIdx == oldIdx) {
      strict.addCol();
      return;
    }
    cIdxToPoly[nuIdx] = oldP;
    cIdxToPoly[oldIdx] = p;
    allPolys[oldP] = nuIdx;
    all.swapCol(oldIdx, nuIdx);
    allPolys[p] = oldIdx;
    strict.addCol();
  }


  void MatrixManager::write() const {
    std::cout << "Vector Rep:" << std::endl;
    std::cout << "sigma";

    int r0 = getStrict().getNumCols();
    int m0 = getAll().getNumCols();
    for(int i = 1; i < m0; i++)
      {
        if (i == r0) { std::cerr << " :"; }
        std::cerr << " (";
        getPoly(i)->write(*PM);
        std::cerr << ")";
      }
    std::cout << std::endl <<  "Strict:" << std::endl;
    getStrict().write();
    std::cout << std::endl << "Nonstrict: " << std::endl;
    getAll().write();
  }


  void MatrixManager::strictUpdate() {
    if (!needUpdate) return;
    vector<bool> isStrict(rIdxToAtom.size(), false);
    for (std::vector<int>::iterator it = tB.begin(); it != tB.end(); ++it) {
      isStrict[*it] = true;
    }
    for (int i = 0; i < rIdxToAtom.size(); i++) {
      if (isStrict[i]) continue;
      TAtomRef tf = rIdxToAtom[i];
      if (isStrictAtom(tf))  {
        strict.addRow(all.getMatrix().at(i));
        tB.push_back(i);
        }
    }
    needUpdate = false;
  }

  bool MatrixManager::isStrictAtom(TAtomRef t) {
    FactRef F = t->getFactors();
    std::map<IntPolyRef, int>::iterator itr = F->MultiplicityMap.begin(), end = F->MultiplicityMap.end();
    for (; itr != end; ++itr ) {
      IntPolyRef p = itr->first;
      if (strongMap.find(p) == strongMap.end())
        return false;
    }
    return true;
  }

}//end namespace
