#include "search.h"
#include "../formula/linearSubs.h"
#include "../formula/formmanip.h"
#include "../poly/md5digest.h"
#include "rewrite.h"
#include <stack>

using namespace std;

namespace tarski {

extern void graphFile(TFQueueRef);

TFQueueObj *badguy = (TFQueueObj*)1;


TFQueueObj* QueueManager::currentQueue(uint64 pQ)
{
  uint64 tmp = pQ;
  stack<map<uint64,uint64>::iterator> S;
  map<uint64,uint64>::iterator itr; 
  while((itr = queuesSubsumedBy.find(tmp)) != queuesSubsumedBy.end())
  {
    S.push(itr);
    tmp = itr->second;
  }
  while(!S.empty())
  {
    S.top()->second = tmp;
    S.pop();
  }
  if (tmp == pQ)
    return tagToQueue[pQ];
  else if (tmp == 0)
    return NULL;
  else
    return tagToQueue[tmp];
}

bool QueueManager::isNew(QAndRef A)
{
  return globalHashMap.find(A->getHash()) == globalHashMap.end();
}

TFQueueObj* QueueManager::find(QAndRef A)
{
  QAndRef Ap = globalHashMap.find(A->getHash())->second;
  return currentQueue(Ap->parentQueueTag);
}

void QueueManager::notify(QInfo I)
{
  unprocessedInfo.push(I);
}

void QueueManager::enqueueOR(vector<QAndRef> &CA,TFQueueRef Q) // Note: this may create loops!
{
  ++callsToEnqueueOR;
  //cout << callsToEnqueueOR << endl;

  // Create TM, which is CA with all the known false disjuncts removed
  vector<QAndRef> TM;
  for(unsigned int i = 0; i < CA.size(); ++i)
  {
    if (constValue(CA[i]->F) != FALSE && (isNew(CA[i]) || find(CA[i]) != NULL))
      TM.push_back(CA[i]);
    else
    {
      // We've actually just deduced something here!
      if (dynamic_cast<PSplitObj*>(&*(CA[i]->PR)))
      {
	if (verbose) {
	  cout << "Deduced inconsistent in " << Q->tag << ": ";
	  CA[i]->PR->write();
	  cout << endl; }
      }
    }
   
  }
  
  if (TM.size() == 0)
  {
    Q->constValue = new TConstObj(FALSE);
    notify(QInfo::equivFalse(Q->tag));
  }
  else if (TM.size() == 1)
  {
    map<string,QAndRef>::iterator itr = globalHashMap.find(TM[0]->getHash());
    if (itr == globalHashMap.end())
      Q->enqueue(TM[0]);
    else if (Q->tag != itr->second->parentQueueTag)
      notify(QInfo::equiv(Q->tag,itr->second->parentQueueTag));
  }
  else
  {
    // Dispense with the special case that a disjunct is in the parent queue
    // In this case, we do nothing!
    for(unsigned int i = 0; i < TM.size(); ++i)
    {
      map<string,QAndRef>::iterator itr = globalHashMap.find(TM[i]->getHash());
      if (itr == globalHashMap.end()) continue;
      TFQueueObj *tmp = currentQueue(itr->second->parentQueueTag); //-- FIXED
      if (tmp == &*Q) return;
    }

    QOrRef newOR = new QOrObj(&*Q);
    for(unsigned int i = 0; i < TM.size(); ++i)
    {
      map<string,QAndRef>::iterator itr = globalHashMap.find(TM[i]->getHash());
      if  (itr != globalHashMap.end())
      {
	TFQueueObj *tmp = currentQueue(itr->second->parentQueueTag); //-- FIXED
	TFQueueObj *dummy = new TFQueueObj(this);
	notify(QInfo::equiv(tmp->tag,dummy->tag));
	newOR->add(dummy);
      }
      else
	newOR->add(new TFQueueObj(TM[i],&*newOR));
    }
    Q->enqueue(newOR);
  }
}

void QueueManager::recordAndEnqueued(QAndRef A)
{
  globalHashMap[A->getHash()] = A;
}


// Return NULL if there is no cycle.  If there is a cycle, return
// the one of q1 and q2 that is an ancestor of the other (before
// the merge).  SCC is filled with all the Queues (besides q1 and q2)
// that are in the strongly connected component containing the cycle.
TFQueueObj* QueueManager::DFS(TFQueueRef root, TFQueueObj* q1, TFQueueObj* q2, 
		set<TFQueueObj*> &SCC)
{
  QueueManager* globalQM = this;
  runNum += 3;
  int count = 0;
  TFQueueObj* src = NULL;
  stack<TFQueueRef> S;
  vector<TFQueueObj*> path;
  q1->inscc = runNum;
  q2->inscc = runNum;
  root->mark = runNum;
  S.push(root);
  while(!S.empty())
  {
    TFQueueRef N = S.top(); S.pop(); 
    // N has been processed, and all of its children have completed!
    if (N->mark == runNum + 1) 
    { 
      count++;
      N->mark++; 
      if (path.size() > 0)
	path.pop_back();
      if (N->inscc == runNum && path.size() > 0 && path[path.size()-1]->inscc != runNum)
      {
	path[path.size()-1]->inscc = runNum;
	SCC.insert(path[path.size()-1]);
      }
      continue; 
    }
    // N has not yet been processed.
    N->mark = runNum + 1;
    S.push(N);
    if ((src == NULL && (&*N == q1 || &*N == q2))
	|| src == &*N
	|| path.size() != 0)
    { 
      path.push_back(&*N); 
    }
    for(set<QNodeRef>::iterator itr = N->nodes.begin(); itr != N->nodes.end(); ++itr)
    {
      QOrRef qn = asa<QOrObj>(*itr);
      if (!qn.is_null())
	for(set<TFQueueRef>::iterator qtr = qn->disjuncts.begin(); qtr != qn->disjuncts.end(); ++qtr)
	{
	  TFQueueObj *ptr = globalQM->currentQueue((*qtr)->tag);
	  if (ptr == NULL)
	  {
	    cerr << "Unexpected condition in DFS!" << endl;
	    exit(1);
	  }
	  if (ptr->inscc == runNum && path.size() > 0)
	  {
	    src = path[0];
	    if (N->inscc != runNum) { N->inscc = runNum; SCC.insert(&*N); }
	  }
	  else
	  {
	    if ((*qtr)->mark < runNum) { (*qtr)->mark = runNum; S.push(*qtr); } 
	    else if ((*qtr)->mark == runNum + 1) 
	    { 
	      cerr << "Illegal cycle!" << endl; 
	    }
	  }
	}
    }
  }
  if (verbose) { cout << count << " nodes searched in graph!" << endl; }
  return src;
}

// An "Or-in-the-connected-component" is an Or whose parent is a Queue in
// the connected component and which has a disjunct-queue in the connected
// component.
void QueueManager::getOrsInSCC(TFQueueObj *src, TFQueueObj* pq1, TFQueueObj* pq2, 
			       set<TFQueueObj*> &SCC, set<QOrObj*> &OrsInSCC)
{
  // Set Q to a vector of pointers to the nodes (other than source) in SCC
  vector<TFQueueObj*> Q;
  if (src == NULL) return;
  TFQueueObj *sink = (src == pq1 ? pq2 : pq1);  
  Q.push_back(sink);
  for(set<TFQueueObj*>::iterator qitr = SCC.begin(); qitr != SCC.end(); ++qitr)
    Q.push_back(*qitr);
  
  // Loop through the Or nodes that have a disjunct-queue in the SCC, and add to
  // set OrsInSCC those whose parentQueue is also in the SCC.
  for(unsigned int i = 0; i < Q.size(); ++i)
    for(set<QOrObj*>::iterator oitr = Q[i]->parentOrs.begin(); oitr != Q[i]->parentOrs.end(); ++oitr)
      if ((*oitr)->parentQueue->inscc == src->inscc)
	OrsInSCC.insert(*oitr);
}





/************************************************************
 * START: The following set of functions are just about keeping
 * ORs unique!
 ************************************************************/
set<TFQueueObj*> QueueManager::getOrSig(QOrRef A)
{
  QueueManager* globalQM = this;
  set<TFQueueObj*> sigA;
  for(set<TFQueueRef>::iterator itr = A->disjuncts.begin(); itr != A->disjuncts.end(); ++itr)
    sigA.insert(globalQM->currentQueue((*itr)->tag));
  return sigA;
}

bool QueueManager::sameSet(set<TFQueueObj*> &A, set<TFQueueObj*> &B)
{
  if (A.size() != B.size()) return false;
  for(set<TFQueueObj*>::iterator itrA = A.begin(), itrB = B.begin(); itrA != A.end(); ++itrA, ++itrB)
    if ((*itrA) != (*itrB)) return false;
  return true;
}

set<QOrRef> QueueManager::equivOrs(QOrRef A)
{
  set<QOrRef> equivA;
  set<TFQueueObj*> sigA = getOrSig(A);
  set<TFQueueRef>::iterator itr = A->disjuncts.begin();
  TFQueueRef Q1 = (*itr);
  for(set<QOrObj*>::iterator oitr = Q1->parentOrs.begin(); oitr != Q1->parentOrs.end(); ++oitr)
  {
    set<TFQueueObj*> sigB = getOrSig(*oitr);
    if (sameSet(sigA,sigB))
      equivA.insert(*oitr);
  }
  return equivA;
}

// If two ORs in the same Queue are equiv, just throw
// one out.  If they're in different queues, the two
// Queues have been deduced equivalent!  Notify of that,
// and still throw out one of the ORs
void QueueManager::mergeEquivOrs(vector<set<QOrRef> > &equivSets)
{
  QueueManager* globalQM = this;
  for(unsigned int i = 0; i < equivSets.size(); ++i)
  {
    set<QOrRef>::iterator itr = equivSets[i].begin();
    QOrRef chosenOne = (*itr);
    TFQueueObj* copq = globalQM->currentQueue(chosenOne->parentQueue->tag);
    for(++itr; itr != equivSets[i].end(); ++itr)
    {
      QOrRef next = (*itr);
      TFQueueObj* npq = globalQM->currentQueue(next->parentQueue->tag); 
      if (copq != npq)
      {
	globalQM->notify(QInfo::equiv(copq->tag,npq->tag));
	if (verbose) { cout << "Equiv deduced via equivalent ORs!" << endl; }
      }
      // take "next" out of its parent queue, and out of its child queues' parentOrs lists
      npq->nodes.erase(next); 
      for(set<TFQueueRef>::iterator qitr = next->disjuncts.begin(); qitr != next->disjuncts.end(); ++qitr)
	(*qitr)->parentOrs.erase(&*next);
    }
  }
}
/************************************************************
 * END:  set of functions are just about keeping ORs unique!
 ************************************************************/




void QueueManager::reorganize(TFQueueRef root)
{
  QueueManager* globalQM = this;
  while(!unprocessedInfo.empty())
  {
    // Dequeue next piece of info, updating references if need be
    QInfo I = unprocessedInfo.front(); unprocessedInfo.pop();
    TFQueueObj *pq1 = globalQM->currentQueue(I.q1); if (pq1 != NULL) I.q1 = pq1->tag; else I.q1 = 0;
    TFQueueObj *pq2 = globalQM->currentQueue(I.q2); if (pq2 != NULL) I.q2 = pq2->tag; else I.q2 = 0;

    // Case 1: Info I declares a queue to be false
    if (I.typeFalse())
    {
      if (pq1 == NULL) continue; // already taken care of!

      if (verbose) {// DEBUGGING
	cout << "Queue " << I.q1 << " deduced FALSE: ";
	cout << endl; }

      // Officially record I.q1 as FALSE, and arrange that whole comp to terminates if I.q1 is root
      queuesSubsumedBy[I.q1] = 0;
      if (I.q1 == root->tag) { pq1->constValue = new TConstObj(FALSE); return; }

      // We don't want the queue I.q1 to die while we're in the for loop, so ...
      TFQueueRef tmpref = pq1;
      
      // Remove pq1 from each parentOr & clean up the mess if the size of the or drops to 0 or 1
      for(set<QOrObj*>::iterator oitr = pq1->parentOrs.begin(); oitr != pq1->parentOrs.end(); oitr++)
      {
	QOrRef D = *oitr;
	D->disjuncts.erase(pq1);
	// Note: at this point we might like to pause to reflect that we've just
	// "discovered" that whatever assumption we added with this disjunct when we
	// created D in its parent queue, that assumption is inconsistent in its
	// parent queue.
if (!pq1->original.is_null() && dynamic_cast<PSplitObj*>(&*(pq1->original->PR)))
 {
   if (verbose||false) {
     cout << "Deduced inconsistent in " << D->parentQueueTag << ": ";
     pq1->original->PR->write();
     cout << endl;

     PSplitObj* ps1 = dynamic_cast<PSplitObj*>(&*(pq1->original->PR));
     TFQueueObj* tmpp = find(pq1->original->PR->predecessor());
     if (tmpp != NULL) 
       nonZeroDeductions[tmpp->tag].insert(ps1->getZeroFactor());
   }
 }
        // If # disjuncts in Or drops to 1/0 we have an equivalence/false deduction 
	if (D->disjuncts.size() == 1)
	  notify(QInfo::equiv(D->parentQueueTag,(*D->disjuncts.begin())->tag));
	else if (D->disjuncts.size() == 0)
	  notify(QInfo::equivFalse(D->parentQueueTag));
      }

      // Deleting pq1 from one of its parentOrs may leave that OR identical to some other
      vector<set<QOrRef> > equivSets;
      runNum += 1;
      for(set<QOrObj*>::iterator oitr = pq1->parentOrs.begin(); oitr != pq1->parentOrs.end(); oitr++)
      {
	if ((*oitr)->mark == runNum || (*oitr)->disjuncts.size() < 2) continue;
	set<QOrRef> equivA = equivOrs(*oitr);
	for(set<QOrRef>::iterator titr = equivA.begin(); titr != equivA.end(); ++titr) 
	  (*titr)->mark = runNum;
	equivSets.push_back(equivA);
      }
      mergeEquivOrs(equivSets);

      // Since each child of pq1 is FALSE, each queue in each child is also false.
      // Thus, they must all be marked as FALSE!
      for(set<QNodeRef>::iterator nitr = pq1->nodes.begin(); nitr != pq1->nodes.end(); ++nitr)
      {
	QOrRef on = asa<QOrObj>(*nitr);
	if (!on.is_null())
	  for(set<TFQueueRef>::iterator qitr = on->disjuncts.begin(); qitr != on->disjuncts.end(); ++qitr)
	    notify(QInfo::equivFalse(*qitr));
      }
    }

    // Case 2: Info I declares two queues to be equivalent
    else if (I.typeEquiv())
    {
      if (I.q1 == I.q2) continue; // This equivalence has already been processed!
      if (I.q1 == 0 || I.q2 == 0)
      {
	if (I.q1 == 0 && I.q2 != 0) notify(QInfo::equivFalse(I.q2));
	if (I.q2 == 0 && I.q1 != 0) notify(QInfo::equivFalse(I.q1));
	continue;
      }
      if (verbose) { cerr << "Queue " << I.q1 << " and queue " << I.q2 << " deduced equivalent" << endl; }
      set<TFQueueObj*> SCC;
      TFQueueObj* sroot; // Ultimately, this will be a pointer to the Queue we keep!
      TFQueueObj* src = DFS((sroot = pq1),pq1,pq2,SCC);
      if (src == NULL)
	src = DFS((sroot = pq2),pq1,pq2,SCC);
      runNum += 3;

      // This is a hack, but if one guy is on life support, and the other is the root, we lose the root!
      if ((pq1->tag == root->tag && sroot != pq1) || (pq2->tag == root->tag && sroot != pq2)) continue;

      // Case 1: Neither Queue is an ancestor of the other
      if (src == NULL)
      { 
	set<QOrObj*> DummyOrsInSCC;
	if (pq1->nodes.size() >= pq2->nodes.size())
	{  subsume(pq1,pq2,DummyOrsInSCC); sroot = pq1; }
	else
	{ subsume(pq2,pq1,DummyOrsInSCC); sroot = pq2; }
      }
      
      // Case 2: One Queue is an ancestor of the other (src is equal to the ancestor)
      else
      {
	stack<TFQueueRef> tempLifeSupport;
	for(set<TFQueueObj*>::iterator qitr = SCC.begin(); qitr != SCC.end(); ++qitr)
	  tempLifeSupport.push(*qitr);
	if (verbose) { cout << "Found Cycle of length " << SCC.size()+1 << endl; }
	set<QOrObj*> OrsInSCC;
	getOrsInSCC(src,pq1,pq2,SCC,OrsInSCC);
	if (pq1 == src)
	  subsume(pq1,pq2,OrsInSCC);
	else
	  subsume(pq2,pq1,OrsInSCC);
	for(set<TFQueueObj*>::iterator qitr = SCC.begin(); qitr != SCC.end(); ++qitr)
	  subsume(src,*qitr,OrsInSCC);
      }

      // Deleting pq1 from one of its parentOrs may leave that OR identical to some other
      vector<set<QOrRef> > equivSets;
      runNum += 1;
      for(set<QOrObj*>::iterator oitr = sroot->parentOrs.begin(); oitr != sroot->parentOrs.end(); oitr++)
      {
	if ((*oitr)->mark == runNum || (*oitr)->disjuncts.size() < 2) continue;
	set<QOrRef> equivA = equivOrs(*oitr);
	for(set<QOrRef>::iterator titr = equivA.begin(); titr != equivA.end(); ++titr) 
	  (*titr)->mark = runNum;
	equivSets.push_back(equivA);
      }
      mergeEquivOrs(equivSets);
    }
  }
}

// Assume neither currently FALSE
void QueueManager::subsume(TFQueueRef Q, TFQueueRef victim, set<QOrObj*> &OrsToDrop)
{
  QueueManager* globalQM = this;

  // Transfer all nodes from victim to Q, except ORs in the OrsToDrop set.
  for(set<QNodeRef>::iterator itr = victim->nodes.begin(); itr != victim->nodes.end(); ++itr)
  {
    QOrRef aq = asa<QOrObj>(*itr);
    if (aq.is_null() || OrsToDrop.find(&*aq) == OrsToDrop.end())
    {
      (*itr)->parentQueue = &*Q;
      (*itr)->parentQueueTag = Q->tag;
      Q->nodes.insert(*itr);
    }
    else
    {
      if (verbose) { cout << "Here's the point!" << endl; }
    }
  }
  victim->nodes.clear();
  
  // What do I do with all the QOrNodes that refer to victim? I'll update the
  // "disjuncts" set, but the "next" queue will have to check for updates as it's
  // being evaluated.
  for(set<QOrObj*>::iterator itr = victim->parentOrs.begin(); itr != victim->parentOrs.end(); ++itr)
  {
    QOrRef po = *itr;
    if ((*itr)->disjuncts.find(victim) == (*itr)->disjuncts.end())
    { cerr << "Inconsistent data structure in OR!" << endl; exit(1); }
    (*itr)->disjuncts.erase(victim);
    if (OrsToDrop.find(*itr) == OrsToDrop.end())
    {
      Q->parentOrs.insert(*itr);
      (*itr)->disjuncts.insert(Q);
    }
    else
    {
      // If (*itr) lives in something other than Q, it's effectively cut out by
      // the above.  If it lives in Q, however, we need to cut it out explicitly.
      if ((*itr)->parentQueue == &*Q) { Q->nodes.erase(*itr); (*itr)->deadMark = 1; }

      // I've got to also deal with the queues that might go away when this OR
      // disappears.  Those that would get put in LifeSupport.
      for(set<TFQueueRef>::iterator ditr = (*itr)->disjuncts.begin(); ditr != (*itr)->disjuncts.end(); ++ditr)
      {
	(*ditr)->parentOrs.erase(*itr);
	if ((*ditr)->parentOrs.size() == 0)
	{ 
	  LifeSupport.insert(*ditr); 
	  //cout << "LS: " << (*ditr)->tag << endl;
	}
      }
    }
  }
  victim->parentOrs.clear();
  
  // Set victim's alias to Q
  globalQM->queuesSubsumedBy[victim->tag] = Q->tag;
}


uint64 totalQNodes = 0, numGenericSubs = 0, numDegenerateSubs = 0;
uint64 totalQAndsGenerated = 0;

Normalizer* defaultNormalizer = NULL;

/********************************************************************************
 * QNode Implementations
 ********************************************************************************/
string QNodeObj::toString()
{
  ostringstream sout;
  PushOutputContext(sout);
  write();
  PopOutputContext();
  return sout.str();
}

void QNodeObj::setHash()
{
  string form = toString();  
  istringstream sin(form);
  hash = md5_digest(sin);
  printedLength = form.length();
}

/********************************************************************************
 * TFQueue Implementations
 ********************************************************************************/
TFQueueObj::TFQueueObj(QueueManager *_globalQM) 
{ globalQM = _globalQM; original = NULL; inscc = mark = 0; globalQM->tagToQueue[tag] = this; } // Dummy queue 
TFQueueObj::TFQueueObj(QAndRef N, QOrObj *parent)
{ 
  globalQM = parent->globalQM;
  globalQM->tagToQueue[tag] = this;
  if (parent != NULL) parentOrs.insert(parent);
  enqueue(N); 
  original = N;
  ++totalQNodes;
  inscc = mark = 0;
}
TFQueueObj::~TFQueueObj() {
  //cout << "Queue " << tag << " deleted!" << endl;
 }
QAndRef TFQueueObj::getOriginal() { return original; }

void TFQueueObj::enqueue(QNodeRef t)
{ 
  t->parentQueue = this;
  t->parentQueueTag = this->tag;

  QAndRef at = asa<QAndObj>(t);
  if (!at.is_null()) // Case t is an AND
  {
    nodes.insert(t);
    globalQM->recordAndEnqueued(at);
  } 
  else if (nodes.find(t) == nodes.end()) // Case t is a new OR
    nodes.insert(t);
}



/********************************************************************************
 * QOr Implementations
 ********************************************************************************/
QOrObj::QOrObj(QAndRef a, QAndRef b, TFQueueObj *parent) 
{ 
  globalQM = parent->globalQM;
  parentQueue = parent;
  parentQueueTag = parent->tag;
  add(new TFQueueObj(a,this)); add(new TFQueueObj(b,this)); 
  mark = 0;
}

QOrObj::QOrObj(TFQueueObj *parent)
{ 
  globalQM = parent->globalQM;
  parentQueue = parent; 
  parentQueueTag = parent->tag; 
  mark = 0;
}

extern bool finalcleanup;

QOrObj::~QOrObj() 
{ 
  if (finalcleanup) return;
  // This is a total hack! I wonder if it's needed now?
  for(set<TFQueueRef>::iterator itr = disjuncts.begin(); itr != disjuncts.end(); ++itr)
  {
    TFQueueObj* ptr = globalQM->currentQueue((*itr)->tag);
    if (ptr != NULL)// && !finalcleanup)
    {
      ptr->parentOrs.erase(this);
      // Is this Queue's gonna die if we don't do something about it?  A Queue should only
      // die because we process the "equiv false" statement.
      if (ptr->parentOrs.size() == 0) 
      {
	globalQM->LifeSupport.insert(ptr);
      }
    }
  }
  
}
void QOrObj::add(TFQueueRef nQ)
{ 
  disjuncts.insert(nQ); 
  nQ->parentOrs.insert(this); 
}  



/********************************************************************************
 * QAnd Implementations
 ********************************************************************************/  
QAndObj::  QAndObj(TFormRef _F, VarSet _QVars, VarSet _UVars, ParentageRef _PR)
{ 
  F = _F; expanded = false; children = 0; 
  QVars = _QVars & F->getVars();
  UVars = _UVars;
  PR = _PR;
}
bool QAndObj::done() { return expanded; }

void QAndObj::write(bool withQuantifiers) 
{
  VarSet QVarsAppearing = QVars & getVars();
  if (withQuantifiers && QVarsAppearing.numElements() != 0)
  {
    cout << "ex ";
    VarSet::iterator vitr = QVarsAppearing.begin();
    cout << F->getPolyManagerPtr()->getName(*vitr);
    for(++vitr; vitr != QVarsAppearing.end(); ++vitr)
    {
      string tmp = F->getPolyManagerPtr()->getName(*vitr);
      cout << ", " << tmp;
    }
  } 
  F->write(); 
}


/***************************************************************
 * 
 ***************************************************************/
QAndObj* findUnexpanded(TFQueueRef Q)
{

  // Enqueue all ORNodes, and any none-"done" ANDNodes
  queue<QNodeRef> A;
  for(set<QNodeRef>::iterator itr = Q->nodes.begin(); itr != Q->nodes.end(); ++itr)
    if (asa<QOrObj>(*itr) != 0 || !asa<QAndObj>(*itr)->done())
      A.push(*itr);

  while(!A.empty())
  {
    QNodeRef next = A.front(); A.pop();
    QAndObj *ap;
    if ((ap = asa<QAndObj>(next)) != NULL)
      return ap;
    else
    {
      QOrObj *pOr = asa<QOrObj>(next);
      for(set<TFQueueRef>::iterator qitr = pOr->disjuncts.begin(); qitr != pOr->disjuncts.end(); ++qitr)
	for(set<QNodeRef>::iterator itr = (*qitr)->nodes.begin(); itr != (*qitr)->nodes.end(); ++itr)
	  if (asa<QOrObj>(*itr) != 0 || !asa<QAndObj>(*itr)->done())
	    A.push(*itr);
    }
  }
  
  return NULL;
}

void printUnexpandedNodes(TFQueueRef Q)
{
  for(set<QNodeRef>::iterator i = Q->nodes.begin(); i != Q->nodes.end(); ++i)
  {
    if (asa<QAndObj>(*i) != 0)
    {
      QAndRef a = asa<QAndObj>(*i);
      if (!a->done())
      {
	cout << "Queue " << Q->tag << " has unexpanded formula: ";
	a->QNodeObj::write();
	cout << endl;
      }
    }
    else
    {
      QOrRef o = asa<QOrObj>(*i);
      for(set<TFQueueRef>::iterator ditr = o->disjuncts.begin(); ditr != o->disjuncts.end(); ++ditr)
	printUnexpandedNodes(*ditr);
    }
  }
}

}//end namespace tarski
