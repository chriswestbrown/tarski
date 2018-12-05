#ifndef _DYNAMIC_MATRIX_H
#define _DYNAMIC_MATRIX_H
#include <vector>
#include <iostream>
#include <vector>
#include <climits>
#include <stdlib.h>
#include <assert.h>

namespace tarski {


  class DMatrix {
  public:
    DMatrix(int cols); //    A simple constructor which does nothing
    DMatrix(int rows, int cols);
    DMatrix(const DMatrix &M);
    ~DMatrix();

    inline size_t getNumRows() const { return m.size(); }
    inline size_t getNumCols() const { return (m.size() > 0)
        ? m[0].size() :  0; }
    inline const std::vector<char>& getRow(int i) const { return m[i]; }
    void write() const; //Exactly what it says on the tin
    inline void set(int i, int j, char val) { m[i][j] = val; }//Set the value of some element in the total matrix
    inline const std::vector<std::vector<bool> >& getComp() const  { return comp; }
    inline const std::vector<std::vector<char> >& getMatrix() const { return m; }


    inline void swapCol(int col1, int col2) {
      for (int i = 0; i < getNumRows(); i++) {
        swapVal(m[i][col1], m[i][col2]);
      }
    }
    inline void swapBack(int col) {
      int col2 = getNumCols()-1;
      swapCol(col, col2);
    }
    //NOTE: excess elements will be destroyed, but only in this copy!

    inline void addCol() {
      for (int i = 0; i < m.size(); i++) {
        m[i].push_back(false);
      }
    }

    inline void toMod2() {
      for (std::vector<std::vector<char> >::iterator bigItr = m.begin();
           bigItr !=  m.end(); ++bigItr) {
        for (std::vector<char>::iterator itr = bigItr->begin();
             itr != bigItr->end(); ++itr ) {
          *itr = *itr % 2;
        }
      }
    }
    inline bool isPivotCol(int i) {
      return pivotCols[i];
    }



    void addRow(const std::vector<char>& vb);
    void doElim(); //FOR TESTING PURPOSES ONLY

    // Input : vc is the row-vector to be reduced. rows should be empty initially
    // Output/SideEffects: vc is reduced by the rows of the matrix, and 'rows'
    // contains the indices of all the rows that were used in the reduction
    void reduceRow (std::vector<char>& vc,
                    std::vector<int>& rows) const;

    

  private:
    std::vector< std::vector<char > > m; //the strict matrix
    std::vector < std::vector<bool > >comp; //the strict companion matrix

    // I believe: Let i be the index of a column (i > 0, since we never reduce
    // the the relop column, which is i = 0).  pivotCols[i] gives the index of
    // the row in m (the strict matrix) that is the pivot for column i.  I.e.,
    // m[i] = 1, and m[i] = 0 for all "previous columns".
    // IMPORTANT: elimination is done left-to-right, i.e. columns 1,2,..., m.size()-1.
    //            that's why it is OK to reduce checking left-to-right for pivots.
    std::vector<int> pivotCols;


    inline void swapVal(bool&  val1, bool& val2){
      if (val1 == val2) return;
      val1 = !val1;
      val2 = !val2;
    }
    inline void swapVal(char& val1, char& val2){
      char tmp = val2;
      val2 = val1;
      val1 = (tmp == 2) ? 0 : tmp;
    }
    void gaussElimExplain();
    void redGaussElimExp();
    bool checkUnsat();


    template <class T>  int rowWeight(const std::vector<T>& v, bool ignoreFirstColumn) {
      int sum = 0;
      int j = 0;
      if (ignoreFirstColumn) j = 1;
      while (j < v.size()) {
        if (v[j]) sum++;
        j++;
      }
      return sum;
    }

    template <class T> void sumRows(std::vector<T>& v,
                                    const std::vector<T>& toAdd) const {
      typename std::vector<T>::iterator itr = v.begin();
      typename std::vector<T>::const_iterator itr2 = toAdd.begin();
      while (itr !=v.end() && itr2 != toAdd.end() ) {
        if (*itr && *itr2)
          *itr = false;
        else if (*itr2)
          *itr = true;
        ++itr;
        ++itr2;
      }
    }


    template <class T> void sumRows(int i, int j, std::vector<std::vector<T> >& v) {
      typename std::vector<T>::iterator itr = v[i].begin();
      typename std::vector<T>::iterator itr2 = v[j].begin();
      while (itr !=v[i].end() ) {
        if (*itr && *itr2)
          *itr = false;
        else if (*itr2)
          *itr = true;
        ++itr;
        ++itr2;
      }

    }
    
    inline void swap(int i, int j, std::vector< std::vector<char> >& v) {
      std::vector<char>  tmp(v[i]);
      v[i] = v[j];
      v[j] = tmp;
    }
    inline void swap(int i, int j, std::vector< std::vector<bool> >& v) {
      std::vector<bool>  tmp(v[i]);
      v[i] = v[j];
      v[j] = tmp;
    }
    inline void swap(int i, int j, std::vector<int >& v) {
      if (i == j) return;
      int tmp(i);
      v[i] = v[j];
      v[j] = tmp;
    }


  };







}//end namespace

#endif
