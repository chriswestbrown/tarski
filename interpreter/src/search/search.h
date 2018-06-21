#ifndef _SEARCH_SPACE_
#define _SEARCH_SPACE_
#include <queue>
#include <stack>
#include <set>
#include "../formula/normalize.h"

namespace tarski {
  
extern uint64 totalQNodes, numGenericSubs, numDegenerateSubs;
extern Normalizer* defaultNormalizer;
extern int verbose; // 0 : no extra info

/*
  System Notes:
  - QOr nodes should always contain two or more queues.  When they diminish
    to a single queue, their queue elements should be enqueued into the
    parent-queue of the QOr node.  If they diminish to no queues (of course)
    they ought to be replaced with a constant FALSE.
 */

class TFQueueObj;
typedef GC_Hand<TFQueueObj> TFQueueRef;
class QNodeObj;
typedef GC_Hand<QNodeObj> QNodeRef;
class QAndObj;
typedef GC_Hand<QAndObj> QAndRef;
class QOrObj;
typedef GC_Hand<QOrObj> QOrRef;

class QueueManager;

/************************************************************************
 * Track parentage of objects.
 ************************************************************************/
class ParentageObj;
typedef GC_Hand<ParentageObj> ParentageRef;

class ParentageObj : public GC_Obj
{
public:
  virtual void write() = 0;
  virtual bool initial() { return false; }
  virtual QAndRef predecessor() { return NULL; }
};

class PInitialObj : public ParentageObj
{
public:
  void write() { std::cout << "Initial"; }
  bool initial() { return true; }
  
};



/********************************************************************************
 * QNode
 * There are two types of QNodes: QAnd and QOr.
 * - QAnd is a conjunction of atoms.
 * - QOr is a disjunction of TFQueues, each of which is a std::set of equivalent
 *   formulas.  An actual disjunction can be realized by choosing an element from
 *   each TFQueue and ORing all of them together.
 * 
 * done() -  This returns true if the node has determined that it cannot generate
 *           any more new elements.
 * refine(Q) This function behaves differently for the two types:
 *        -  QAnd: try to perform some rewriting operator to generate children,
 *           enqueue them in Q ... and itself if it's not done.
 *        -  QOr : try to improve itself by improving one of its disjuncts,
 *           reenqueueing itself if it's not done.
 ********************************************************************************/
class QNodeObj : public TFormObj
{
private:
  std::string hash;
  int printedLength;
  void setHash();

public:
  TFQueueObj *parentQueue; // pointer to queue in which this QNode lives/lived
  uint64 parentQueueTag;
  int deadMark;

public:
  QNodeObj() : printedLength(-1), parentQueue(0), parentQueueTag(0), deadMark(0) { }
  VarSet QVars; // Set of quantified variables
  VarSet UVars; // Used variables - i.e. vars already substituted for.

  // Pure virtual functions
  virtual void write(bool withQuantifiers) { ; }
  virtual void write() { write(false); }

  // Common functionality
  std::string getHash() { if (printedLength == -1) setHash(); return hash; }
  int getPrintedLength() { if (printedLength == -1) setHash(); return printedLength; }
  std::string toString();
  void apply(TFPolyFun&) { }
};

template<class T>
T* asa(const QNodeRef& p) { return dynamic_cast<T*>(&(*p)); }

/********************************************************************************
 * TFQueue
 * A TFQueue represents a std::set of formulas that are equivalent to some "original"
 * formula.  It also has a mechanism for generating further elements to add to
 * itself.
 ********************************************************************************/
class TFQueueObj : public GC_Obj
{
  friend void subsume(TFQueueRef Q, TFQueueRef victim, std::set<QOrObj*> &OrsToDrop);

public:
  TFQueueObj(QueueManager *_globalQM);
  TFQueueObj(QAndRef N, QOrObj *parent);
  ~TFQueueObj();
  QAndRef getOriginal();
  void enqueue(QNodeRef t);

public:
  TConstRef constValue;
  std::set<QNodeRef> nodes;
  std::set<QOrObj*> parentOrs; // Pointers to the QOrObj's in which this queue lives
  int mark, inscc;
  QAndRef original;
  QueueManager *globalQM;
};

class QOrObj : public QNodeObj
{
public:
  friend class TFQueueObj;
  std::set<TFQueueRef> disjuncts; 
  int mark;
  QueueManager *globalQM;

