#ifndef _HIT_SET_
#define _HIT_SET_
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>

/****************************************************
 * The basic representation here is 
 ****************************************************/
class HitProb
{
private:
  int n; // <-- number of elements in base set
  int r; // <-- number of subsets
  int *A; // the subsets (with 0's separating subsets)
  int *S; // starting indices for sets
  int *Z; // sizes of sets
public:
  int numElts() const { return n; }
  int numSets() const { return r; }
  int get(int j) const { return A[j]; }
  int setEnd(int i) const // returns the index of the 0 trailing set S_i
  {
    if (i == -1) return 0;
    return S[i];
  }
  int setSize(int i) const { return Z[i]; }

  // call fill to fill an empty hitset problem.
  // n - number of elements in bse set.  These are {1,2,...,n}
  // r - number of sets, i.e. we have S_0,S_1,...,S_{n-1}
  // V - V[i] represents set S_i
  //     NOTE: the last element of V[i] must be 0!
  void fill(const int n, const int r,
	    const std::vector< std::vector<int> > &V);

  /*  Read assumes a format like this
   4 6 <-- this means 4-elements (1,2,3,4), 6 subsets
   4 2 3 0
   1 3 0
   2 4 0
   1 2 4 0
   1 3 4 0
   3 4 0
  */
  void read(std::istream& in);
  void write(std::ostream& out);
};

// naiveSolve takes HitProb HP as input, and writes
// a minimum cardinality hitting set as output into H.
// This a
void naiveSolve(HitProb& HP, std::vector<int> &H);

// Check's whether H is a hitting set for Hp.
// returns 1 if successful.  Otherwise returns -i s.t. set S_i
// is not hit by H.
int check(HitProb& HP, std::vector<int> &H);

#endif
