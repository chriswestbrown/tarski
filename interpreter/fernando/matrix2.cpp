//TODO: Fix Reduced Gaussian Elimination
#include "matrixtest.h"

namespace tarski {
  /*
    A simple constructor which does nothing
  */
  Matrix2::Matrix2(){};
  /*
    A simple constructor which initializes the size of the std::vector
  */
  Matrix2::Matrix2(int rows, int cols): m(rows, std::vector<bool>(cols)){
    george.resize(rows);
    for(int i = 0; i < rows; i++) {
      std::vector<bool> tmp = george[i];
      tmp.resize(rows);
    }
  }
  /*
    A constructor which copies another Matrix2 object by copying m
  */
  Matrix2::Matrix2(const Matrix2 &M): m(M.m) {

  }

  int Matrix2::getNumRows() {return m.size();}
  int Matrix2::getNumCols() {
    if (m.size() == 0) return 0;
    else return m[0].size();
  }

  void Matrix2::write() {
    for (std::vector<std::vector<bool> >::iterator iter = m.begin(); iter != m.end(); ++iter){
      for (std::vector<bool>::iterator iter2 = iter->begin(); iter2 != iter->end(); ++iter2) {
        std::cout << *iter2 << " ";
      }
      std::cout << std::endl;
    }
  }

  //NOTE: You should only add vectors of size equal to getNumCols
  inline void Matrix2::addRow(std::vector<bool> vb) {
    m.push_back(vb);
  }

  inline void Matrix2::addCol() {
    for (int i = 0; i < m.size(); i++) {
      m[i].push_back(false);
      george[i].push_back(false);
    }
    george[m.size()-1][m.size()-1] = true;
  }

  inline void Matrix2::swapVal(bool val1, bool val2){
    if (val1 == val2) return;
    else {
      val1 = !val1;
      val2 = !val2;
    }
  }

  inline void Matrix2::swapCol(int col1, int col2, std::vector<std::vector<bool> >& matrix) {
    for (int i = 0; i < getNumRows(); i++) {
      swapVal(matrix[col1][i], matrix[col2][i]);
    }
  }

  inline std::vector<bool> Matrix2::getSpace() {
    return matrixSpace;
  }

  inline std::vector<bool> Matrix2::getStrictSpace() {
    return strictSpace;
  }

  inline void Matrix2::std::set(int i, int j, bool val) {
    m[i][j] = val;
  }


  inline bool Matrix2::doUnsat(int strictLimit) {
    createStrictMatrix(strictLimit);
    redGaussElimExp(ms, gs, strictSpace);
    return checkUnsat();
  }


  /*
    Creates the strict part of the matrix from the full matrix, where strictLimit is the largest column which represents something
    which is strict
  */
  void Matrix2::createStrictMatrix(int strictLimit) {
    ms.clear();
    gs.clear();
    int colSize = getNumCols();
    for (std::vector<std::vector<bool> >::iterator iter = m.begin(); iter != m.end(); ++iter) {
      bool valid = false; //valid is true if there is at least one true in the strict part
      std::vector<bool>::iterator rowIter = iter->begin();
      std::vector<bool> toAdd(strictLimit); // if we find at least one true in the strict part, we add this to ms
      int count = 0;
      while (count <= strictLimit) {
        if (*rowIter)
          valid = true;
        toAdd.push_back(*rowIter);
      }
      if (valid) //here we copy over the row
        ms.push_back(toAdd);
    }
    int j = ms.size();
    for (int i = 0; i < j; i++) {
      std::vector<bool> toPush(j);
      toPush[i] = true;
      gs.push_back(toPush);
    }
  }

  void Matrix2::gaussElimExplain(std::vector<std::vector<bool> >& m, std::vector<std::vector<bool> >& george) {
    std::vector<int> pivotRows;
    std::vector<int> pivotCols;
    int r = getNumRows();
    if (r == 0) return;
    int c = getNumCols();

    for(int i = 0; i < r; ++i)   {
      // Find next pivot row
      int wmin = INT_MAX, kmin, tmp=0;
      for(int k = i; k < r; ++k)
        if ((tmp = rowWeight(k, m,  true)) < wmin && tmp != 0) { wmin = tmp; kmin = k; }
      if (wmin == INT_MAX) { return; }
      swap(i, kmin, m); //do the original swap,
      swap(i, kmin, george); //then swap for the george matrix,
      pivotRows.push_back(kmin); //then log the original position
      // Find next pivot column & reduce other rows by pivot rows that aren't in pivot row list
      int j = 1;
      while(m[i][j] == 0) ++j;
      pivotCols.push_back(j);
      for(int k = i + 1; k < r; ++k)
        if (m[k][j]) {
          sumRows(k, j, m);
          sumRows(k, j, george);
        }
    }
  }


