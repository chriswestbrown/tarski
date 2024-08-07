#include "testsamplepointmanager.h"

#include "term.h"
#include "property.h"
#include "checks.h"

using namespace tarski;


RealAlgNumRef TestSamplePointManagerObj::getCoordinate(SamplePointId s, int level)
{
  std::vector<RealAlgNumRef>& coords = store[s];
  return coords.size() < level ? integerToRealAlgNum(0) : coords[level-1];
}

TestSamplePointManagerObj::TestSamplePointManagerObj(VarOrderRef varOrder) : SamplePointManagerObj(varOrder)
{
  std::vector<RealAlgNumRef> *p = new std::vector<RealAlgNumRef>();
  store.push_back(*p);
  sectionValues.push_back(std::map<SectionRef,RealAlgNumRef>());
  rootsMap.push_back(std::map<IntPolyRef,std::vector<RealAlgNumRef> >());
  delete p;
}
  
  // register's point (s_1,...,s_{k},P_1,...,P_n)
SamplePointId TestSamplePointManagerObj::addRationalOver(SamplePointId s, GCWord P, int k)
{
  // Deal properly with recreating zero
  bool allZeros = true;
  for(Word L = P; L != NIL; L = RED(L))
    allZeros = (allZeros && (FIRST(L) == 0));
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
    rootsMap.push_back(std::map<IntPolyRef,std::vector<RealAlgNumRef> >());
  }
  return i;
}


SamplePointId TestSamplePointManagerObj::addRealOver(SamplePointId s, RealAlgNumRef P, int k)
{
  // Deal properly with recreating zero
  bool isZero =  (P->compareToLBRN(0) == 0);
  if (s == 0 && isZero || k == 0 && isZero) return 0;
  
  std::vector<RealAlgNumRef> newP;
  std::vector<RealAlgNumRef>& oldP = store[s];
  int c = 0;
  while(c < k && s != 0)
    newP.push_back(oldP[c++]);
  while (c++ < k)
    newP.push_back(integerToRealAlgNum(0));
  newP.push_back(P);
  
  // Check if already in store ... if not, add to store NOTE: we know newP is not zero
  int i;
  for(i = 1; i < store.size() && ! testIdentical(i,newP); i++);
  if (i == store.size())
  {
    store.push_back(newP);
    sectionValues.push_back(std::map<SectionRef,RealAlgNumRef>());
    rootsMap.push_back(std::map<IntPolyRef,std::vector<RealAlgNumRef> >());
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
  {
    // throw TarskiException("TestSamplePointManager: Section value over alpha not found!");
    IntPolyRef p = S1->getPoly();
    std::vector<RealAlgNumRef> V = fetchRootsOverAlpha(alpha,p);
    int k = S1->getIndex();
    if (k > V.size()) throw TarskiException("Section S1 doesn't exist in fetchSectionValueOverAlpha");
    registerSectionValue(alpha,S1,V[k-1]);
    return V[k-1];
  }
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
    Word x = i < P.size() ? P[i]->tryToGetExactRationalValue() : 0;
    if (x == NIL)
      throw TarskiException("SamplePointManager not ready to eval poly at irrational coordinate.");
    tmpP = COMP(x,tmpP);
  }
  // std::cerr << "p = "; p->write(*getPolyManager()); std::cerr << std::endl;
  // OWRITE(tmpP);
  // std::cerr << std::endl;
  return X->partialEval(p,tmpP,k);
}


int TestSamplePointManagerObj::polynomialSignAt(SamplePointId s, IntPolyRef p)
{
  int t;
  auto itr = signForCanonicalIntPoly.find(p.vpval());
  if (itr == signForCanonicalIntPoly.end())
  {
    VarOrderRef X = getVarOrder();
    t = signForCanonicalIntPoly[p.vpval()] = partialEval(s,p,X->level(p))->signIfConstant();    
  }
  else
    t = itr->second;
  return t;
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

std::vector<RealAlgNumRef>& TestSamplePointManagerObj::fetchRootsOverAlpha(SamplePointId s, IntPolyRef p)
{
  auto itr = rootsMap[s].find(p);
  if (itr == rootsMap[s].end())
  {
    VarOrderRef X = getVarOrder();
    int k = X->level(p);
    IntPolyRef A = partialEval(s,p, k - 1);
    auto V = RealRootIsolateRobust(A);
    std::vector<RealAlgNumRef> W;
    for(auto itr = V.begin(); itr != V.end(); ++itr)
      W.push_back(*itr);    
    auto res =
      rootsMap[s].insert(std::pair<IntPolyRef,std::vector<RealAlgNumRef> >(p,W));
    itr = res.first;
  }
  return itr->second;
}

std::pair<SectionRef,SectionRef> TestSamplePointManagerObj::boundingSections(SamplePointId s, IntPolyRef p)
{
  VarOrderRef X = getVarOrder();
  int k = X->level(p);
  std::vector<RealAlgNumRef>& V = fetchRootsOverAlpha(s,p);

  if (V.size() == 0) return std::pair<SectionRef,SectionRef>(Section::mkNegInfty(k),Section::mkPosInfty(k));
  
  // std::cout << "roots ... ";
  // for(int i = 0; i < V.size(); i++)
  //   std::cout << V[i]->toStr() << " ";
  // std::cout << std::endl;

  RealAlgNumRef alpha_k = getCoordinate(s,k); //store[s].at(k-1);
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
    //    std::cout << "p(alpha) = 0" << std::endl;
    sz = Section::mkIndexedRoot(p,il + 1);
    registerSectionValue(s,sz,V[il]);
    //    std::cout << "comparing to alpha ... " << compareToAlpha(s,sz) << std::endl;
  }
  else {
    if (il == 0 && cmpl > 0) {
      sl = Section::mkNegInfty(k);
      //      std::cout << "(s p v alpha) = -infty" << std::endl;
    }
    else {
      sl = Section::mkIndexedRoot(p,il);
      registerSectionValue(s,sl,V[il-1]);
      //      std::cout << "(s p v alpha) = (s p " << il << std::endl;
    }
    if (iu == V.size() - 1 && cmpl < 0) {
      su = Section::mkPosInfty(k);
      //      std::cout << "(s p ^ alpha) = +infty" << std::endl;
    }
    else {
      su = Section::mkIndexedRoot(p,iu + 2);
      registerSectionValue(s,su,V[iu + 1]);
      //      std::cout << "(s p ^ alpha) = (s p " << iu + 2 << std::endl;
    }
  }
  return isSector ? std::pair<SectionRef,SectionRef>(sl,su) : std::pair<SectionRef,SectionRef>(sz,NULL);
}