  QOrObj(TFQueueObj *parent);
  QOrObj(QAndRef a, QAndRef b, TFQueueObj *parent);
  void add(TFQueueRef nQ);
  ~QOrObj();
  VarSet getVars() { return VarSet(); /* Should never get called! */}
};

class QAndObj : public QNodeObj
{
  friend class TFQueueObj;

public:
  QAndObj(TFormRef _F, VarSet _QVars, VarSet _UVars, ParentageRef _PR);
  bool done();
  void write(bool withQuantifiers);
  VarSet getVars() { return F->getVars(); }
  ParentageRef PR;
  TFormRef F;
  PolyManager* getPolyManagerPtr() { return F->getPolyManagerPtr(); }

  bool expanded;
  int children;

};



/************************************************************************
 * The algorithm keeps a queue of "facts", i.e. QInfo objects, that are
 * statements that two nodes are equivalent, or that a given node is 
 * equivalent to false.
 ************************************************************************/
class QInfo
{
public:
  uint64 q1, q2;

  // This is a hack.  It can be used to keep a queue alive ... at least
  // until the fact that it is false can be processed.  I need this for
  // the following situation: Q1 is proven false, and it goes away.
  // However, any queue Q2 in a child-OR of Q1's is implicitly proved
  // false by the falsity of Q1.  I need to record that fact.  But if Q2
  // has no parent but that OR living in Q1, it will be deleted by the
  // reference counter before I can record its falsity.  Lifeline fixes
  // this by providing an artificial reference.
  TFQueueRef lifeline; 

  static QInfo equivFalse(TFQueueRef ptr) 
  {
    QInfo I; I.q1 = ptr->tag; I.q2 = 0; 
    I.lifeline = ptr;
    return I; 
  }
  static QInfo equivFalse(uint64 p) { QInfo I; I.q1 = p; I.q2 = 0; return I; }
  static QInfo equiv(uint64 pL, uint64 pR) 
  { 
    QInfo I; 
    I.q1 = pL; 
    I.q2 = pR; 
    return I; 
  }
  bool typeFalse() { return q2 == 0; }
  bool typeEquiv() { return q2 != 0; }
};

/************************************************************************
 * This would better be called "GraphManager".  
 ************************************************************************/
class QueueManager
{
public:
  // When an QAndNode is enqueued, it needs to be recorded with the QueueManager
  virtual void recordAndEnqueued(QAndRef A);

  // Because of subsumption, pQ may refer to a Queue that no longer exists.  This returns the
  // current equivalent queue.  Returns NULL if the queue was proved false.
  TFQueueObj* currentQueue(uint64 pQ); 

  // Returns true if A doesn't live in any queue.  False otherwise.
  bool isNew(QAndRef A);

  // Returns pointer to the current Queue in which A lives (NULL if that queue was proved false).
  // Note: this is only valid if A *does* already live in a queue!
  TFQueueObj* find(QAndRef A);

  // Notify QueueManager of a piece of information
  void notify(QInfo I);

  // Tells QueueManager to reorganize based on notifications
  void reorganize(TFQueueRef root);

  // takes CA, a std::vector of conjuncts that I'd like to form the disjunction of, forms the
  // disjunction and enqueues it.  This takes care of simplifying away any conjuncts that
  // are either false (because they're a bit illformed) or are in a queue which was found
  // to be false
  void enqueueOR(std::vector<QAndRef> &CA,TFQueueRef Q);

  // This prints out basic stats concerning "the graph"
  void graphStats(TFQueueRef root);
  void graphFile(TFQueueRef root);


  QueueManager() { callsToEnqueueOR = 0; runNum = 1; }
  void basicCleanup()
  {   
    //std::cerr << LifeSupport.size() << std::endl;
    LifeSupport.clear();  
  }

  virtual ~QueueManager() { basicCleanup(); }
  
private:
  std::map<std::string,QAndRef> globalHashMap;
  std::map<uint64,TFQueueObj*> tagToQueue;
  std::map<uint64,uint64> queuesSubsumedBy; // queueSubsumedBy[A] == B means the elements of A went into B
                                       // if B == 0, then A was deduced to be FALSE.
  queue<QInfo> unprocessedInfo;

