#include "matrix-manager.h"
#include <assert.h>
namespace tarski {


  //This constructor is intended to be able to create a MatrixManager as
  //a subset of the rows of another
  //the vector rows indicates which rows we desire in the new matrix manager
  MatrixManager::MatrixManager(const MatrixManager& m, const vector<int>& rows) :
    strict(rows.size(),-1), all(rows.size(),-1), needUpdate(false), cIdxToPoly(m.cIdxToPoly)
  {
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

  MatrixManager::MatrixManager(TAndRef t) : strict(0), all(0), needUpdate(false), cIdxToPoly(1) {
    PM = t->getPolyManagerPtr();
    std::set<IntPolyRef> strongPolys;
    std::set<IntPolyRef> weakPolys;
    //The first loop determines which polynomials are strong, and makes a vector of all atoms
    for (TAndObj::conjunct_iterator itr = t->conjuncts.begin(); itr != t->conjuncts.end(); ++itr) {
      TAtomRef tf = asa<TAtomObj>(*itr);
      if (tf->getRelop() != NEOP)
        rIdxToAtom.push_back(tf);

      if (isStrictRelop(tf)) {
        for (auto fitr = tf->factorsBegin(); fitr != tf->factorsEnd(); ++fitr) {
          strongPolys.insert(fitr->first);
          strongMap[fitr->first] = tf;
        }
      }
      else {
        for (auto fitr = tf->factorsBegin(); fitr != tf->factorsEnd(); ++fitr) {
          weakPolys.insert(fitr->first);
        }
      }
    }

    // These two loops give each polynomial its index
    // NOTE: cIdxToPoly already has an index 0 element to the place for the relop
    //       i.e. column 0 is already taken.
    for (auto itr = strongPolys.begin(), end = strongPolys.end(); itr != end; ++itr) {
      IntPolyRef p = *itr;
      allPolys[p] = cIdxToPoly.size();
      cIdxToPoly.push_back(p);
      weakPolys.erase(p);
    }
    for (auto itr = weakPolys.begin(), end = weakPolys.end(); itr != end; ++itr) {
      IntPolyRef p = *itr;
      allPolys[p] = cIdxToPoly.size();
      cIdxToPoly.push_back(p);
    }

    //This loop gives each Atom in the formula a row, and populates that row
    DMatrix Mtmp(0,strongPolys.size() + 1);
    swap(this->strict,Mtmp);

    DMatrix d(rIdxToAtom.size(), cIdxToPoly.size()); // note cIdxToPoly.size() is correct, see above NOTE
    for (size_t i = 0; i < rIdxToAtom.size(); i++) {
      TAtomRef tf = rIdxToAtom[i];
      if (tf->getRelop() == LTOP || tf->getRelop() == LEOP ||
          tf->getRelop() == EQOP) {
        d.set(i, 0, true);
      }
      bool eqMod = (tf->getRelop() == EQOP) ? true : false;
      bool noStrict = false;
      for (auto fitr = tf->factorsBegin();
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

  //ASSUMES STRICT
  TAtomRef MatrixManager::getMeaning(int row) {
    const vector<char>& vc = strict.getRow(row);
    FactRef F = new FactObj(PM);

    for (int i = 1; i < vc.size(); i++) {
      if (vc[i]) F->addFactor(getPoly(i), 1);
    }
    int relop;
    if (vc[0]) relop = LTOP;
    else relop = GTOP;
    TAtomRef t = new TAtomObj(F, relop);
    return t;
  }

  //ASSUMES STRICT
  forward_list<TAtomRef> MatrixManager::explainMeaning(int row) {
    forward_list<TAtomRef> fl;
    const vector<bool>& vb =  strict.getComp()[row];
    for (int i = 0; i < vb.size(); i++) {
      if (vb[i]) fl.push_front(getStrictAtom(i));
    }
    return fl;
  }


  TAtomRef MatrixManager::mkNonStrictAtom(const vector<char>& row, bool& res) {
    TAtomRef t;
    short relop = (row[0] == 0) ? GEOP : LEOP;
    FactRef F = new FactObj(PM);
    bool allTwo = true;
    for (size_t i = 1; i < row.size(); i++) {
      if (row[i] == 1) allTwo = false;
      if (row[i] != 0) F->addFactor(getPoly(i), row[i]); 
    }
    if (allTwo && relop == GEOP) { res = false; return t; }
    if (relop == LEOP && allTwo) {
      for (map<IntPolyRef,int>::iterator itr = F->MultiplicityMap.begin();
           itr != F->MultiplicityMap.end(); ++itr)
        itr->second = 1;
      t = new TAtomObj(F, EQOP);
    }
    else t = new TAtomObj(F, relop);
    res = true;
    return t;
  }


  void MatrixManager::addAtom(TAtomRef tf)
  {
    // cerr << "In addAtom("; tf->write(true); cerr << ")" << endl;
    //-- Dr. Brown DEBUG
    TAtomRef tf_orig = tf;
    if (tf->getRelop() == EQOP)
    {
      FactRef F = new FactObj(*(tf->getPolyManagerPtr()));
      for (auto fitr = tf->factorsBegin(); fitr != tf->factorsEnd(); ++fitr)
	F->addFactor(fitr->first,2);
      tf = new TAtomObj(F,LEOP);
    }
    
    bool isRow = true;
    if (tf->getRelop() == NEOP || tf->getRelop() == EQOP) { // NOTE: EQOP is not possible now! DRBROWN
      isRow = false;
    }
    else {
      rIdxToAtom.push_back(tf_orig);
    }

    if (isStrictRelop(tf))
    {
      std::vector<char> newRowVec(getNumStrictCols(), 0);
      if (getNumStrictCols() == 0) newRowVec.resize(1, 0);
      if (tf->getRelop() == LTOP) {
        newRowVec[0] = 1;
      }
      for (auto fitr = tf->factorsBegin(); fitr != tf->factorsEnd(); ++fitr) {
        IntPolyRef p = fitr->first;
        short val = fitr->second % 2;
        auto pItr = allPolys.find(p);
        //unknown and learned strict
        if (pItr == allPolys.end()) {
          addNewStrict(p);
          newRowVec.push_back(false);
          newRowVec[allPolys[p]] = val;
          strongMap[p] = tf_orig;
        }
        //known and becomes strict
        else if (pItr->second >= getNumStrictCols()){
          needUpdate = true;
          swapToStrict(p);
          newRowVec.push_back(val);
          strongMap[p] = tf_orig;
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

    else // tf is a non-strict relop
    { 
      bool isStrict = true;
      vector<char> newRowVec(allPolys.size() + 1,0);
      newRowVec[0] = tf->getRelop() == GEOP ? 0 : 1; // Note: this choice means EQOP gets value 1 as well as LEOP
      for (auto fitr = tf->factorsBegin(); fitr != tf->factorsEnd(); ++fitr) {
        IntPolyRef p = fitr->first;
        if (allPolys.find(p) == allPolys.end() ) {
          //unknown and learned not strict
          isStrict = false;
          
          all.addCol();
	  int j = cIdxToPoly.size();
          allPolys[p] = j;
          cIdxToPoly.push_back(p);
	  int val = fitr->second % 2 == 0 ? 2 : 1;
          newRowVec.push_back(val);
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
          else {
	    if (j >= newRowVec.size())
	    {
	      std::cerr << "j = " << j << ", newRowVec.size() = " << newRowVec.size() << std::endl;
	      throw TarskiException("Error in MatrixManager::addAtom!");
	    }
	    newRowVec[j] = 1;
	  }
        }
      }
      if (isStrict && isRow) {
	// TODO!!! If this comes with tf_orig being an equation ... then we already have a contradiction!
	
        if (getNumStrictCols() != 0) { // Fernando had == 0 ... is that even possible?
          // vector<char> sRowVec(newRowVec.begin(), newRowVec.begin()+1+strongMap.size());
          // strict.addRow(sRowVec);
	  strict.addRow(newRowVec);
	  all.addRow(newRowVec);
	  tB.push_back(rIdxToAtom.size()-1);
        }
      }
      else if (isRow)
	all.addRow(newRowVec);
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
    for (auto it = tB.begin(); it != tB.end(); ++it) {
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

  bool MatrixManager::isStrictPoly(IntPolyRef p)
  {
    return strongMap.find(p) != strongMap.end();
  }
    
  bool MatrixManager::isStrictAtom(TAtomRef t)
  {
    FactRef F = t->getFactors();
    auto itr = F->MultiplicityMap.begin(), end = F->MultiplicityMap.end();
    for (; itr != end; ++itr ) {
      IntPolyRef p = itr->first;
      if (! isStrictPoly(p)) //(strongMap.find(p) == strongMap.end())
        return false;
    }
    return true;
  }

}//end namespace
