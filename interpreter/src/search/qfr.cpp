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
  {
    F = defaultNormalizer->normalize(F);
  }
    
  // What to do if F normalizes to a constant?

  // Initialize QueueManager
  switch(QMTf)
  {
  case 1: QMaC = new BFQueueManager; break; 
  case 2: QMaC = new DFQueueManager; break; 
  case 3: QMaC = new PrioritySearchQueueManager<SmallestDimensionFirst>; break;
  case 4: QMaC = new PrioritySearchQueueManager<FewestQuantifiedVariablesFirst>; break;
  case 5: QMaC = new GreedyQueueManager; break;
  case 6: QMaC = new GreedyGuidedQueueManager; break;    
  } 
  globalQM = QMaC;
  pReW = new BasicRewrite(globalQM,0/*BasicRewrite::M_linearSpolys*/);
  Q = new TFQueueObj(globalQM);

  
  // finish intialization of QueueManager if needed
  SimpleGradeForQEPCAD tmpG;
  if (QMTf == 5 || QMTf == 6) { dynamic_cast<GreedyQueueManager*>(QMaC)->setData(Q,tmpG); }

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
  // This bit is all about the self-monitoring determination of when to stop/continue
  bool selfMonitorStop = true;
  int iterCount = 0, stepsSinceLastBest = 0, stepsToLastBest = 10, countOfLastBest = 0;
  double lastBestScore = -1.0;

  // The loop!
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

    // SelfMonitor - stop if there hasn't been an improvement in a sufficient # of steps
    if (selfMonitorStop) {
      ++iterCount;
      if (++stepsSinceLastBest > stepsToLastBest) { break; }
      getBest();
      double score = minp.second;
      if (lastBestScore < 0 || score < lastBestScore) {
	lastBestScore = score;
	stepsSinceLastBest = 0;	
	stepsToLastBest = max(iterCount - countOfLastBest,stepsToLastBest);
	countOfLastBest = iterCount;
      }
      if (false) {
	cout << "iterCount = " << iterCount << ", score = " << score << " "
	     <<  stepsSinceLastBest << " " << stepsToLastBest << endl << endl;
	if (dynamic_cast<GreedyQueueManager*>(globalQM)) {
	  //NOTE: only GreedyQueueManager's currently have a field for the root
	  globalQM->graphStats(dynamic_cast<GreedyQueueManager*>(globalQM)->_root);
	  cout << endl << endl;
	}
      }
    }
  }  
}

TFormRef QFR::getBest()
{
  // cout << "In getBest()" << endl;
  if (!Q->constValue.is_null()) { return Q->constValue; }
  MF.process(Q,SG); 
  minp.first = MF.getMinFormula(Q); minp.second = MF.getMinGrade(Q);

  // minp.first is QAndRef or QOrRef, I need to get a regular TFormRef
  // Out of it!  NOTE: I decided to let the caller worry about this!

  return minp.first;
}

VarSet QFR::getQuantifiedVariables()
{
  return QVars;
}

  void QFR::printDerivation() {
    //cout << "In printDerivation()!" << endl;
    TOrRef tor = new TOrObj();
    tor->OR(minp.first);
    for(set<TFormRef>::iterator itr = tor->disjuncts.begin(); itr != tor->disjuncts.end(); ++itr)
    {
      QNodeRef qnsa = MF.fromTtoQ[(*itr)->tag];
      QAndRef sa = asa<QAndObj>(qnsa);
      if (!sa.is_null())
      {
	cout << "Derivation is:" << endl;
	stack<QAndRef> S_p;
	while(!sa->PR->initial())
	{
	  S_p.push(sa);
	  sa = sa->PR->predecessor();
	}
	while(!S_p.empty())
	{
	  S_p.top()->PR->write();
	  cout << " -> ";
	  S_p.top()->write(true);
	  cout << endl;
	  S_p.pop();
	}
      }
    }
}

QFR::~QFR() { 
  finalcleanup = true;
  Q = 0;
  delete pReW; 
  delete globalQM;
}


  void GreedyGuidedQueueManager::recordAndEnqueued(QAndRef A)
  {
    QueueManager::recordAndEnqueued(A);
    andsToExpand.push(A);
  }


  QAndRef GreedyGuidedQueueManager::next() 
  {     
    //-- Find the "best" rewriting currently available
    SimpleGradeForQEPCAD SG;
    MinFormFinder MF; MF.process(_root,SG); 
    pair<TFormRef,double> minp; minp.first = MF.getMinFormula(_root); minp.second = MF.getMinGrade(_root);

    //-- Build a std::vector of all unexpanded disjuncts
    std::vector<QAndRef> Parts;
    TOrRef tor = new TOrObj();
    tor->OR(minp.first);
    for(std::set<TFormRef>::iterator itr = tor->disjuncts.begin(); itr != tor->disjuncts.end(); ++itr)
    {
      QNodeRef qnsa = MF.fromTtoQ[(*itr)->tag];
      QAndRef sa = asa<QAndObj>(qnsa);
      if (sa.is_null()) {
	std::cerr << "Error in GreedyQueueManager::next!!!" << std::endl; 
	(*itr)->write();
	std::cout << std::endl;
	if (asa<TAndObj>(*itr)) { std::cerr << "It's an AND!" << std::endl; } else { std::cerr << "It's not an AND!" << std::endl; }
	throw TarskiException("Error in GreedyQueueManager!");
      }
      if (!sa->expanded) Parts.push_back(sa);
    }

    //-- If Parts empty, then greedy search is done, else return "minimum" element of Parts
    if (Parts.size() == 0) {
      //cerr << "Parts is 0" << endl;
      QAndRef n;
      TFQueueRef np = NULL;
      while(np.is_null())
      {
	if (andsToExpand.empty()) return 0;
	n = andsToExpand.top(); 
	andsToExpand.pop();
	if (n->done()) { continue; }
	np = find(n);
      }
      return n; 
    }
    else {
      sort(Parts.begin(),Parts.end(),Cmp(&MF));
      for(int i = 0; i < Parts.size() - 1; i--) {
	andsToExpand.push(Parts[i]);
	cerr << "Pushing back!" << endl;
      }
      return Parts[Parts.size() - 1];
    }
  }


  
}//end namespace tarski
