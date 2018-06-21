#include "samplePointManager.h"

using namespace tarski;

TestSamplePointManagerObj::TestSamplePointManagerObj(VarOrderRef varOrder) : SamplePointManagerObj(varOrder)
{
  std::vector<RealAlgNumRef> *p = new std::vector<RealAlgNumRef>();
  store.push_back(*p);
  sectionValues.push_back(std::map<SectionRef,RealAlgNumRef>());
  rootsMap.push_back(std::map<IntPolyRef,std::vector<RealRootIUPRef> >());
  delete p;
}
  
  // register's point (s_1,...,s_{k},P_1,...,P_n)
SamplePointId TestSamplePointManagerObj::addRationalOver(SamplePointId s, GCWord P, int k)
{
  // Deal properly with recreating zero
  bool allZeros = true;
  for(Word L = P; L != NIL; L = RED(L))
    allZeros = (allZeros && (FIRST(P) == P));
  if (s == 0 && allZeros || k == 0 && allZeros) return 0;
  
  std::vector<RealAlgNumRef> newP;
  std::vector<RealAlgNumRef>& oldP = store[s];
  int c = 0;
  while(c < k && s != 0)
    newP.push_back(oldP[c++]);
  while (c++ < k)
    newP.push_back(integerToRealAlgNum(0));
  for(Word L = P; L != NIL; L = RED(L))
    newP.push_back(rationalToRealAlgNum(FIRST(L)));
  
  // Check if already in store ... if not, add to store NOTE: we know newP is not zero
  int i;
  for(i = 1; i < store.size() && ! testIdentical(i,newP); i++);
  if (i == store.size())
  {
    store.push_back(newP);
    sectionValues.push_back(std::map<SectionRef,RealAlgNumRef>());
    rootsMap.push_back(std::map<IntPolyRef,std::vector<RealRootIUPRef> >());
  }
  return i;
}

void TestSamplePointManagerObj::registerSectionValue(SamplePointId alpha, SectionRef S1, RealAlgNumRef beta)
{
  sectionValues[alpha][S1] = beta;
}

RealAlgNumRef TestSamplePointManagerObj::fetchSectionValueOverAlpha(SamplePointId alpha, SectionRef S1)
{
  if (S1->getKind() == Section::negInfty) { return new NegInftyObj(); }
  if (S1->getKind() == Section::posInfty) { return new PosInftyObj(); }
  auto itr = sectionValues[alpha].find(S1);
  if (itr == sectionValues[alpha].end())
    throw TarskiException("TestSamplePointManager: Section value over alpha not found!");
  return itr->second;
}

int TestSamplePointManagerObj::compareAtAlpha(SamplePointId id_alpha, SectionRef s1, SectionRef s2)
{
  RealAlgNumRef z1 = fetchSectionValueOverAlpha(id_alpha,s1);
  RealAlgNumRef z2 = fetchSectionValueOverAlpha(id_alpha,s2);
  return z1->compareTo(z2);
}

int TestSamplePointManagerObj::compareToAlpha(SamplePointId id_alpha, SectionRef s1)
{
  if (s1->getKind() == Section::negInfty) { return -1; }
  if (s1->getKind() == Section::posInfty) { return +1; }
  if (s1->getKind() == Section::indexedRoot)
  {  
    VarOrderRef X = getVarOrder();
    int k = X->level(s1->getPoly());
    RealAlgNumRef z1 = fetchSectionValueOverAlpha(id_alpha,s1);
    RealAlgNumRef z2 = store[id_alpha][k-1];
    return z1->compareTo(z2);
  }
  throw TarskiException("TestSamplePointManager::compareToAlpha: Unknown section kind");
}


IntPolyRef TestSamplePointManagerObj::partialEval(SamplePointId s, IntPolyRef p, int kp)
{
  // For the moment just throw an exception if the point isn't
  // rational
  VarOrderRef X = getVarOrder();
  int k = std::min(X->level(p),kp);
  GCWord tmpP = NIL;
  std::vector<RealAlgNumRef> &P = store[s];
  for(int i = k-1; i >= 0; i--)
  {
    Word x = P[i]->tryToGetExactRationalValue();
    if (x == NIL)
      throw TarskiException("SamplePointManager not ready to eval poly at irrational coordinate.");
    tmpP = COMP(x,tmpP);
  }
  std::cerr << "p = "; p->write(*getPolyManager()); std::cerr << std::endl;
  OWRITE(tmpP);
  std::cerr << std::endl;
  return X->partialEval(p,tmpP,k);
}


