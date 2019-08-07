#ifndef _CWBFACTOR_
#define _CWBFACTOR_

#include "polymanager.h"
#include <vector>

namespace tarski {
/********************************************************************************
 * A FactObj represents a integer polynomial in factored form.  Each factor will
 * be a reference to a PolyManager's cannonical copy.
 ********************************************************************************/
class FactObj;
typedef GC_Hand<FactObj> FactRef;

class FactObj : public GC_Obj
{
public:
  PolyManager *M;
  std::map<IntPolyRef,int> MultiplicityMap;
  GCWord content; // this is a SACLIB integer

  FactObj(PolyManager &_M) { M = &_M; content = 1; }
  FactObj(PolyManager &_M, Word C) { M = &_M; content = C; }
  FactObj(PolyManager * M) {this->M = M; content = 1; }
  FactObj(const FactObj &f) : M(f.M), MultiplicityMap(f.MultiplicityMap), content(f.content) { }
  int numFactors() const { return MultiplicityMap.size(); }
  int isZero() const { return content == 0; }
  int isConstant() const { return numFactors() == 0; }
  void addFactor(IntPolyRef p, int mult); // p is assumed to be positive, primitive, irreducible
  void addMultiple(IntPolyRef p, int mult); // p is NOT assumed to be positive, primitive, irreducible
  void write() const; // Write to the current Saclib Output Context
  std::string toString() const;
  void writeMAPLE(VarContext &C, std::ostream& out); // Write in Maple syntax to stream out
  void writeMATHEMATICA(VarContext &C, std::ostream& out); // Write in Maple syntax to stream out
  VarSet getVars() const;
  void varStats(VarKeyedMap< std::vector<int> > &M); // M[x] = [maxdeg of x, max tot degree of term in x, num terms with x]
  void addFactors(FactRef f, int mult);
  int signOfContent() const { return ISIGNF(content); }
  Word getContent() const { return content; }
  void negateContent() { content = INEG(content); }
  PolyManager* getPolyManagerPtr() { return M; }

  // If itr is a factorIterator, itr->first is the IntPolyRef, and itr->second is the multiplicity
  typedef std::map<IntPolyRef,int>::iterator factorIterator;
  factorIterator factorBegin() { return MultiplicityMap.begin(); }
  factorIterator factorEnd()  { return MultiplicityMap.end(); }

  
  //returns an integer
  //only works because factrefs are guaranteed to be references to canonical copies
  //do not use this if you want the ordering to be meaningful
  //returns the difference between the pointers
  int canonCompare(const FactRef F) const {return(this - &(*F));}

  // returns -1,0,+1
  int cmp(const FactRef F) const;


  class lt {
  public:
    bool operator()(const tarski::FactRef &a, const tarski::FactRef &b) { return a->cmp(b) < 0; }
  };
};

 struct FactComp  {
   bool operator() (const FactRef lhs, const FactRef rhs) const {
     if (lhs->cmp(rhs) >= 0)          { return false;  }
     else                             { return true; }
   }
 };

inline FactRef makeFactor(PolyManager &M, short int x) { FactObj* f = new FactObj(M); f->content = x; return f; }
inline FactRef makeFactor(PolyManager &M, IntPolyRef p, int e = 1) { FactObj* f = new FactObj(M); f->addMultiple(p,e); return f;}
FactRef operator*(FactRef f, FactRef g);


}//end namespace tarski

#endif
