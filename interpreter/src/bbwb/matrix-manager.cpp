#include "matrix-manager.h"
#include <assert.h>
namespace tarski {

  MatrixManager::MatrixManager(TAndRef t) : strict() {
    PM = t->getPolyManagerPtr();
    std::set<IntPolyRef> strongPolys;
    std::set<IntPolyRef> weakPolys;
    //The first loop determines which polynomials are strong, and makes a vector of all atoms
    for (TAndObj::conjunct_iterator itr = t->conjuncts.begin(); itr != t->conjuncts.end(); ++itr) {
      TAtomRef tf = asa<TAtomObj>(*itr);
      rIdxToAtom.push_back(tf);
      if (tf->getRelop() == NEOP || tf->getRelop() == GTOP || tf->getRelop() == LTOP)
        for (map<IntPolyRef, int>::iterator fitr = tf->factorsBegin(); fitr != tf->factorsEnd(); ++fitr) {
          strongPolys.insert(fitr->first);
          strongMap[fitr->first] = tf;
        }
      else
        for (map<IntPolyRef, int>::iterator fitr = tf->factorsBegin(); fitr != tf->factorsEnd(); ++fitr) {
          weakPolys.insert(fitr->first);
        }
    }

    //These two loops give each polynomial its index
    for (std::set<IntPolyRef>::iterator itr = strongPolys.begin(), end = strongPolys.end(); itr != end; ++itr) {
      allPolys[*itr] = cIdxToPoly.size();
      cIdxToPoly.push_back(*itr);
      weakPolys.erase(*itr);
    }
    for (std::set<IntPolyRef>::iterator itr = weakPolys.begin(), end = weakPolys.end(); itr != end; ++itr) {
      allPolys[*itr] = cIdxToPoly.size();
      cIdxToPoly.push_back(*itr);
    }

    //This loop gives each Atom in the formula a row, and populates that row
    DMatrix d(rIdxToAtom.size(), cIdxToPoly.size());
    cerr << "strongPolys.size is " << strongPolys.size();
    for (int i = 0; i < rIdxToAtom.size(); i++) {
      TAtomRef tf = rIdxToAtom[i];
      std::vector<char> strictRow(strongPolys.size());
      if (tf->getRelop() == LTOP || tf->getRelop() == LEOP) { d.set(i, 0, true); strictRow[0] = true; }
      bool noStrict = false;
      for (map<IntPolyRef, int>::iterator fitr = tf->factorsBegin();
           fitr != tf->factorsEnd(); ++fitr) {
        assert(allPolys.find(fitr->first) != allPolys.end());
        if (tf->getRelop() == GTOP || tf->getRelop() == LTOP || tf->getRelop() == GEOP || tf->getRelop() == LEOP){
          int j = allPolys[fitr->first];
          if (fitr->second % 2 == 0 && fitr->second > 0 && j >= strongPolys.size()) d.set(i, j, 2);
          else if (fitr->second % 2 == 1) d.set(i, j, 1);
          if (j >= strongPolys.size()) noStrict = true;
          if (!noStrict) strictRow[j] = fitr->second % 2;
        }
        else if (tf->getRelop() == NEOP || tf->getRelop() == EQOP) {

        }
      }
      cerr << "strictRow size is " << strictRow.size() << endl;
      if (noStrict) strict.addRow(strictRow);
    }
    all = d;

    vector<int> traceBack(rIdxToAtom.size());
    for (int i = 0; i < traceBack.size(); i++) traceBack[i] = i;
    tB = traceBack;
    write();
  }

  void MatrixManager::addAtom(TAtomRef tf) {
    rIdxToAtom.push_back(tf);
    vector<char> newRowVec(allPolys.size());
    for (map<IntPolyRef, int>::iterator fitr = tf->factorsBegin(); fitr != tf->factorsEnd(); ++fitr) {
      IntPolyRef p = fitr->first;
      if (allPolys.find(p) == allPolys.end() ) {
        //unknown and learned strict
        if (isStrictRelop(tf)) {
          addNewStrict(p);
          newRowVec.push_back(false);
          newRowVec[allPolys[p]] = true;
          strongMap[p] = tf;
        }
        //unknown and learned not strict
        else {
          all.addCol();
          allPolys[p] = all.getNumCols();
          cIdxToPoly.push_back(p);
          newRowVec.push_back(false);
        }
      }
      //was non strict but learned strict
      if (isStrictRelop(tf)) {
        if (allPolys[p] >= strict.getNumCols()) {
          swapToStrict(p);
          newRowVec[allPolys[p]] = true;
          strongMap[p] = tf;
        }
      }
      //not strict and known not strict
      else {
        newRowVec[allPolys[p]] = true;
      }
    }
    strict.addRow(newRowVec);
    all.addRow(newRowVec);
  }


  //increments all non strict idxs
  //sets p to last strict idx
  //changes matrix column sizes
  void MatrixManager::addNewStrict(IntPolyRef p) {
    IntPolyRef oldP = cIdxToPoly[strict.getNumCols()];
    cIdxToPoly.push_back(oldP);
    cIdxToPoly[strict.getNumCols()] = p;
    allPolys[oldP] = all.getNumCols();
    allPolys[p] = strict.getNumCols();
    all.addCol();
    all.swapBack(strict.getNumCols());
    strict.addCol();
  }

  void MatrixManager::swapToStrict(IntPolyRef p) {
    //We learned a polynomial is now strict that was nonstrict!
    IntPolyRef oldP = cIdxToPoly[strict.getNumCols()];
    cIdxToPoly[allPolys[p]] = oldP;
    cIdxToPoly[strict.getNumCols()] = p;
    allPolys[oldP] = all.getNumCols()-1;
    allPolys[p] = strict.getNumCols();
    all.swapBack(strict.getNumCols());
    strict.addCol();
  }


  void MatrixManager::write() {
    std::cout << "Vector Rep:" << std::endl;
    std::cout << "sigma";
    int r0 = getStrict().getMatrix().size();
    int m0 = getAll().getMatrix().size();
    for(int i = 1; i < m0; i++)
      {
        if (i == r0) { std::cerr << " :"; }
        std::cerr << " (";
        getPoly(i)->write(*PM);
        std::cerr << ")";
      }
    std::cout << std::endl;

    getStrict().write();
  }


}//end namespace
