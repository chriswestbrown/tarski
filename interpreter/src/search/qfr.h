#ifndef _QFR_H_
#define _QFR_H_
#include <algorithm>
#include <sys/types.h>
#include <time.h>
#include <iostream>
#include <stack>
#include "../algparse/algparse.h"
#include "search.h"
#include "grade.h"
#include "rewrite.h"
#include "../formula/formmanip.h"
#include "../tarski.h"

namespace tarski {
/************************************************************
 * GreedyQueueManager:  This is a dirt simple QM for doing a
 * greedy search rather than the exhaustive search I've been
 * using.  
 ************************************************************/
class GreedyQueueManager : public QueueManagerAndChooser
{
public:
  TFQueueRef _root;
  FormulaGrader *_FG; 

  class Cmp
  {
    MinFormFinder *_FG; 
  public:
    Cmp(MinFormFinder *FG) { _FG = FG; }
    bool operator()(QAndRef a, QAndRef b) const { return _FG->getMinGrade(a) < _FG->getMinGrade(b); }
  };

public:
  void setData(TFQueueRef root, FormulaGrader &FG) { _root = root; _FG = &FG; }
  void recordAndEnqueued(QAndRef A) { QueueManager::recordAndEnqueued(A);  }
  QAndRef next() 
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
    if (Parts.size() == 0) return 0;
    sort(Parts.begin(),Parts.end(),Cmp(&MF));
    return Parts[Parts.size() - 1];
  }
};

  
class GreedyGuidedQueueManager : public GreedyQueueManager
{
  class negcomp { public: FewestQuantifiedVariablesFirst F; bool operator()(QAndRef a, QAndRef b) const { return !F(a,b); } };
  priority_queue<QAndRef,std::vector<QAndRef>, negcomp> andsToExpand;

public:
  void recordAndEnqueued(QAndRef A);
  QAndRef next();  
};
  
/************************************************************
 * BEGIN GLOBAL VARIABLES
 ************************************************************/
extern int timing;
extern int verbose;
extern int developer;
extern bool finalcleanup;
extern bool fullyGeneric;
/************************************************************
 * END GLOBAL VARIABLES
 ************************************************************/

class QFR
{
public:
  QFR();
  int init(int QMTf, TFormRef _T, PolyManager *_pM); // Returns 0 if successful
  void rewrite();
  TFormRef getBest();
  VarSet getQuantifiedVariables();
  void printDerivation();
  ~QFR();
private:
  PolyManager *pM;
  TFormRef F, Forig;
  VarSet QVars;
  QueueManagerAndChooser *QMaC;
  QueueManager *globalQM;
  BasicRewrite* pReW;
  TFQueueRef Q;
  SimpleGradeForQEPCAD SG;
  MinFormFinder MF;
  pair<TFormRef,double> minp;
};
}//end namespace tarski
#endif
