#include <iostream>
#include <sstream>

#include "sacMod.h"
#include "poly.h"
#include "polymanager.h"
#include "factor.h"
#include "tracker.h"

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
namespace tarski { Tracker compTracker; }

int mainDUMMY(int argc, char **argv, void* topOfTheStack)
{
  int ac; char **av;
  SacModInit(argc,argv,ac,av,"MapleSac","Maple","/home/wcbrown/maple17/bin",topOfTheStack);

  PolyManager M;

  VarSet S = M.addVar("x");
  S = S + M.addVar("y");
  S = S + M.addVar("z");
  S = S + M.addVar("w");
  S = S + M.addVar("u");
  S = S + M.addVar("v");
  
  IntPolyRef a = new IntPolyObj;
  a->read(M,S);
  a->write(M); cout << endl;
  
  VarKeyedMap<VarSet> m;
  m[M.getVar("x")] = M.getVar("x");
  m[M.getVar("y")] = M.getVar("u");
  m[M.getVar("z")] = M.getVar("w");

  IntPolyRef b = a->renameVars(m);
  b->write(M); cout << endl;

  FactRef f = new FactObj(M);
  f->addMultiple(a,1);
  f->write(); cout << endl;  
  
  SacModEnd();
  return 0;
}

int main(int argc, char **argv)
{
  int dummy;
  mainDUMMY(argc,argv,&dummy);
}
