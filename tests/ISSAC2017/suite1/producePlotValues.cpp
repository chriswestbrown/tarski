/*******
 * Input is a sequence of numbers representing the time to
 * complete a task.  Negative numbers indicate a task that
 * could not be completed.
 * Output is the points defining a plot of number of instances
 * completed over time, where instances are assumed to be
 * solved in order of difficulty.
 *******/
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

int main(int argc, char** argv)
{  
  int logBase = 0;

  if (argc > 1 && argv[1] == string("-l2"))
    logBase = 2;

  istream& in = cin;
  vector<double> V;
  int count;
  double next;
  while(in >> next)
  {
    ++count;
    if (next >= 0)
      V.push_back(next);
  }

  sort(V.begin(),V.end());

  int n = 0;
  double t = 0.0;
  for(int i = 0; i < V.size(); ++i)
  {
    t += V[i];
    n++;
    double tout = t;
    if (logBase > 0)
    { 
      tout = t > 1 ? log2(t) : 0;
      
    }
    cout << tout << ' ' << n << endl;
  }
  
  
  return 0;
}
