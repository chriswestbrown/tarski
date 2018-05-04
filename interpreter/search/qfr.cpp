#include "qfr.h"

using namespace std;

namespace tarski {
  
/************************************************************
 * BEGIN GLOBAL VARIABLES
 ************************************************************/
int timing = 0;
int verbose = 0;
int developer = 0;
bool finalcleanup = false;
bool fullyGeneric = false;
/************************************************************
 * END GLOBAL VARIABLES
 ************************************************************/

QFR::QFR() { pM = NULL; QMaC = NULL; pReW = NULL; globalQM = NULL; }
int QFR::init(int QMTf, TFormRef _T, PolyManager *_pM)
{
  pM = _pM;
  Forig = _T;

  finalcleanup = false;

  // Process Forig, get F and QVars
  if (!isPrenex(Forig)) 
  { 
    throw TarskiException("QFR: Input formula must be prenex!"); 
  }
  
  TQBRef FQB = asa<TQBObj>(Forig);
  if (!FQB.is_null())
  {
    if (FQB->size() > 1 || FQB->outermostBlockType != EXIST) 
    { 
      throw TarskiException("Existentially quantified formulas only!");
    }
    F = FQB->formulaPart;
    QVars = FQB->blocks[0];
  }
  else
    F = Forig;
  F = getDNF(F);

  // Normalize
  if (asa<TOrObj>(F))
  { 
    TOrRef tor = asa<TOrObj>(F);
    TOrRef tmp = new TOrObj;
    for(set<TFormRef>::iterator itr = tor->disjuncts.begin(); itr != tor->disjuncts.end(); ++itr)
      tmp->OR(defaultNormalizer->normalize(*itr));
    if (tmp->size() == 0) F = new TConstObj(FALSE);
    if (tmp->size() == 1) F = *tmp->disjuncts.begin();
    else F = tmp;
  }
  else
    F = defaultNormalizer->normalize(F);

  // What to do if F normalizes to a constant?

  // Initialize QueueManager
  switch(QMTf)
  {
  case 1: QMaC = new BFQueueManager; break; 
  case 2: QMaC = new DFQueueManager; break; 
  case 3: QMaC = new PrioritySearchQueueManager<SmallestDimensionFirst>; break;
  case 4: QMaC = new PrioritySearchQueueManager<FewestQuantifiedVariablesFirst>; break;
  case 5: QMaC = new GreedyQueueManager; break;
  } 
  globalQM = QMaC;
  pReW = new BasicRewrite(globalQM);
  Q = new TFQueueObj(globalQM);

  
  // finish intialization of QueueManager if needed
  SimpleGradeForQEPCAD tmpG;
  if (QMTf == 5) { dynamic_cast<GreedyQueueManager*>(QMaC)->setData(Q,tmpG); }

  // Initialize queue Q
  if (asa<TAndObj>(F) || asa<TAtomObj>(F))
    Q->enqueue(new QAndObj(F,QVars,0,new PInitialObj()));
  else if (asa<TOrObj>(F))
  {
    TOrRef tor = asa<TOrObj>(F);
    vector<QAndRef> AV;
    for(set<TFormRef>::iterator itr = tor->disjuncts.begin(); itr != tor->disjuncts.end(); ++itr)
      AV.push_back(new QAndObj(*itr,QVars,0,new PInitialObj()));
    globalQM->enqueueOR(AV,Q);
  }
  else if (asa<TConstObj>(F))
  {
    Q->constValue = F;
  }
  else
  { 
    cerr << "QFR: Error! Unknown formula type to begin search!" << endl; 
    return 3; 
  }

  return 0;
}

void QFR::rewrite()
{
  QAndRef nextAnd;
  while(Q->constValue.is_null() && !(nextAnd = QMaC->next()).is_null())
  {    
    // Expand the node "nextAnd"
    TFormRef res = pReW->refine(nextAnd,QMaC->find(nextAnd));
    
    // React if refine returned nextAnd either identically TRUE or FALSE
    if (constValue(res) == 1) { Q->constValue = res; break; }
    if (constValue(res) == 0) { QMaC->notify(QInfo::equivFalse(nextAnd->parentQueue)); }
    
    // Reorganization of "the graph" might be necessary based on things found equiv or false
    QMaC->reorganize(Q);
  }  
}

TFormRef QFR::getBest()
{
  if (!Q->constValue.is_null()) { return Q->constValue; }
  SimpleGradeForQEPCAD SG;
  MinFormFinder MF; MF.process(Q,SG); 
  minp.first = MF.getMinFormula(Q); minp.second = MF.getMinGrade(Q);

  // minp.first is QAndRef or QOrRef, I need to get a regular TFormRef
  // Out of it!  NOTE: I decided to let the caller worry about this!

  return minp.first;
}

VarSet QFR::getQuantifiedVariables()
{
  return QVars;
}

void QFR::printDerivation() { }

QFR::~QFR() { 
  finalcleanup = true;
  Q = 0;
  delete pReW; 
  delete globalQM;
}

}//end namespace tarski
