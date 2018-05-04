#ifndef _OC_BUILDER_
#define _OC_BUILDER_

#include <algorithm>
#include <string>
#include <iostream>
#include <cstdio>
#include <sstream>
#include <vector>
#include <map>
#include <cctype>
#include "../GC_System/GC_System.h"
#include "../algparse/algparse.h"
#include "../formula/formmanip.h"
#include "../formula/formula.h"
#include "../formula/normalize.h"
#include "../formula/linearSubs.h"
#include "../search/qfr.h"

#include "cellbound.h"
#include "varorder.h"
#include "memopolymanager.h"
#include "opencell.h"

namespace tarski {

class BuilderObj;  typedef GC_Hand<BuilderObj> BuilderRef;

class BuilderObj :  public GC_Obj
{
private:
  std::set<IntPolyRef,PolyManager::PolyLT> pfset;
  std::vector< queue<IntPolyRef> > Unprocessed; 
  std::vector< queue<IntPolyRef> > Processed;
  OpenCellRef D;

  bool errorStateFlag;  // True if the builder has encountered a projection factor
                        // that vanishes at point alpha.
  IntPolyRef errorPoly; // If errorStateFlag is true, this is std::set to the projection
                        // factor that was encountered that vanished at alpha.
  void setError(IntPolyRef p) { errorPoly = p; errorStateFlag = true; }

public:
  BuilderObj(OpenCellRef _D) : Unprocessed(_D->dimension()+1), 
    Processed(_D->dimension()+1), errorStateFlag(false) { D = _D; }
  
  
  std::set<IntPolyRef,PolyManager::PolyLT>::iterator pfbegin() { return pfset.begin(); }
  std::set<IntPolyRef,PolyManager::PolyLT>::iterator pfend() { return pfset.end(); }

  bool inPFSet(IntPolyRef p) { return pfset.find(p) != pfset.end(); }
  void swapPFSet(std::set<IntPolyRef,PolyManager::PolyLT> &newpfset) { swap(pfset,newpfset); }
  
  /*** Non-Recursive Contruction ***/
  bool oneStep(int n);  // return true if 'done', i.e. Unprocessed and Processed queues are all empty
  bool add(FactRef f);
  static BuilderRef buildOne(IntPolyRef p, VarOrderRef X, Word PT, int dim = -1);

  static void mergeBintoA(BuilderRef A, BuilderRef B, int dim); // dim <= dimension B, dimension A
  // NOTE: this function requires that the sample point of A is inside cell B

  void close() { int n = D->dimension(); while(!inErrorState() && !oneStep(n)); }

  /*** Recursive Construction ***/
  static BuilderRef buildOneRec(IntPolyRef p, VarOrderRef X, Word PT, int dim = -1);
  bool addRec(FactRef f, int k = -1);  

  /*** split ***/
  // cell Ap \subseteq A, dim is the dimension of A & Ap, k is the level to split at
  // std::vector V is empty initially.
  // result. V[0] is null or the "left" cell in the split
  // result. V[1] is null or the "right" cell in the split
  static void splitAtLevelK(BuilderRef A, BuilderRef Ap, int dim, int k, std::vector<BuilderRef> &V);

  // make an open-NuCAD in which polynomial p is sign-invariant
  static void mkNuCAD1(VarOrderRef X, IntPolyRef p, int dim, Word alpha, std::vector<BuilderRef> &V, std::vector<std::string> &L); 



  OpenCellRef getCell() { return D; }
  bool inErrorState() { return errorStateFlag; }
  IntPolyRef getErrorPoly() { return errorPoly; }
  std::string projFacSummary();
  std::string projFacInfo();
  std::vector< std::set<IntPolyRef,PolyManager::PolyLT> >  projFacByLevel();
  PolyManager* getPolyManager() { return getCell()->getPolyManager(); }
};
}//end namespace tarski
#endif
