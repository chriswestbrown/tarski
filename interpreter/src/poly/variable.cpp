#include "variable.h"
#include "cstdlib"
#include "../tarski.h"
#include <sstream>

using namespace std;

namespace tarski {

int VarSet::positionInOrder(VarSet x) const
{ 
  if ((x & *this).none()) return 0;
  int i = 1; 
  for(iterator itr = begin(); (x & *itr).none(); ++itr) ++i; 
  return i;
}  

Variable VarContext::getVar(const string &name)
{
  map<string,Variable>::iterator itr = table.find(name);
  if (itr == table.end()) return VarSet(); else return itr->second; 
}

Variable VarContext::addVar(const string &name)
{
  map<string,Variable>::iterator itr = table.find(name);
  if (itr != table.end()) return itr->second; 
  Variable v = (VarSet(1) << nextIndex++);
  if (nextIndex > varNumLimit) 
  { 
    ostringstream sout; 
    sout << "The current variable context threshold of " << varNumLimit << " variables exceeded!";
    throw TarskiException(sout.str());
    //    cerr << "Currently we only allow " << varNumLimit << " variables. Sorry." << endl; exit(1); 
  }
  names[ v.to_ulong() % (unsigned long)specialMod ] = name;
  return table[name] = v;
}

Variable VarContext::newVar(VarSet V)
{
  Variable x = VarSet(1);
  int i = 0;
  while(i < nextIndex && V.test(i)) { ++i; x <<= 1; }
  if (i < nextIndex) return x;
  if (nextIndex < varNumLimit)
  {
    return addVar("DUMMY");
  }
  return VarSet();
}

}//end namespace tarski
