#include <iostream>
#include <fstream>
#include <vector>
#include <limits.h>
#include <string>
#include <stdlib.h>

#ifndef DBV_H
#define DBV_H

namespace tarski {

// This is "dynamic" in the sense of getting arbitrary size at run-time
// not of expanding and shrinking
class DBV
{
protected:
  typedef unsigned char blocktype;
  static const blocktype altblock0 = 0x55, altblock1 = 0x55 << 1;
  static const int blocksize = (sizeof(blocktype)*8);
  int n, numBlocks;
  blocktype* buff;
  
public:
  class iterator
  {
  public:
    iterator(blocktype* p, blocktype m) { parent = p; posMask = m; }
    iterator& operator=(bool b) { if (b) *parent |= posMask; else *parent &= ~posMask; return *this; }
    operator bool() { return bool(*parent & posMask); }
    iterator& operator++() { posMask <<= 1; if (posMask = 0) { posMask = 1; ++parent; } return *this; }
    iterator& operator--() 
    { posMask >>= 1; if (posMask = 0) { posMask = 1 << (blocksize - 1); --parent; } return *this; }
  private:
    blocktype* parent;
    blocktype posMask;
  };

  DBV() { n = numBlocks = 0; buff = 0; }

  DBV(const DBV &B)
  {
    n = B.n; numBlocks = B.numBlocks; 
    if (numBlocks == 0)
      buff = 0;
    else
    {
      buff = new blocktype[numBlocks];
      for(int i = 0; i < numBlocks; ++i) this->buff[i] = B.buff[i];
    }
  }

  DBV(int s) 
  { 
    n = s; 
    int block = n/(sizeof(blocktype)*8);
    int offset = n%(sizeof(blocktype)*8);
    numBlocks = (offset == 0 ? block : block + 1);
    buff = new blocktype[numBlocks];
    for(int i = 0; i < numBlocks; ++i) buff[i] = 0;
  }
  void resize(int s)
  {
    if (numBlocks == 0)
    {
      n = s; 
      int block = n/(sizeof(blocktype)*8);
      int offset = n%(sizeof(blocktype)*8);
      numBlocks = (offset == 0 ? block : block + 1);
      buff = new blocktype[numBlocks];
      for(int i = 0; i < numBlocks; ++i) buff[i] = 0;
    }
    else
    {
      //throw TarskiException("Resize of non-empty DBV not yet supported!");
    }
  }

  ~DBV() { delete [] buff; }
  
  inline int size() const { return n; }
  
  bool get(int i) const { return buff[i/blocksize] & (1 << i%blocksize); }
  bool set(int i, bool b)
  {
     blocktype &B = buff[i/blocksize];
     int m = 1 << i%blocksize;
     if (b) B |= m; else B &= ~m; 
     return b;
  }




  iterator operator[](int i) const
  {
    int block = i/(sizeof(blocktype)*8);
    int offset = i%(sizeof(blocktype)*8);
    return iterator(buff + block,1 << offset);
  }
  
  DBV& operator+=(const DBV &B)
  {
    for(int i = 0; i < numBlocks; ++i) this->buff[i] ^= B.buff[i];
    return *this;
  }

  DBV& operator=(const DBV &B)
  {
    if (&B == this) return *this;
    if (n != B.n)
    {
      if (numBlocks != 0) delete [] buff;
      n = B.n; numBlocks = B.numBlocks; 
      buff = new blocktype[numBlocks];
    }
    for(int i = 0; i < numBlocks; ++i) this->buff[i] = B.buff[i];
    return *this;
  }
  
  int isZero() const { int i = 0; while(i < numBlocks && !(buff[i])) ++i; return i == numBlocks; }

  bool operator==(const DBV &B) const
  {
    if (numBlocks != B.numBlocks) return false;
    int i = 0; while(i < numBlocks && buff[i] == B.buff[i]) ++i; return i == numBlocks; 
  }

  bool operator<(const DBV &B) const // this is lex order
  {
    int K = std::min(numBlocks,B.numBlocks);
    int i = 0; while(i < K && buff[i] == B.buff[i]) ++i;
    if (i < K) { return buff[i] < B.buff[i]; }
    if (i == std::max(numBlocks,B.numBlocks)) return false;
    return numBlocks < B.numBlocks;
  }

  int weight(bool ignoreFirstColumn) const // returns number of non-zero entries
  {
    int w = 0;
    for(blocktype *p = buff, *eob = buff + numBlocks; p != eob; ++p)
      for(blocktype m = 1; m != 0; m <<= 1)
	w += (m&*p ? 1 : 0);
    if (ignoreFirstColumn && (buff[0] & 1))
      return w-1;
    else
      return w;
  }

  /*
    Returns -1 if there is not only one non-zero element
    Otherwise, returns the index of the non-zero element
   */
  short getNonZero(bool ignoreFirstColumn) {
    if (weight(ignoreFirstColumn) != 1) return -1;
    if (!ignoreFirstColumn) {
      for (int i = 0; i < size(); i++) {
        if (get(i) != 0) return i;
      }
    }
    else {
      for (int i = 1; i < size(); i++) {
        if (get(i) != 0) return i;
      }
    }
    return -1; //shouldn't get here
  }

