#include <iostream>
#include <iomanip>
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

ostream& form(double x, ostream& out)
{
  out << setw(5);
  out.setf(ios::fixed);
  out.setf(ios::showpoint);
  out.precision(1);
  return out << x;
}

int main()
{
  Analyzer A; // percentage of NuCAD construction time needed for projection
  Analyzer B; // percentage of original NuCAD cell-size in projected NuCAD
  int npolys, nvars, r, tfull, tproj, ncfull, ncproj;
  string formName;

  while( cin >> npolys >> nvars >> r >> formName
	 >> tfull >> tproj >> ncfull >> ncproj)
  {
    double p = 100.0*double(max(.1,1.0*tproj))/tfull;
    A.add(p);
    B.add(100.0*double(ncproj)/ncfull);
  }
  form(A.min(),cout); cout << ' '; form(A.ave(),cout); cout << ' '; form(A.max(),cout);
  cout << "        ";
  form(B.min(),cout); cout << ' '; form(B.ave(),cout); cout << ' '; form(B.max(),cout);
  cout << endl;
  return 0;
}