  //-- Implementation stuff!
public: 
  std::set<TFQueueRef> LifeSupport;
private:
  void subsume(TFQueueRef Q, TFQueueRef victim, std::set<QOrObj*> &OrsToDrop);
  int runNum; //-- This is used for marking in DFS
  TFQueueObj* DFS(TFQueueRef root, TFQueueObj* q1, TFQueueObj* q2, 
		  std::set<TFQueueObj*> &SCC);
  void getOrsInSCC(TFQueueObj *src, TFQueueObj* pq1, TFQueueObj* pq2, std::set<TFQueueObj*> &SCC, 
		   std::set<QOrObj*> &OrsInSCC);
  std::set<TFQueueObj*> getOrSig(QOrRef A);
  bool sameSet(std::set<TFQueueObj*> &A, std::set<TFQueueObj*> &B);
  std::set<QOrRef> equivOrs(QOrRef A);
  void mergeEquivOrs(std::vector<std::set<QOrRef> > &equivSets);



  friend class TFQueueObj;
  int callsToEnqueueOR;

  // EXPERIMENTAL
 public:
  std::map<uint64, std::set<IntPolyRef> > nonZeroDeductions;  
};

class QueueManagerAndChooser : public QueueManager
{
public:
  virtual void recordAndEnqueued(QAndRef A) = 0;
  virtual QAndRef next() = 0; 
};

class BFQueueManager : public QueueManagerAndChooser
{
private:
  queue<QAndRef> andsToExpand;

public:
  void recordAndEnqueued(QAndRef A) { QueueManager::recordAndEnqueued(A); andsToExpand.push(A); }
  QAndRef next() 
  {     
    QAndRef n;
    TFQueueObj* np = NULL;
    while(np == NULL)
    {
      if (andsToExpand.empty()) return NULL;
      n = andsToExpand.front(); 
      andsToExpand.pop(); 
      np = find(n);
    }
    return n; 
  }
  ~BFQueueManager() { while(!andsToExpand.empty()) andsToExpand.pop(); basicCleanup(); }
};

class DFQueueManager : public QueueManagerAndChooser
{
private:
  std::stack<QAndRef> andsToExpand;

public:
  void recordAndEnqueued(QAndRef A) { QueueManager::recordAndEnqueued(A); andsToExpand.push(A); }
  QAndRef next() 
  {     
    QAndRef n;
    TFQueueObj* np = NULL;
    while(np == NULL)
    {
      if (andsToExpand.empty()) return NULL;
      n = andsToExpand.top(); 
      andsToExpand.pop(); 
      np = find(n);
    }
    return n; 
  }
  ~DFQueueManager() { while(!andsToExpand.empty()) andsToExpand.pop();  basicCleanup(); }
};

// This uses lowest priority first!
template<class comp>
class PrioritySearchQueueManager : public QueueManagerAndChooser
{
private:
  class negcomp { public: comp F; bool operator()(QAndRef a, QAndRef b) { return !F(a,b); } };
  priority_queue<QAndRef,std::vector<QAndRef>, negcomp> andsToExpand;

public:
  void recordAndEnqueued(QAndRef A) { QueueManager::recordAndEnqueued(A); andsToExpand.push(A); }
  QAndRef next() 
  {     
    QAndRef n;
    TFQueueRef np = NULL;
    while(np.is_null())
    {
      if (andsToExpand.empty()) return NULL;
      n = andsToExpand.top(); 
      andsToExpand.pop(); 
      np = find(n);
    }
    return n; 
  }
  ~PrioritySearchQueueManager() { while(!andsToExpand.empty()) andsToExpand.pop();  basicCleanup(); }
};


// Fewest quantified variables first, to be used with priority search
class FewestQuantifiedVariablesFirst
{
public:
  bool operator()(QAndRef a, QAndRef b)
  {
    if (a->deadMark) return true;
    if (b->deadMark) return false;
    int va = a->QVars.numElements();
    int vb = b->QVars.numElements();
    int pa = a->getPrintedLength();
    int pb = b->getPrintedLength();
    return 
      (va < vb) ||
      (va == vb &&  pa < pb) ||
      (va == vb &&  pa == pb && a->tag < b->tag);      
  }
};


class SmallestDimensionFirst
{
public:
  bool operator()(QAndRef a, QAndRef b)
  {
    FindEquations Ea; Ea(a);
    FindEquations Eb; Eb(b);
    int va = a->getVars().numElements() - Ea.res.size();
    int vb = b->getVars().numElements() - Eb.res.size();
    int pa = a->getPrintedLength();
    int pb = b->getPrintedLength();
    return (va < vb) || (va == vb &&  pa < pb) || (va == vb &&  pa == pb && a->tag < b->tag);
  }
};





QAndObj* findUnexpanded(TFQueueRef Q);
void printUnexpandedNodes(TFQueueRef Q);

//extern QueueManager *globalQM;
extern uint64 totalQAndsGenerated;

}//end namespace tarski

#endif
