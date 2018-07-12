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
    DMatrix(); //    A simple constructor which does nothing
    DMatrix(int rows, int cols);
    DMatrix(const DMatrix &M);

    inline int getNumRows() const { return m.size();}
    inline int getNumCols() const { return (m.size() > 0) ? m[0].size() :  0; }
    void write() const; //Exactly what it says on the tin
    inline void set(int i, int j, bool val) { m[i][j] = val; }//Set the value of some element in the total matrix
    inline const std::vector<std::vector<bool> >& getComp() const  { return comp; }
    inline const std::vector<std::vector<char> >& getMatrix() const { return m; }
    inline void swapCol(int col1, int col2) {
      for (int i = 0; i < getNumRows(); i++) {
        swapVal(m[col1][i], m[col2][i]);
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



    void addRow(std::vector<char> vb);
    void doElim(); //FOR TESTING PURPOSES ONLY




  private:
    std::vector< std::vector<char > > m; //the strict matrix
    std::vector < std::vector<bool > >comp; //the strict companion matrix
    inline void swapVal(bool&  val1, bool& val2){
      if (val1 == val2) return;
      else 
        val1 = !val1;
        val2 = !val2;
    }
    inline void swapVal(char& val1, char& val2){
      char tmp = val2;
      val2 = val1;
      val1 = tmp;
    }
    void gaussElimExplain();
    void redGaussElimExp();
    bool checkUnsat();


    template <class T> int rowWeight(int i, std::vector<std::vector<T> >& v, bool ignoreFirstColumn) {
      int sum = 0;
      int j = 0;
      if (ignoreFirstColumn) j = 1;
      while (j < v[i].size()) {
        if (v[i][j]) sum++;
        j++;
      }
      return sum;
    }


    template <class T> void sumRows(int i, int j, std::vector<std::vector<T> >& v) {
      typename std::vector<T>::iterator itr = v[i].begin();
      typename std::vector<T>::iterator itr2 = v[j].begin();
      while (itr !=v[i].end() && itr2 != v[j].end()) {
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
  };







}//end namespace

#endif