  void write() const 
  {
    for(int i = 0; i < n; ++i)
      std::cout << bool((*this)[i]);
  }

  friend void swap(DBV &A, DBV &B);
};


void swap(DBV &A, DBV &B);

class Matrix : public std::vector<DBV>
{
  int r, c;
public:


  Matrix(int rows, int cols)
  {
    r = rows; c = cols;
    resize(r); for(int i = 0; i < r; ++i) (*this)[i].resize(c);
  }

  int getNumRows() {return r;}

  int getNumCols() {return c;}

  void write() {
    for (unsigned int i = 0; i < size(); i++) {
      at(i).write(); std::cout << std::endl;
    }
  }

  Matrix(const Matrix &M) {
    r = M.size();
    c = M[0].size();
    resize(r);
    for (int i = 0; i < r; ++i) {
      DBV d(M[i]);
      (*this)[i] = d;
    }
  }

};






/*
  Input: String name, a file which dictates a bit std::vector to be read
  Output: A properly coded matrix
*/
std::vector<DBV> * read(std::string name);

void gaussianElimination(std::vector< DBV > &M, std::vector<int> &pivotCols, bool ignoreFirstColumn = false);

/*
  Outputs an identity matrix whose size is given by the std::vector toSize
*/
std::vector < DBV > * identityMatrix(std::vector < DBV> * toSize);
std::vector < DBV > idMatrix(std::vector <DBV> toSize);

/*
  INPUT:
  M, the matrix to do elimination on
  george, the identity matrix
  pivotCols, an empty std::vector
  pivotRows, an empty std::vector
  
  Output:
  M, the reduced but not triangular matrix
  george, the companion matrix to M
  pivotCols, which describes the hypothetical pivots of the columns
  pivotRows, which describes the hypothetical pivots of the rows
 */
void gaussianEliminationExplain(std::vector< DBV > &M, std::vector<DBV> &george, std::vector<int> &pivotCols, std::vector<int> &pivotRows, bool ignoreFirstColumn = false);

void reducedGaussExplain(std::vector< DBV > &M, std::vector<DBV> &george, std::vector<int> &pivotCols, std::vector<int> &pivotRows, bool ignoreFirstColumn = false);
/*
  Prints a message if the matrix indicates UNSAT
  INPUT:
  M, the matrix
  george, the companion matrix to M

  OUTPUT:
  If unsat, return unsat. Indicies of the rows responsible placed in the std::vector reasons

  If verbose is true, say things to std::cout.
 */
bool detectUnsat(std::vector< DBV > &M, std::vector < DBV> george, std::vector<int> &reasons, bool verbose);

/*
  Same as previous, but uses the std::vector og to display the
  actual rows which produced a conflict as opposed to only their indices.
 */
bool detectUnsat(std::vector<DBV> &M, std::vector <DBV> george, std::vector<int> &reasons, std::vector <DBV> og);

void reducedGauss(std::vector< DBV > &M, std::vector<int> &pivotCols, bool ignoreFirstColumn = false);

DBV reduce(Matrix &M, std::vector<int> &piv, const DBV &v);

 DBV reduceExplain(Matrix &M, std::vector<int> &piv, const DBV &v, std::vector<int>& explain);

void write(std::vector<DBV> &M);

class FunnyVec : public DBV
{
public:
  FunnyVec() : DBV() { }
  FunnyVec(int n) : DBV(2*n) { }
  void resize(int n) { DBV::resize(2*n); }
  static int norm(int x) { return x == 0 ? 0 : 2 + x % 2; }
  void set(int i, int x) 
  { 
    int y = norm(x);
    DBV::set(2*i,y&1);
    DBV::set(2*i+1,y&2);
  }
  int get(int i) const
  {
    DBV::iterator itr = (*this)[2*i];
    int i0 = bool(itr);
    int i1 = bool(++itr);
    return DBV::get(2*i) + 2*DBV::get(2*i+1);
  }
  void write() const
  {
    for(int i = 0, x; i < n/2; ++i)
      std::cout << (x = get(i),(x == 3 ? 1 : x));
    std::cout << std::endl;
  }

  FunnyVec& operator+=(const FunnyVec &B)
  {
    for(int i = 0; i < numBlocks; ++i) 
      this->buff[i] = 
	(this->buff[i] & altblock0) ^ (B.buff[i] & altblock0) |
	(this->buff[i] & altblock1) | (B.buff[i] & altblock1);
    return *this;
  }
};

}//end namespace tarski

#define DEBUG_NO_MAIN
#ifndef DEBUG_NO_MAIN
int main(int argc, char * argv[])
{
  std::string fname;
  std::cin >> fname;
  std::vector<DBV> * n = read(fname.c_str());
  std::vector<DBV> * og = read(fname.c_str());
  std::cout << "ORIGINAL---------------------" << std::endl;
  write(*n);
  std::cout << "ORIGINAL---------------------" << std::endl;
  std::vector<DBV> * georgie = identityMatrix(n);
  std::vector<int> a;
  std::vector<int> b;
  reducedGaussExplain(*n, *georgie, a, b);

  std::cout << "GEORGE-----------------------" << std::endl;
  write(*georgie);
  std::cout << "GAUSS------------------------" << std::endl;
  write(*n);

  detectUnsat(*n, *georgie, *og);
  return 0;
}
#endif


#endif