  void Matrix2::redGaussElimExp(std::vector<std::vector<bool> >& m, std::vector<std::vector<bool> >& george, std::vector<bool>& matrixSpace)
  {
    std::vector<int> pivotRows;
    std::vector<int> pivotCols;
    int r = getNumRows();
    if (r == 0) return;
    int c = getNumCols();

    for(int i = 0; i < r; ++i)   {
      // Find next pivot row
      int wmin = INT_MAX, kmin, tmp=0;
      for(int k = i; k < r; ++k) {
        if ((tmp = rowWeight(k, m, true)) < wmin && tmp != 0) { wmin = tmp; kmin = k; }
      }
      if (wmin == INT_MAX) { break; }
      swap(i, kmin, m); //do the original swap,
      swap(i, kmin, george); //then swap for the george matrix,
      pivotRows.push_back(kmin); //then log the original position
      // Find next pivot column & reduce other rows by pivot rows that aren't in pivot row list
      int j = 1;
      while(m[i][j] == 0) {++j; }
      pivotCols.push_back(j);
      for(int k = i + 1; k < r; ++k){
        if (m[k][j]) {
          sumRows(k, j, m);
          sumRows(k, j, george);
        }
      }
    }

    //END REG GAUSSIAN ELIMINATION, BEGIN REDUCING

    for(int r = pivotCols.size() - 1; r > 0; --r)
      {
        int p = pivotCols[r];
        for(int k = r-1; k >= 0; -- k) {

          if (m[k][p])
            {
              sumRows(k, r, m);
              sumRows(k, r, george);
            }
        }
      }
    //Now time to use pivotRows to undo all of our swaps
  
    for (int i = pivotRows.size()-1; i >= 0; i--) {
      swap(i, pivotRows[i], m);
      swap(i, pivotRows[i], george);
    }
  

    matrixSpace.resize(m.size());
    for (int i = 0; i < m.size(); i++) {
      if (m[i][i]) matrixSpace[i] = true;
    }
  }

  inline bool Matrix2::checkUnsat() {
    if (ms[0][0] == false) return false;
    for (int i = 1; i < ms[i].size(); i++) {
      if (ms[0][i] == true) return false;
    }
    return true;
  }

  /* Use to swap rows */
  inline void Matrix2::swap(int i, int j, std::vector< std::vector<bool> >& v) {
    std::vector<bool> tmp(v[i]);
    v[i] = v[j];
    v[j] = tmp;
  }


  inline int Matrix2::rowWeight(int i, std::vector<std::vector<bool> >& v, bool ignoreFirstColumn) {
    int sum = 0;
    int j = 0;
    if (ignoreFirstColumn) j = 1;
    while (j < v[i].size()) {
      if (v[i][j]) sum++;
      j++;
    }
    return sum;
  }

  void Matrix2::sumRows(int i, int j, std::vector<std::vector<bool> >& v) {
    std::vector<bool>::iterator itr = v[i].begin();
    std::vector<bool>::iterator itr2 = v[j].begin();
    while (itr !=v[i].end() && itr2 != v[j].end()) {
      if (*itr && *itr2)
        *itr = false;
      else if (*itr2)
        *itr = true;
      ++itr;
      ++itr2;
    }

  }

  /*
    If strict is true, then do elimination on ms, which is std::set by val
    Otherwise, do elimination on the entire matrix
  */
  void Matrix2::doElim(bool strict, int val) {
    if (strict) {
      createStrictMatrix(val);
      redGaussElimExp(ms, gs, strictSpace);
      return;
    }
    else {
      redGaussElimExp(m, george, matrixSpace);
    }
  }




  int main() {
    srand(time(NULL));
    Matrix2 M(8, 8);
    for (int i = 0; i < 20; i++) {
      int a = rand() % 5;
      int b = rand() % 5;
      M.std::set(a, b, true);
    }
    std::cout << "Before Elimination\n";
    M.write();
    std::cout << "After Elimination\n";
    M.doElim(false, 0);
    M.write();
  }

}//end namespace
