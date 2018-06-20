
#include "search.h"
#include "rewrite.h"
#include "grade.h"

using namespace std;

namespace tarski {
 
  static int runNum = 1;

  class fgcmp
  {
  public:
    bool operator()(pair<TFormRef,double> a, pair<TFormRef,double> b)
    {
      return a.second < b.second;
    }
  };

  void exploreQueue(TFQueueRef Q, VarSet QVars, FormulaGrader &FG)
  {
    fgcmp C;
    set<pair<TFormRef,double>,fgcmp> S(C);
    for(set<QNodeRef>::iterator itr = Q->nodes.begin(); itr != Q->nodes.end(); ++itr)
    {
      QAndRef AN = asa<QAndObj>(*itr);
      if (!AN.is_null())
      {
	S.insert(pair<TFormRef,double>(AN,FG.grade(AN->F,QVars)));
      }
      else
      {
	QOrRef ON = asa<QOrObj>(*itr);
	double gON = 0;
	TOrRef D = new TOrObj;
	for(set<TFQueueRef>::iterator qtr = ON->disjuncts.begin(); qtr != ON->disjuncts.end(); ++qtr)
	{
	  pair<TFormRef,double> rres = minFormAndGrade(*qtr,QVars,FG);
	  D->OR(rres.first);
	  gON = max(rres.second,gON);
	}
	S.insert(pair<TFormRef,double>(D,gON));
      }
    }
  
    for(set<pair<TFormRef,double>,fgcmp>::iterator itr = S.begin(); itr != S.end(); ++itr)
    {
      cout << "####################################################################" << endl;
      cout << "Grade = " << itr->second << endl;
      itr->first->write();
      cout << endl;
      printDerivationIfAnd(itr->first);
    }

  }





  void graphFile(ostream &out, TFQueueRef root,  int &runNum)
  {
    runNum += 3;
    out << "graph: {" << endl;
    map<GC_Obj*,int> mark;
    queue<GC_Obj*> nodes;
    queue<pair<GC_Obj*,GC_Obj*> > edges;
    stack<GC_Obj*> S;
    mark[&*root] = 1;
    S.push(&*root);
    while(!S.empty())
    {
      GC_Obj* N = S.top(); S.pop();
      if (mark[N] == 2) { mark[N] = 3; nodes.push(N); continue; }
      mark[N] = 2;
      S.push(N);
      queue<GC_Obj*> K;
      TFQueueObj* qp = dynamic_cast<TFQueueObj*>(N);
      QOrObj* op = dynamic_cast<QOrObj*>(N);
      if (qp != NULL)
      {
	for(set<QNodeRef>::iterator itr = qp->nodes.begin(); itr != 
	      qp->nodes.end(); ++itr)
	  if (asa<QOrObj>(*itr) != NULL)
	    K.push(&**itr);
      }
      else
      {
	for(set<TFQueueRef>::iterator itr = op->disjuncts.begin();
	    itr != op->disjuncts.end(); ++itr)
	{
	  TFQueueObj* tmp1 = &**itr;
	  TFQueueObj* tmp2 = root->globalQM->currentQueue(tmp1->tag);
	  K.push(root->globalQM->currentQueue((*itr)->tag));
	}
      }
      while(!K.empty())
      {
	GC_Obj* nn = K.front(); K.pop();
	edges.push(pair<GC_Obj*,GC_Obj*>(N,nn));
	if (mark.find(nn) == mark.end()) 
	{ mark[nn] = 1; S.push(nn); } 
	else if (mark[nn] == 2) 
	{ 
	  cerr << "Loop! Must be an error!" << endl; 
	}
      }
    }
    cout << " Num Nodes = " << nodes.size() << endl;
    while(!nodes.empty())
    {
      out << "node: { title: \"" << nodes.front()->tag << "\" ";
      if (dynamic_cast<QOrObj*>(nodes.front()) != NULL)
	out << "shape: ellipse ";
      out << "}" << endl;
      nodes.pop();
    }

    while(!edges.empty())
    {
      out << "edge: { "
	  << "sourcename: \"" << edges.front().first->tag << "\" " 
	  << "targetname: \"" << edges.front().second->tag << "\" " 
	  << " }" << endl;
      edges.pop();
    }
    out << "}" << endl;
    runNum += 3;
  }

  void QueueManager::graphFile(TFQueueRef root)
  {
    ofstream fout("tmp.gdl");
    tarski::graphFile(fout,root,runNum);
  }



  // Breadth-first search (Remember, this is supposed to be a DAG!)
  void QueueManager::graphStats(TFQueueRef root)
  {
    runNum += 3;
    vector<int> andSizes;
    vector<TFQueueRef> andSizeQs;
    int totalAnds = 0;
    int totalOrs = 0;
    int totalQueues = 0;
    queue<TFQueueRef> S;  
    S.push(root);
    root->mark = runNum;
    while(!S.empty())
    {
      TFQueueRef N = S.front(); S.pop();
      if (N->mark > runNum) { cerr << "In graphStats: This should never happen!" << endl; }
      ++totalQueues;
      N->mark = runNum+1;

      // Process N!
      int numNAnds = 0;
      int numNOrs = 0;
      for(set<QNodeRef>::iterator itr = N->nodes.begin(); itr != N->nodes.end(); ++itr)
      {
	if (asa<QOrObj>(*itr) != NULL)
	{
	  ++numNOrs;
	  QOrObj* op = asa<QOrObj>(*itr);
	  for(set<TFQueueRef>::iterator ditr = op->disjuncts.begin(); ditr != op->disjuncts.end(); ++ditr)
	    if ((*ditr)->mark < runNum)
	    {
	      S.push(*ditr);
	      (*ditr)->mark = runNum;
	    }
	}
	else
	  ++numNAnds;
      }
      totalAnds += numNAnds;
      totalOrs += numNOrs;
      andSizes.push_back(numNAnds);
      andSizeQs.push_back(N);
    }
    cout << "totalAnds   = " << totalAnds << endl;
    cout << "totalOrs    = " << totalOrs << endl;
    cout << "totalQueues = " << totalQueues << endl;
    //   for(int i = 0; i < andSizes.size(); ++i)
    //   {
    //     cout << andSizes[i] << '\t' << andSizeQs[i]->tag << endl;

    //     // Totally temporary!
    //     if (true)
    //     for(set<QNodeRef>::iterator itr = andSizeQs[i]->nodes.begin(); itr != andSizeQs[i]->nodes.end(); ++itr)
    //     {
    //       if (asa<QAndObj>(*itr) != NULL)
    //       {
    // 	cout << ">>> ";
    // 	QAndRef tmp(asa<QAndObj>(*itr));
    // 	tmp->write(cout);
    // 	cout << endl;
    //       }
    //     }
    //   }
    runNum += 3;
  }
}//end namespace tarski

