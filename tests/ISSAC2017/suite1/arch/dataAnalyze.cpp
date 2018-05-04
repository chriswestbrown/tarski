#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class Analyzer
{
  vector<double> D;
public:
  void add(double x) { D.push_back(x); }
  double min()
  {
    double m = D[0]; for(int i = 1; i < D.size(); i++) m = ::min(m,D[i]);
    return m;
  }
  double max()
  {
    double m = D[0]; for(int i = 1; i < D.size(); i++) m = ::max(m,D[i]);
    return m;
  }
  double ave()
  {
    double m = D[0]; for(int i = 1; i < D.size(); i++) m += D[i];
    return m/D.size();
  }
};

int main()
{
  Analyzer A;
  int npolys, nvars, r, tfull, tproj, ncfull, ncproj;
  string formName;

  while( cin >> npolys >> nvars >> r >> formName
	 >> tfull >> tproj >> ncfull >> ncproj)
  {
    double p = 100.0*double(max(1,tproj))/tfull;
    A.add(p);
  }
  cout << A.min() << ' ' << A.ave() << ' ' << A.max() << endl;
  
  return 0;
}
