#include "hitset.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>

int main(int argc, char** argv)
{
  using namespace std;
  if (argc < 2) { cout << "usage: \n" << endl; return 0; }
  ifstream fin(argv[1]);
  if (!fin) { cout << "file '" << argv[1] << "' not found!" << endl; return 1; }

  HitProb HP;
  HP.read(fin);
  HP.write(cout);

  vector<int> H;
  naiveSolve(HP,H);

  for(int i = 0; i < H.size(); i++)
    cout << H[i] << ' ';
  cout << ": " << H.size() << endl;

  cout << "check : " << check(HP,H) << endl;
  
  return 0;
}
