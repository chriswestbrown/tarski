
#include <vector>
#include <iostream>
#include <vector>
#include <climits>
#include <stdlib.h>

namespace tarski {
  class Matrix2 {


  public:
    Matrix2(); //    A simple constructor which does nothing
    Matrix2(int rows, int cols); //    A simple constructor which initializes the size of the std::vector
    Matrix2(const Matrix2 &M); //    A constructor which copies another Matrix2 object by copying m

    int getNumRows(); //Exactly what it says on the tin
    int getNumCols(); //Exactly what it says on the tin
    void write(); //Exactly what it says on the tin
    inline void addRow(std::vector<bool>); //Adds a row to the matrix given
    inline void addCol(); //Adds a column to the end of the matrix
    inline std::vector<bool> getSpace(); //if m[i][i] is true post elimination, then returned[i] is true
    inline std::vector<bool> getStrictSpace(); //if ms[i][i] is true post elimination, then returned[i] is true
    inline void std::set(int, int, bool); //Set the value of some element in the total matrix
    inline bool doUnsat(int strictLimit); //Does the UNSAT check part of BB (eg, create a strict matrix, GE)

    void doElim(bool, int); //FOR TESTING PURPOSES ONLY
    
  private:
    std::vector< std::vector<bool> > ms; //the strict matrix
    std::vector < std::vector<bool> > gs; //the strict george matrix
    std::vector < std::vector<bool> > m; //the whole matrix
    std::vector < std::vector<bool> > george; //the whole george matrix
    std::vector<bool> matrixSpace; //Checking this is only valid once reduced gaussian elimination has been done!
    std::vector<bool> strictSpace; //Ditto, for checkUnsat

    inline void swapVal(bool, bool); //Swaps two values
    inline void swapCol(int, int, std::vector<std::vector<bool > >&); //Swaps two columns in the target matrix
    void createStrictMatrix(int);
    void gaussElimExplain(std::vector<std::vector<bool> >&, std::vector<std::vector<bool> >&);
    void redGaussElimExp(std::vector<std::vector<bool> >&, std::vector<std::vector<bool> >&, std::vector<bool>&);
    inline bool checkUnsat();
    inline void swap(int, int, std::vector< std::vector<bool> >&);
    inline int rowWeight(int, std::vector<std::vector<bool> >&, bool);
    void sumRows(int, int, std::vector<std::vector<bool> >&);

  };
}//end namespace
