//TODO: Fix Reduced Gaussian Elimination
#include "dmatrix.h"
#include <assert.h>

using namespace std;

namespace tarski {
  /*
    A simple constructor which does nothing
  */
  DMatrix::DMatrix(int cols) : pivotCols(cols,-1)
  {
    // cerr << "DMatrix() " << this << endl;
  };

  /*
    A simple constructor which initializes the size of the std::vector
  */
  DMatrix::DMatrix(int rows, int cols):
    m(rows, std::vector<char>(cols)), comp(rows, std::vector<bool>(rows)),
    pivotCols(cols,-1)
  {
    // cerr << "DMatrix(rows,cols) " << this << endl;
    for (size_t i = 0; i < comp.size(); i++) {
      comp[i][i] = true;
    }
  }
  /*
    A constructor which copies another DMatrix object by copying m
  */
  DMatrix::DMatrix(const DMatrix &M): m(M.m), comp(M.comp), pivotCols(M.pivotCols)
  {
    //cerr << "DMatrix(M) " << this << endl;
  }

  void DMatrix::write() const {
    for (size_t i = 0; i < m.size(); i++) {
      for (size_t j = 0; j < m[i].size(); j++) {
        std::cerr << (int) m[i][j] << " ";
      }
      std::cout << std::endl;
    }
  }
  





  void DMatrix::gaussElimExplain()
  {
    cerr << "gaussElimExplain " << this << endl;
    std::vector<int> pivotRows;
    pivotCols.clear();
    int r = getNumRows();
    if (r == 0) return;
    int c = getNumCols();

    for(int i = 0; i < r; ++i)   {
      // Find next pivot row
      int wmin = INT_MAX, kmin, tmp=0;
      for(int k = i; k < r; ++k)
        if ((tmp = rowWeight(m[k],  true)) < wmin && tmp != 0) { wmin = tmp; kmin = k; }
      if (wmin == INT_MAX) { return; }
      swap(i, kmin, m); //do the original swap,
      swap(i, kmin, comp); //then swap for the comp matrix,
      pivotRows.push_back(kmin); //then log the original position
      // Find next pivot column & reduce other rows by pivot rows that aren't in pivot row list
      int j = 1;
      while(m[i][j] == 0) ++j;
      pivotCols.push_back(j);
      for(int k = i + 1; k < r; ++k)
        if (m[k][j]) {
          sumRows(k, j, m);
          sumRows(k, j, comp);
        }
    }
  }


  void DMatrix::redGaussElimExp()
  {
    // cerr << "redGaussElimExplain " << this << endl;
    pivotCols.clear();
    std::vector<int> pivotRows;
    int r = getNumRows();
    if (r == 0) return;

    for(int i = 0; i < r; ++i)   {

      // Find next pivot row
      int wmin = INT_MAX, kmin, tmp = 0;
      for(int k = i; k < r; ++k) {
        if ((tmp = rowWeight(m[k], true)) < wmin && tmp != 0) { wmin = tmp; kmin = k; }
      }
      if (wmin == INT_MAX) { break; }
      swap(i, kmin, m); //do the original swap,
      swap(i, kmin, comp); //then swap for the comp matrix,
      pivotRows.push_back(kmin); //then log the original position
      // Find next pivot column & reduce other rows by pivot rows that aren't in pivot row list
      int j = 1;
      while(m[i][j] == 0) { ++j; }
      pivotCols.push_back(j);
      for(int k = i + 1; k < r; ++k){
        if (m[k][j]) {
          sumRows(k, i, m);
          sumRows(k, i, comp);
        }
      }
    }

    //END REG GAUSSIAN ELIMINATION, BEGIN REDUCING

    for(int r = pivotCols.size() - 1; r > 0; --r) {
      int p = pivotCols[r];
      for(int k = r-1; k >= 0; -- k) {

        if (m[k][p])
          {
            sumRows(k, r, m);
            sumRows(k, r, comp);
          }
      }
    }
    //Now time to use pivotRows to undo all of our swaps
    for (int i = pivotRows.size()-1; i >= 0; i--) {
      swap(i, pivotRows[i], m);
      swap(i, pivotRows[i], comp);
    }
    pivotCols.clear();
    pivotCols.resize(getNumCols(), -1);
    for (int i = 0; i < m.size(); i++) {
      for (int j = 1; j < m[i].size(); j++) {
        if (m[i][j]) { pivotCols[j] = i; break; }
      }
    }
  }

  void DMatrix::reduceRow (std::vector<char>& vc, std::vector<int>& rows) const
  {
    // start at i=1 because we don't want to reduce the relop entry
    for (size_t i = 1; i < getNumCols(); i++)
    {
      // if vc has a non-zero entry in column i
      if (vc[i]) {
        if (pivotCols[i] == -1) break;
        sumRows(vc, m[pivotCols[i]]);
        rows.push_back(pivotCols[i]);
      }
    }
  }

  bool DMatrix::checkUnsat() {
    if (m[0][0] == false) return false;
    for (int i = 1; i < m[i].size(); i++) {
      if (m[0][i] == true) return false;
    }
    return true;
  }



  void DMatrix::doElim() {
    redGaussElimExp();
  }


  void DMatrix::addRow(const std::vector<char>& vb)
  {
    std::vector<char> vc(vb); 
    if (m.size() > 0)
      vc.resize(m[0].size(), 0);
    m.push_back(vc);
    #pragma omp prallel for
    for (size_t i = 0; i < comp.size(); i++) {
      comp[i].push_back(false);
    }
    std::vector<bool> b(m.size(), false);
    b[m.size()-1] = true;
    comp.push_back(b);
  }


  DMatrix::~DMatrix() {
    comp.clear();
    m.clear();
  }

}//end namespace


/*
int main() {
  srand(time(NULL));
  tarski::DMatrix M(8, 8);
  for (int i = 0; i < 20; i++) {
    int a = rand() % 5;
    int b = rand() % 5;
    M.set(a, b, true);
  }
  std::cout << "Before Elimination\n";
  M.write();
  std::cout << "After Elimination\n";
  M.doElim(false, 0);
  M.write();
  return 0;
}
*/
