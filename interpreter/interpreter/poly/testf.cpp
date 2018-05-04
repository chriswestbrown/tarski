#include <iostream>
#include <sstream>

#include "sacMod.h"
#include "poly.h"
#include "polymanager.h"
#include "factor.h"
#include "../formrepconventions.h"

using namespace std;
using namespace tarski;

VarSet getVarSet(istream &in, VarContext &C)
{
  VarSet S;
  for(string name; cin >> name && name != ";"; S = S + C.addVar(name));
  return S;
}

uint64 GC_Obj::next_tag = 0;
uint64 GC_Obj::num_deleted = 0;


int stringToRelop(string s) { int i = 0; while(s != relopString(i)) ++i; return i; }

int main(int argc, char **argv)
{
  int ac; char **av;
  SacModInit(argc,argv,ac,av);

  PolyManager M;

  VarSet S;
  VarKeyedMap<int> VS(ALOP);
  cout << "Enter vars: e.g. 'x ?? 0 y > 0 ;' ";
  string s, rop, dz;
  while(cin >> s && s != ";" && cin >> rop >> dz)
  {
    Variable v  = M.addVar(s);
    S = S + v;
    VS[v] = stringToRelop(rop);
  }
  
  cout << "Enter poly P: ";
  IntPolyRef P = new IntPolyObj;
  P->read(M,S);

  CREAD();
  
  cout << "Enter poly Q: ";
  IntPolyRef Q = new IntPolyObj;
  Q->read(M,S);

  int sL, sR, c_strict, c_equal;
  int res = testf(P,Q,VS,GTOP,sL,sR,c_strict,c_equal,&M);
  
  if (res == 0) cout << "Output was false!" << endl; else cout << "Output was true!" << endl;

  SacModEnd();
  return 0;
}