SamplePointId TestSamplePointManagerObj::splitBelowSample(SamplePointId id_alpha, SectionRef s1,
							  WithBoundsInterface* bip)
{
  VarOrderRef X = getVarOrder();
  int k = s1->level(X);
  RealAlgNumRef z2 = getCoordinate(id_alpha,k); //store[id_alpha][k-1]; // This is alpha_k
  RealAlgNumRef z1;
  if (s1->getKind() == Section::negInfty) { z1 = new NegInftyObj(); }
  if (s1->getKind() == Section::posInfty) { z1 = new PosInftyObj(); }
  if (s1->getKind() == Section::indexedRoot) { z1 = fetchSectionValueOverAlpha(id_alpha,s1); }
  Word x = rationalPointInInterval(z1,z2);
  SamplePointId nsid = addRationalOver(id_alpha,LIST1(x),k-1);
  nsid = completeSample(nsid,k,bip);
  return nsid;
}

SamplePointId TestSamplePointManagerObj::splitAboveSample(SamplePointId id_alpha, SectionRef s1,
							  WithBoundsInterface* bip)
{
  VarOrderRef X = getVarOrder();
  int k = s1->level(X);
  RealAlgNumRef z2 = getCoordinate(id_alpha,k); //store[id_alpha][k-1]; // This is alpha_k
  RealAlgNumRef z1;
  if (s1->getKind() == Section::negInfty) { z1 = new NegInftyObj(); }
  if (s1->getKind() == Section::posInfty) { z1 = new PosInftyObj(); }
  if (s1->getKind() == Section::indexedRoot) { z1 = fetchSectionValueOverAlpha(id_alpha,s1); }
  Word x = rationalPointInInterval(z2,z1);
  SamplePointId nsid =  addRationalOver(id_alpha,LIST1(x),k-1);
  nsid = completeSample(nsid,k,bip);
  return nsid;
}


SamplePointId TestSamplePointManagerObj::samplePointOnSection(SamplePointId id_alpha, SectionRef S,
							      WithBoundsInterface* bip)
{
  VarOrderRef X = getVarOrder();
  int k = S->level(X);

  RealAlgNumRef a1 = fetchSectionValueOverAlpha(id_alpha,S);
  SamplePointId nsid =  addRealOver(id_alpha,a1,k-1);
  nsid = completeSample(nsid,k,bip);
  return nsid;  
}

SamplePointId TestSamplePointManagerObj::completeSample(SamplePointId sid, int k, WithBoundsInterface* bip)
{
  if (bip == NULL) return sid;
  SamplePointId nsid = sid;
  int N = bip->getMaxLevel();
  for(int i = k+1; i <= N; ++i)
  {
    SectionRef sl = bip->getLowerBoundAtLevel(i);
    SectionRef su = bip->getUpperBoundAtLevel(i);
    if (sl->compare(su) == 0) // section
    {
      RealAlgNumRef a1 = fetchSectionValueOverAlpha(nsid,sl);
      nsid = addRealOver(nsid,a1,i-1);
    }
    else // sector
    {
      RealAlgNumRef a1 = fetchSectionValueOverAlpha(nsid,sl);
      RealAlgNumRef a2 = fetchSectionValueOverAlpha(nsid,su);
      Word beta = rationalPointInInterval(a1,a2);
      nsid = addRationalOver(nsid,LIST1(beta),i-1);
    }
  }
  return nsid;
}


std::string TestSamplePointManagerObj::toStr(SamplePointId sid)
{
  std::ostringstream sout;
  auto &V = store[sid];
  for(auto itr = V.begin(); itr != V.end(); ++itr)
  {
    sout << (*itr)->toStr() << endl;
  }
  return sout.str();
}

