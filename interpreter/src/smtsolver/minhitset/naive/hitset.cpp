#include "hitset.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>

static bool debug_flag = false;

void HitProb::fill(const int n, const int r,
		   const std::vector< std::vector<int> > &V)
{
  using namespace std;
  this->n = n;
  this->r = r;
  if (V.size() != r) {
    cerr << "HitProb::fill: r != V.size()" << endl;
    throw exception(); }

  int M = 1;  
  vector<int> startPos;
  for(int i = 0; i < V.size(); i++)
  {
    startPos.push_back(M);
    M += V[i].size();
    if (V[i].size() < 2 || V[i].back() != 0) {
    cerr << "HitProb::fill: each V[i] must have length >= 2 and have 0 as last value" << endl;
    throw exception(); }
  }
  S = new int[r+1];
  A = new int[M];
  Z = new int[M];
  A[0] = 0;
  int k = 1;
  for(int i = 0; i < V.size(); i++)
  {
    Z[i] = V[i].size();
    for(int j = 0; j < V[i].size(); j++)
      A[k++] = V[i][j];
    S[i] = k-1;
  }
  S[r] = M-1;
}
  
void HitProb::read(std::istream& in)
{
  using namespace std;
  in >> n >> r;
  int M = 1, x, count = 0;
  vector< vector<int> > V(r);
  while(count < r && (in >> x))
  {
    V[count].push_back(x);
    if (x == 0) count++;
  }
  fill(n,r,V);
}

void HitProb::write(std::ostream& out)
{
  int i = 1;
  for(int c = 0; c < r; )
  {
    out << A[i++] << ' ';
    if (A[i] == 0) c++;
  }  
  out << A[i] << std::endl;
}


void naiveSolve(HitProb& HP, std::vector<int> &H)
{
  const int N = HP.numElts();
  const int R = HP.numSets();
  bool *mask = new bool[N + 1];
  for(int i = 1; i <= N; i++)
    mask[i] = 0;
  int currSize = 0, bestSize = N + 1, count = 0;

  std::vector<int> S, whichSet;
  int i = 0, j = 0;
  while(true)
  {
    count++;
    //    std::cout << "i = " << i << " j = " << j << std::endl;
    if (i == R || currSize == bestSize)
    {
      if ( currSize < bestSize)
      {
	// new best hit set!
	if (debug_flag)
	{
	  for(int k = 1; k <= N; k++)
	    if (mask[k])
	      std::cout << ' ' << k;
	  std::cout << " : " << currSize << std::endl;
	}
	bestSize = currSize;
	H.resize(bestSize);
	for(int a = 0; a < S.size(); a++)
	  H[a] = HP.get(S[a]);
      }
      
      // pop_back until we get to an S_i we can try another hitting element from
      int x = 0, inew, jnew;
      while(!S.empty() && x == 0)
      {
	inew = whichSet.back(); whichSet.pop_back();
	int jold = S.back(); S.pop_back();
	mask[HP.get(jold)] = 0; currSize--;
	jnew = 1 + jold;
	x = HP.get(jnew);
      }
      if (S.empty() && x == 0) break;
      S.push_back(jnew);
      mask[x] = 1; currSize++;
      whichSet.push_back(inew);      
      i = inew + 1;
      j = HP.setEnd(inew);
    }
    else
    {
      //std::cerr << "i = " << i << std::endl;
      int jp = j, x;
      while((x = HP.get(++jp)) != 0 && !mask[x]);
      if (x == 0)
      {
	int z = HP.get(j+1);
	//std::cerr << "z = " << z << std::endl;
	mask[z] = 1; currSize++;
	S.push_back(j+1);
	whichSet.push_back(i);
	j = jp;
      }
      else
	j = HP.setEnd(i);
      i++;
    }
  }
  if (debug_flag) { std::cout << "count = " << count << std::endl; }
}

int check(HitProb& HP, std::vector<int> &H)
{
  const int N = HP.numElts();
  const int R = HP.numSets();

  // Set mask - mask[i] = 1 iff element i is in the hitting set H
  bool *mask = new bool[N + 1];
  for(int i = 1; i <= N; i++)
    mask[i] = 0;
  for(int i = 0; i < H.size(); i++)
    mask[H[i]] = 1;

  int completed = 0, count = 0;
  for(int k = 1; completed < R; k++)
  {
    const int x = HP.get(k);
    if (x == 0)
    {
      if (count == 0) return -completed;
      completed++;
      count = 0;
      //      std::cerr << " finished set " << completed-1 << std::endl;
    }
    else
    {
      count += (mask[x]);
      //      std::cerr << "val " << k << " = " << x << " (" << mask[x] << ")" << std::endl;
    }
  }
  return 1;
}
 
