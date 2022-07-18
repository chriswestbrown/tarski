#include "DynamicBitVector.h"
#include <algorithm>
#include <unordered_set>
using namespace std;


namespace tarski {
void swap(DBV &A, DBV &B)
{
  if (&A != &B)
  {
    std::swap(A.n,B.n);
    std::swap(A.numBlocks,B.numBlocks);
    std::swap(A.buff,B.buff);
  }
}

vector<DBV> * read(std::string name){
  //an example file
  //c
  //01010101
  //01010101
  //01010101
  std::ifstream fin(name.c_str());
  std::string tmp;
  int c;
  int size = -1;
  //get num columns as first line

  //Read line by line
  //Construct a Matrix
  vector <DBV> * toRet = new vector<DBV>();
  while (fin >> tmp){
    c = tmp.size();
    if (size == -1)
      size = c;
    else if (c != size) {
      cerr << "NOT A MATRIX, YOU JERK";
      exit(1);

    }
    DBV * d = new DBV(c);
    for (int i = 0; i < c; i++) {
      bool val = false;
      if (tmp[i] == '1')
        val = true;
      d->set(i, val);
        }
    toRet->push_back(*d);
  }
  return toRet;
}


void gaussianElimination(vector< DBV > &M, vector<int> &pivotCols, bool ignoreFirstColumn)
{
  int r = M.size();// number of rows
  if (r == 0) return;
  int c = M[0].size(); // number of columns

  for(int i = 0; i < r; ++i)
  {
    // Find next pivot row
    int wmin = INT_MAX, kmin, tmp=0;
    for(int k = i; k < r; ++k)
      if ((tmp = M[k].weight(ignoreFirstColumn)) < wmin && tmp != 0) { wmin = tmp; kmin = k; }
    //    if (wmin == 0) { return; }
    if (wmin == INT_MAX) { return; }

    swap(M[i],M[kmin]);
    // Find next pivot column & reduce other rows
    int j = ignoreFirstColumn ? 1 : 0; while(M[i].get(j) == 0) ++j;
    pivotCols.push_back(j);
    for(int k = i + 1; k < r; ++k)
      if (M[k][j]) M[k] += M[i];
  }
}



vector < DBV > * identityMatrix(vector < DBV> * toSize)
{
  vector <DBV> * toRet = new vector<DBV>();
  int size = toSize->size();
  for (int i = 0; i < size; i++){
    DBV toAdd(size);
    toAdd.set(i, true);
    toRet->push_back(toAdd);
  }
  return toRet;
}

vector < DBV > idMatrix(vector <DBV> toSize) {

  int size = toSize.size();
  vector<DBV> toRet(size);
  for (int i = 0; i < size; i++) {
    DBV toAdd(size);
    toAdd.set(i, true);
    toRet[i] = toAdd;
  }
  return toRet;


}


void gaussianEliminationExplain(vector< DBV > &M, vector<DBV> &george, vector<int> &pivotCols, vector<int> &pivotRows, bool ignoreFirstColumn)
{
  int r = M.size();// number of rows
  if (r == 0) return;
  int c = M[0].size(); // number of columns

  for(int i = 0; i < r; ++i)
    {
      // Find next pivot row
      int wmin = INT_MAX, kmin, tmp=0;
      for(int k = i; k < r; ++k)
        if ((tmp = M[k].weight(ignoreFirstColumn)) < wmin && tmp != 0) { wmin = tmp; kmin = k; }
      if (wmin == INT_MAX) { return; }
      swap(M[i], M[kmin]); //do the original swap,
      swap(george[i], george[kmin]); //then swap for the george matrix,
      pivotRows.push_back(kmin); //then log the original position
      // Find next pivot column & reduce other rows by pivot rows that aren't in pivot row list
      int j = ignoreFirstColumn ? 1 : 0; while(M[i].get(j) == 0) ++j;
      pivotCols.push_back(j);
      for(int k = i + 1; k < r; ++k)
        if (M[k][j])
          {
            M[k] += M[i];
            george[k] += george[i];

          }
    }
}

void reducedGaussExplain(vector< DBV > &M, vector<DBV> &george, vector<int> &pivotCols, vector<int> &pivotRows, bool ignoreFirstColumn)
{
  gaussianEliminationExplain(M, george, pivotCols, pivotRows, ignoreFirstColumn);

  for(int r = pivotCols.size() - 1; r > 0; --r)
    {
      int p = pivotCols[r];
      for(int k = r-1; k >= 0; -- k) {

        if (M[k][p])
          {
            M[k] += M[r];
            george[k] += george[r];
          }
      }
    }
  //Now time to use pivotRows to undo all of our swaps
  for (int i = pivotRows.size()-1; i >= 0; i--) {
    swap(M[i], M[pivotRows[i]]);
    swap(george[i], george[pivotRows[i]]);
  }
}


bool detectUnsat(vector< DBV > &M, vector < DBV> george, vector <int> &reasons, bool verbose){
  for (unsigned int i = 0; i < M.size(); i++){

    if (M[i][0] == true) {
      bool UNSAT = true;

      if (UNSAT) {
        for (int j = 1; j < M[i].size(); j++){
          if (M[i][j] == true) {
            UNSAT = false;
          }
        }
      }

      if (UNSAT) {
        if (verbose) {
          cout << endl << endl;
          cout << "UNSAT on row " << i << endl;
          cout << "Core: " << endl;
          for (int j = 0; j < george[i].size(); j++){
            if (george[i][j]) {
              cout << "Row " << j;
              cout << endl;
              reasons.push_back(j);
            }
          }
          cout << endl;
        }
        else {
          for (int j = 0; j < george[i].size(); j++){
            if (george[i][j]) {
              reasons.push_back(j);
            }
          }
        }
        return true;
      }
    }
  }
  if (verbose) {
    cout << "SAT";
    cout << endl;
  }
  return false;
  
}


bool detectUnsat(vector<DBV> &M, vector <DBV> george, vector <int> &reasons,  vector < DBV> og, bool verbose) {

  for (unsigned int i = 0; i < M.size(); i++){

    if (M[i][0] == true) {
      bool UNSAT = true;

      if (UNSAT) {
        for (int j = 1; j < M[i].size(); j++){
          if (M[i][j] == true) {
            UNSAT = false;
          }
        }
      }

      if (UNSAT) {
        if (verbose) {
          cout << endl << endl;
          cout << "UNSAT on row " << i << endl;
          cout << "Core: " << endl;
          
          for (int j = 0; j < george[i].size(); j++){
            if (george[i][j]) {
              if (j < 10) cout << "Row " << j << " : ";
              else cout << "Row " << j << ": ";
              for (int k = 0; k < og[j].size(); k++)
                cout << og[j][k];
              cout << endl;
              reasons.push_back(j);
            }
          }
        }
        else {
          for (int j = 0; j < george[i].size(); j++){
            if (george[i][j]) {
              reasons.push_back(j);
            }
          }
        }
        return true;
      }
    }
  }
  
  if (verbose) {
    cout << "SAT";
    cout << endl;
  }
  return false;
}

void reducedGauss(vector< DBV > &M, vector<int> &pivotCols, bool ignoreFirstColumn)
{
  gaussianElimination(M,pivotCols,ignoreFirstColumn);
  for(int r = pivotCols.size() - 1; r > 0; --r)
  {
    int p = pivotCols[r];
    for(int k = r-1; k >= 0; -- k)
      if (M[k][p]) M[k] += M[r];
  }
}

DBV reduce(Matrix &M, vector<int> &piv, const DBV &v)
{
  DBV u;
  u = v;
  for(unsigned int i = 0; i < piv.size(); ++i)
    if (u.get(piv[i]) == true)
      u += M[i];
  return u;
}



void write(vector<DBV> &M)
{
  for(unsigned int i = 0; i < M.size(); ++i)
  {
    if (i < 10)
      cout << i << " : ";
    else
      cout << i << ": ";

    M[i].write(); cout << endl;
  }
}
}//end namespace tarski
