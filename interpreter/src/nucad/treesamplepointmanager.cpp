#include "treesamplepointmanager.h"

#include "term.h"
#include "property.h"
#include "checks.h"

using namespace tarski;

//-- SPNode --------------------------------------------------------------//
SPNode::SPNode(int level, SPNode* parent, RealAlgNumRef x) : level(level), parent(parent), me(x)
{
  sid = -1;
}

std::string SPNode::toStr()
{
  return me->toStr();
}

int SPNode::coordKind(int level)
{
  int k = getLevel();
  if (level > k) return rational;
  if (level < k) return getParent()->coordKind(level);
  return myKind();
}

int SPNode::myKind()
{
  return me->tryToGetExactRationalValue() != NIL ? 0 : 1;
}


int SPNode::getLevel()
{
  return level;
}

SPNode* SPNode::getParent()
{
  return parent;
}

int SPNode::samplePointNumberOfExtensions(int level)
{
  int k = getLevel();
  return (myKind() != 0 ? 1 : 0) + (k > 0 ? getParent()->samplePointNumberOfExtensions(k-1) : 0);
}

SPNodeRef SPNode::addRealOver(RealAlgNumRef P, int k)
{
  if (k <= getLevel()) { throw TarskiException("Error in SPNode::addRealOver()!  k is too small!"); }
  std::pair<RealAlgNumRef,int> K = make_pair(P,k);
  auto itr = child.find(K);
  if (itr != child.end())
    return itr->second;
  SPNode* q = new SPNode(k,this,P);
  child[K] = q;
  return q;
}


Word SPNode::getRationalSampleUpTo(int k)
{
  Word res = NIL;
  SPNode* node = this;
  while(k > 0)
  {
    int kme = node->getLevel();
    if (k < kme) return node->getParent()->getRationalSampleUpTo(k);
    
    while(k > kme)
    {
      res = COMP(0,res);
      --k;
    }
    Word x = me->tryToGetExactRationalValue();
    if (x == NIL) throw TarskiException("Coordinate is not rational!");
    res = COMP(x,res);
  }
  return res;  
}



//------------------------------------------------------------------------//

TreeSamplePointManagerObj::TreeSamplePointManagerObj(VarOrderRef varOrder) : SamplePointManagerObj(varOrder)
{
  SPNode* p = new SPNode(0,NULL,NULL);
  p->setSamplePointId(0);
  idToSamplePoint.push_back(p);
}
  
SamplePointId TreeSamplePointManagerObj::addRationalOver(SamplePointId s, GCWord P, int k)
{
  // Set nsid to the id of the sample point weare actually adding onto
  int curr = k;
  SamplePointId nsid = s;
  while(idToSamplePoint[nsid]->getLevel() > curr)
    nsid = idToSamplePoint[nsid]->getParent()->getSamplePointId();

  // Add elements of P as coordinates of level k+1,k+2,...,
  for(Word Pp = P; Pp != NIL; Pp = RED(Pp))
  {
    if (FIRST(Pp) == 0) { ++curr; continue; }
    SPNodeRef R = idToSamplePoint[nsid]->addRealOver(rationalToRealAlgNum(FIRST(Pp)),++curr);
    if (R->getSamplePointId() < 0)
    {
      nsid = idToSamplePoint.size();
      R->setSamplePointId(nsid);
      idToSamplePoint.push_back(R);
    }
  }
  return nsid;
}

SamplePointId TreeSamplePointManagerObj::addRealOver(SamplePointId s, RealAlgNumRef P, int k)
{
  // Set nsid to the id of the sample point weare actually adding onto
  int curr = k;
  SamplePointId nsid = s;
  while(idToSamplePoint[nsid]->getLevel() > curr)
    nsid = idToSamplePoint[nsid]->getParent()->getSamplePointId();

  
  SPNodeRef R = idToSamplePoint[nsid]->addRealOver(P,++curr);
  if (R->getSamplePointId() < 0)
  {
    nsid = idToSamplePoint.size();
    R->setSamplePointId(nsid);
    idToSamplePoint.push_back(R);
  }
  return nsid;
}
  
int TreeSamplePointManagerObj::polynomialSignAt(SamplePointId s, IntPolyRef p)
{
  int k = getVarOrder()->level(p);
  SPNodeRef N = getNode(s);
  
  
  throw TarskiException("Not yet implemented!");
}

bool TreeSamplePointManagerObj::isNullifiedAt(SamplePointId s, IntPolyRef p)
{
  int k = getVarOrder()->level(p);
  SPNodeRef N = getNode(s);
  //  while(N->getLevel() > 
  throw TarskiException("Not yet implemented!");
}

std::pair<SectionRef,SectionRef> TreeSamplePointManagerObj::boundingSections(SamplePointId s, IntPolyRef p)
{ throw TarskiException("Not yet implemented!"); }

int TreeSamplePointManagerObj::compareAtAlpha(SamplePointId id_alpha, SectionRef s1, SectionRef s2)
{ throw TarskiException("Not yet implemented!"); }

int TreeSamplePointManagerObj::compareToAlpha(SamplePointId id_alpha, SectionRef s1)
{ throw TarskiException("Not yet implemented!"); }

SamplePointId TreeSamplePointManagerObj::splitBelowSample(SamplePointId id_alpha, SectionRef s1, WithBoundsInterface* bip)
{ throw TarskiException("Not yet implemented!"); }

SamplePointId TreeSamplePointManagerObj::splitAboveSample(SamplePointId id_alpha, SectionRef s1, WithBoundsInterface* bip)
{ throw TarskiException("Not yet implemented!"); }

SamplePointId TreeSamplePointManagerObj::samplePointOnSection(SamplePointId id_alpha, SectionRef S, WithBoundsInterface* bip)
{ throw TarskiException("Not yet implemented!"); }

SamplePointId TreeSamplePointManagerObj::completeSample(SamplePointId sid, int k, WithBoundsInterface* bip)
{ throw TarskiException("Not yet implemented!"); }
  
void TreeSamplePointManagerObj::debugDump()
{ throw TarskiException("debugDump() - not yet implemented"); }

std::string TreeSamplePointManagerObj::toStr(SamplePointId sid) {
  std::ostringstream sout;
  SPNodeRef p = getNode(sid);
  if (p.is_null()) { throw TarskiException("Invalid sample point id"); }  
  int k = p->getLevel();
  vector<std::string> S;
  while(k > 0)
  {
    SPNodeRef pp = p->getParent();
    int kp = pp.is_null() ? 0 : pp->getLevel();
    S.push_back(p->toStr());
    for(--k; k > kp; --k)
      S.push_back("0");
    p = pp;
  }
  for(int i = S.size() - 1; i >= 0; --i)
    sout << S[i] << '\n';
  return sout.str();
}
