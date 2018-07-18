#include "matrix-manager.h"
#include <assert.h>
namespace tarski {

  MatrixManager::MatrixManager(TAndRef t) : strict(), needUpdate(false), cIdxToPoly(1) {
    PM = t->getPolyManagerPtr();
    std::set<IntPolyRef> strongPolys;
    std::set<IntPolyRef> weakPolys;
    //The first loop determines which polynomials are strong, and makes a vector of all atoms
    for (TAndObj::conjunct_iterator itr = t->conjuncts.begin(); itr != t->conjuncts.end(); ++itr) {
      TAtomRef tf = asa<TAtomObj>(*itr);
      if (tf->getRelop() != NEOP && tf->getRelop() != EQOP)
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
    for (int i = 0; i < rIdxToAtom.size(); i++) {
      TAtomRef tf = rIdxToAtom[i];
      std::vector<char> strictRow(strongPolys.size()+1);
      if (tf->getRelop() == LTOP || tf->getRelop() == LEOP) { d.set(i, 0, true); strictRow[0] = true; }
      bool noStrict = false;
      for (map<IntPolyRef, int>::iterator fitr = tf->factorsBegin();
           fitr != tf->factorsEnd(); ++fitr) {
        assert(allPolys.find(fitr->first) != allPolys.end());

        if (tf->getRelop() == GTOP || tf->getRelop() == LTOP || tf->getRelop() == GEOP || tf->getRelop() == LEOP){
          int j = allPolys[fitr->first];
          if (fitr->second % 2 == 0 && fitr->second > 0
              && j >= strongPolys.size()) d.set(i, j, 2);
          else if (fitr->second % 2 == 1) d.set(i, j, 1);
          if (j >= strongPolys.size()+1)  {
            noStrict = true;
            nSR[tf].push_back(fitr->first);
          }
          if (!noStrict) strictRow[j] = fitr->second % 2;
        }
      }
      if (!noStrict) {
        tB.push_back(i);
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
          //TODO: Some function to reassign NS atoms to strict
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



    bool isStrict = true;
    vector<char> newRowVec(allPolys.size());
    for (map<IntPolyRef, int>::iterator fitr = tf->factorsBegin();
         fitr != tf->factorsEnd(); ++fitr) {
      IntPolyRef p = fitr->first;
      if (allPolys.find(p) == allPolys.end() ) {
        //unknown and learned not strict
        isStrict = false;
        nSR[tf].push_back(p);
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
          nSR[tf].push_back(p);
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


  //increments all non strict idxs
  //sets p to last strict idx
  //changes matrix column sizes
  void MatrixManager::addNewStrict(IntPolyRef p) {
    if (strict.getNumCols() == all.getNumCols()) {
      cIdxToPoly.push_back(p);
      all.addCol();
      strict.addCol();
    }
    else {
      IntPolyRef oldP = cIdxToPoly[strict.getNumCols()];
      cIdxToPoly.push_back(oldP);
      cIdxToPoly[strict.getNumCols()] = p;
      allPolys[oldP] = all.getNumCols();
      allPolys[p] = strict.getNumCols();
      all.addCol();
      all.swapBack(strict.getNumCols());
      strict.addCol();
    }
  }

  void MatrixManager::swapToStrict(IntPolyRef p) {
    //We learned a polynomial is now strict that was nonstrict!
    IntPolyRef oldP = (getNumStrictCols() != 0) ? cIdxToPoly[getNumStrictCols()] : cIdxToPoly[1];
    cIdxToPoly[allPolys[p]] = oldP;
    if (strict.getNumCols() == 0)
      cIdxToPoly[1] = p;
    else
      cIdxToPoly[strict.getNumCols()] = p;
    allPolys[oldP] = all.getNumCols()-1;
    allPolys[p] = strict.getNumCols();
    all.swapBack(strict.getNumCols());
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
      assert(nSR.find(tf) != nSR.end());
      for (list<IntPolyRef>::iterator itr = nSR[tf].begin(); itr != nSR[tf].end(); ++itr) {
        if (strongMap.find(*itr) != strongMap.end())  {
          list<IntPolyRef>::iterator itr2 = itr;
          --itr;
          nSR[tf].erase(itr2);
        }
      }
      if (nSR[tf].empty())  {
        isStrict[i] = true;
        strict.addRow(all.getMatrix().at(i));
        tB.push_back(i);
        nSR.erase(rIdxToAtom[i]);
      }
    }
    needUpdate = false;
  }

}//end namespace
