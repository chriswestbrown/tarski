#include "factor.h"
#include "sacMod.h"
#include "tracker.h"

using namespace std;

namespace tarski {
void FactObj::addFactor(IntPolyRef p, int mult) // p is assumed to be positive, primitive, irreducible
{
  if (mult == 0) return;
  if (p->isConstant()) { content = IPROD(content,IEXP(p->sP,mult)); return; } 
  IntPolyRef pc = M->getCannonicalCopy(p);
  map<IntPolyRef,int>::iterator i = MultiplicityMap.find(pc);
  if (i != MultiplicityMap.end())
    (*i).second += mult;
  else
    MultiplicityMap.insert(pair<IntPolyRef,int>(pc,mult));
}

extern Tracker compTracker;

void FactObj::addMultiple(IntPolyRef p, int mult) // p is NOT assumed to be positive, primitive, irreducible
{
  if (mult == 0) return;
  if (p->isConstant()) { addFactor(p,mult); return; }
  Word s,c,L;
  M->factorHelper(p,s,c,L);
    //if (t1-t0 > 10) { cout << t1-t0 << " : "; p->write(*M); cout << endl; }
  content = IPROD(content,IEXP(IPROD(s,c),mult));
  for(Word Lp = L; Lp != NIL; Lp = RED(Lp))
    addFactor(new IntPolyObj(p->slevel,SECOND(FIRST(Lp)),p->svars), mult*FIRST(FIRST(Lp)));
}

FactRef operator*(FactRef f, FactRef g)
{
  FactRef r = new FactObj(*(f->M));
  r->content = IPROD(f->content,g->content);
  for(map<IntPolyRef,int>::iterator itr = f->MultiplicityMap.begin(); itr != f->MultiplicityMap.end(); ++itr)
    r->MultiplicityMap[itr->first] = itr->second;
  for(map<IntPolyRef,int>::iterator itr = g->MultiplicityMap.begin(); itr != g->MultiplicityMap.end(); ++itr)
  {
    map<IntPolyRef,int>::iterator ritr = r->MultiplicityMap.find(itr->first);
    if (ritr != r->MultiplicityMap.end()) { ritr->second += itr->second; }
    else { r->MultiplicityMap[itr->first] = itr->second; }
  }
  return r;
} 


void FactObj::write() const // Write to the current Saclib Output Context 
{
  if (numFactors() == 0 || content != 1) IWRITE(content);
  map<IntPolyRef,int>::const_iterator i = MultiplicityMap.cbegin();
  if (content == 1 && numFactors() == 1 && i->second ==  1)
    i->first->write(*M);
  else
    while(i != MultiplicityMap.cend())
    {
      CWRITE('(');
      i->first->write(*M);
      CWRITE(')');
      if (i->second > 1) { CWRITE('^'); IWRITE(i->second); }
      ++i;
    }
}

void FactObj::writeMATHEMATICA(VarContext &C, ostream& out)
{ // This is easy because Mathematica's input syntax for polynomials matches Saclib's
  PushOutputContext(out);
  write();
  PopOutputContext();
}

void FactObj::writeMAPLE(VarContext &C, ostream& out)
{
  PushOutputContext(out);
  bool contentPrinted = false;
  if (numFactors() == 0 || content != 1) { IWRITE(content); contentPrinted = true; }
  map<IntPolyRef,int>::iterator i = MultiplicityMap.begin();
  if (content == 1 && numFactors() == 1 && i->second ==  1)
    i->first->writeMAPLE(C,out);
  else
  {
    bool firstFactor = true;
    while(i != MultiplicityMap.end())
    {
      if (!firstFactor || contentPrinted) { CWRITE('*'); }
      CWRITE('(');
      i->first->writeMAPLE(C,out);
      CWRITE(')');
      if (i->second > 1) { CWRITE('^'); IWRITE(i->second); }
      ++i;
      firstFactor = false;
    }
  }
  PopOutputContext();
}

VarSet FactObj::getVars() const
{
  VarSet res;
  for(map<IntPolyRef,int>::const_iterator i = MultiplicityMap.begin(); i != MultiplicityMap.end(); ++i)
    res |= (*i).first->getVars();
  return res;
}


// M's nullvalue must be set to a vector of 3, maxdeg, maxtdeg,nterms
void FactObj::varStats(VarKeyedMap< vector<int> > &M)
{
  for(map<IntPolyRef,int>::iterator i = MultiplicityMap.begin(); i != MultiplicityMap.end(); ++i)
  {
    (*i).first->varStats(M);
    // Word L = IPVARSTATS((*i).first->slevel,(*i).first->sP);
    // VarSet V = (*i).first->getVars();
    // for(VarSet::iterator vitr = V.begin(); vitr != V.end(); ++vitr)
    // {
    //   Word w = FIRST(L);
    //   L = RED(L);
    //   vector<int> res = M[*vitr];
    //   res[0] = max(res[0],FIRST(w));
    //   res[1] = max(res[1],SECOND(w));
    //   res[2] = res[2] + THIRD(w);
    //   M[*vitr] = res;
    // }
  }
}

void FactObj::addFactors(FactRef f, int mult)
{
  for(map<IntPolyRef,int>::iterator i = f->MultiplicityMap.begin(); i != f->MultiplicityMap.end(); ++i)
    addFactor(i->first,i->second * mult);
}

int FactObj::cmp(FactRef F) const
{
  if (this == &(*F)) { return 0; }
  int t1 = ICOMP(getContent(),F->getContent());
  if (t1 != 0) { return t1; }
  int t2 = ISIGNF(numFactors() - F->numFactors());
  if (t2 != 0) { return t2; }
  map<IntPolyRef,int>::const_iterator i1 = MultiplicityMap.cbegin(),
    i2 = F->MultiplicityMap.cbegin();
  while(i1 != MultiplicityMap.cend())
  {
    if (i1->second != i2->second) { return SIGN(i1->second - i2->second); }
    int t3 = IntPolyObj::ipcompare(i1->first,i2->first);
    if (t3 != 0) { return t3; }
    ++i1; ++i2;
  }
  return 0;
}

string FactObj::toString() const
{
  ostringstream sout; PushOutputContext(sout); 
  write();
  PopOutputContext(); return sout.str();
}
}//end namespace tarski
