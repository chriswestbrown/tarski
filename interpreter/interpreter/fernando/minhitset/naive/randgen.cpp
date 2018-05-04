#include <iostream>
#include <cstdlib>
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 4)
  {
    cout << "usage: randgen <n> <k> <w>" << endl;
    cout << endl;
    cout << "Generates a random hitting set problem with <n>" << endl
	 << "elements in the base set, <k> subsets S_0,...,s_k, and" << endl
	 << "each S_i consisting of <w> elements." << endl;
    return 0;
  }
  
  srand(time(0));
  int n = atoi(argv[1]),
    k = atoi(argv[2]),
    w = atoi(argv[3]);

  cout << n << ' ' << k << endl;

  int *A = new int[w];
  
  for(int i = 0; i < k; i++)
  {
    for(int j = 0; j < w; j++)
    {
      bool x = true;
      while(x)
      {
	A[j] = 1 + (rand() % n);
	x = false;
	for(int m = 0; m < j; m++)
	  x = (x || (A[m] == A[j]));
      }
      cout << A[j] << ' ';
    }
    cout << 0 << endl;
  }

  
  
  
}