int TestSamplePointManagerObj::polynomialSignAt(SamplePointId s, IntPolyRef p)
{
  VarOrderRef X = getVarOrder();
  return partialEval(s,p,X->level(p))->signIfConstant();
}

bool TestSamplePointManagerObj::isNullifiedAt(SamplePointId s, IntPolyRef p)
{
  VarOrderRef X = getVarOrder();
  return partialEval(s,p,X->level(p)-1)->isZero();
}

bool TestSamplePointManagerObj::testIdentical(SamplePointId s, std::vector<RealAlgNumRef>& newP)
{
  std::vector<RealAlgNumRef>& oldP = store[s];
  int n_o = oldP.size(), n_n = newP.size();
  if (n_o != n_n) return false;
  for(int i = 0; i < n_o; i++)
    if (oldP[i]->compareTo(newP[i]) != 0)
      return false;
  return true;
}

void TestSamplePointManagerObj::debugDump()
{
  for(unsigned int i = 1; i < store.size(); i++)
  {
    std::cout << i << " : ";
    std::vector<RealAlgNumRef>& P = store[i];
    for(unsigned int j = 0; j < P.size(); j++)
      std::cout << P[j]->toStr() << " ";
    std::cout << std::endl;
  }
}

std::vector<RealRootIUPRef>& TestSamplePointManagerObj::fetchRootsOverAlpha(SamplePointId s, IntPolyRef p)
{
  auto itr = rootsMap[s].find(p);
  if (itr == rootsMap[s].end())
  {
    VarOrderRef X = getVarOrder();
    int k = X->level(p);
    IntPolyRef A = partialEval(s,p, k - 1);  
    auto res =
      rootsMap[s].insert(std::pair<IntPolyRef,std::vector<RealRootIUPRef> >(p,RealRootIsolateRobust(A)));
    itr = res.first;
  }
  return itr->second;
}

void TestSamplePointManagerObj::roots(SamplePointId s, IntPolyRef p)
{
  VarOrderRef X = getVarOrder();
  int k = X->level(p);
  std::vector<RealRootIUPRef>& V = fetchRootsOverAlpha(s,p);
  std::cout << "roots ... ";
  for(int i = 0; i < V.size(); i++)
    std::cout << V[i]->toStr() << " ";
  std::cout << std::endl;
  RealAlgNumRef alpha_k = store[s].at(k-1);
  int il = 0, cmpl = 1;
  while(il < V.size() && (cmpl = V[il]->compareTo(alpha_k)) < 0)
    ++il;
  int iu = V.size() - 1, cmpu = -1;
  while(iu>= 0 && (cmpu = V[iu]->compareTo(alpha_k)) > 0)
    --iu;
  bool isSector = true;
  SectionRef su, sl, sz;
  if (cmpl == 0) {
    isSector = false;
    std::cout << "p(alpha) = 0" << std::endl;
    sz = Section::mkIndexedRoot(p,il + 1);
    registerSectionValue(s,sz,V[il]);
    std::cout << "comparing to alpha ... " << compareToAlpha(s,sz) << std::endl;
  }
  else {
    if (il == 0 && cmpl > 0) {
      sl = Section::mkNegInfty();
      std::cout << "(s p v alpha) = -infty" << std::endl; }
    else {
      sl = Section::mkIndexedRoot(p,il);
      registerSectionValue(s,sl,V[il-1]);
      std::cout << "(s p v alpha) = (s p " << il << std::endl; }
    if (iu == V.size() - 1 && cmpl < 0) {
      su = Section::mkPosInfty();
      std::cout << "(s p ^ alpha) = +infty" << std::endl; }
    else {
      su = Section::mkIndexedRoot(p,iu + 2);
      registerSectionValue(s,su,V[iu + 1]);
      std::cout << "(s p ^ alpha) = (s p " << iu + 2 << std::endl; }
  }
  if (isSector) {
    std::cout << sl->toStr(getPolyManager()) << " < " << su->toStr(getPolyManager()) << std::endl;
    std::cout << "comparing sl to alpha ... " << compareToAlpha(s,sl) << std::endl;
    std::cout << "comparing su to alpha ... " << compareToAlpha(s,su) << std::endl;
    std::cout << "comparing su to sl ... " << compareAtAlpha(s,su,sl) << std::endl;
    
  }
  else { std::cout << sz->toStr(getPolyManager()) << std::endl; }
}

