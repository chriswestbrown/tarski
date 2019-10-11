/**
 * @file opennucad.h
 * This file defines the basic classes representing Open NuCADs.
 */
#ifndef _OC_ONUCAD_
#define _OC_ONUCAD_

#include "builder.h"
#include <sstream>
#include "../formula/writeForQE.h"
#include "nnet.h"

namespace tarski {

 
class NodeObj; typedef GC_Hand<NodeObj> NodeRef;
/** A class for representing a node the in Open NuCAD structure.
 *  The concept of a "label" for a cell in a NuCAD is crucial, and a few notes on the subject are needed.
 *  1. X and Y are used for leaf / internal nodes.  The distinction is not really important at this point.
 *  2. The "level" associated with an X/Y label is now meaningful.  It kX (or kY) means that the cell is 
 *     of the form (S x R^(n-k)) \cap P  where n is the dimension of the ambient space, S is a cell in R^k,
 *     and P is the parent cell.
 */
class NodeObj : public GC_Obj
{
 public:
  int truthValue; // TRUE/FALSE means that the cell is known to be truth-invariant with value TRUE/FALSE.
  int truthValueAtAlpha;
  NodeObj* parent;
  int splitLevel;
  char splitLabel;
  BuilderRef data;
  std::vector<NodeRef> Lchild, Uchild;
  NodeRef XYchild;

  //-- DATA FOR 2019 HPC PROJECT
  BuilderRef originalData;
  int originalTruthValue;
  int numSplitOptions;

 public:
  NodeObj(NodeObj* parent, BuilderRef data, int truthValue, int splitLevel,
	  char splitLabel
	  )
  { 
    this->parent = parent; this->data = data; this->truthValue = truthValue;
    this->splitLevel = splitLevel; this->splitLabel = splitLabel;
    XYchild = NULL;
    this->truthValueAtAlpha = truthValue;

    this->originalData = data;
    this->originalTruthValue = truthValue;
    this->numSplitOptions = 0;
  }

  // This method reverts a node back to the state it had prior to being refined
  // This is not quite completely trivial, because some "refinement" steps don't
  // change the cell bounds, but do change the cell's "pfset", i.e. the set of
  // poly's known to be sign-invariant in the cell might expand.  This function
  // actually wants to set that back to what it was originally.  NOTE:  the
  // function supports exploring the consequences of different split polynomial
  // choices.  That's why it exists.
  void revert()
  {
    this->data = this->originalData;
    this->truthValue = this->originalTruthValue;
    Lchild.clear();
    Uchild.clear();
    XYchild = NULL;
  }
  
  BuilderRef getData() { return data; }
  int getTruthValue() { return truthValue; }
  void setTruthValue(int tv) { truthValue = tv; }
  bool hasChildren() { return !XYchild.is_null(); }
  int getSplitLevel() { return splitLevel; }

  // A node's subtree level is the highest split level of any descendent node
  int getSubtreeLevel(bool initFlag = true);

  // If this node's subtree level is <= k, returns null.  Otherwise, returns
  // a node with XYchild whose split level exceeds k, but such that the XYchild
  // and its L and U std::stack siblings have subtree level <= k.
  NodeRef lowerMostNodeWithSubtreeLevelExceedingK(int k);
    
  void getLabel(std::ostringstream &sout)
  {
    if (splitLabel == 'C') { sout << 'C'; return; }
    parent->getLabel(sout);
    if (splitLabel == 'L' || splitLabel == 'U') { sout << splitLevel; }
    sout << splitLabel;
    return;
  }
  std::string getLabel()
  {
    std::ostringstream sout;
    getLabel(sout);
    return sout.str();
  }

  class ChildIterator
  {
  private:
    int which; // 0 = Lchildren, 1 = XYchild, 2 = U children, 3 = END
    int i; // index of next
    NodeRef parent;
  public:
    ChildIterator(NodeRef P) 
    { 
      parent = P; 
      i = 0; 
      if (!P->hasChildren())
	which = 3;
      else if (parent->Lchild.size() == 0)
	which = 1;
      else
	which = 0;
    }
    ChildIterator() { which = 3; parent = NULL; i = 0; } 
    static ChildIterator getEnd(NodeRef n) { ChildIterator i; i.parent = n; return i; }
    bool hasNext() { return which != 3; }    
    NodeRef next() 
    { 
      NodeRef res = NULL;
      if (which == 0) { res = parent->Lchild[i++]; if (i == parent->Lchild.size()) { which = 1; i = 0; } }
      else if (which == 1) { res = parent->XYchild; if (parent->Uchild.size() > 0) { which = 2; i = 0; } else { which = 3; i = 0; } }
      else if (which == 2) { res = parent->Uchild[i++]; if (i == parent->Uchild.size()) { which = 3; i = 0; } }
      return res;
    }
  };
  ChildIterator childBegin() { return ChildIterator(this); }
  ChildIterator childEnd() { return ChildIterator::getEnd(this); }
};


 class SplitSetChooserObj; typedef GC_Hand<SplitSetChooserObj> SplitSetChooserRef;
 class SplitSetChooserObj : public GC_Obj
 {
 public:
   virtual void chooseSplit(VarOrderRef X, NodeRef node, int dim,
			    set<IntPolyRef> &Q, int &tvAtAlpha, int &targetTruthValue) = 0;

   virtual IntPolyRef chooseNextPoly(set<IntPolyRef> &S, VarOrderRef X, NodeRef node);
 };

 class SplitSetChooserConjunction : public SplitSetChooserObj
 {
 public:
   SplitSetChooserConjunction(TAndRef C) { this->C = C; }
   void chooseSplit(VarOrderRef X, NodeRef node, int dim,
		    set<IntPolyRef> &Q, int &tvAtAlpha, int &targetTruthValue);
 private:
   TAndRef C;
 };
  
  /********* HPC LEARNING STUFF *************/
  
  class SSCCompObj; typedef GC_Hand<SSCCompObj> SSCCompRef;
  class SSCCompObj : public GC_Obj
  {
  public:
    virtual float eval(const std::vector<float> &F) = 0;
  };

  class BPCAsComp : public SSCCompObj
  {
  public:
    float eval(const std::vector<float> &F)
    {
      if (F[0] != 0) return F[0];
      if (F[1] != 0) return F[1];
      return F[3];
    }
  };

  /*
if ({['W0=1.0','sign(W9-W10)=0.0','abs(W9-W10)<2=True']}) then ({choose-first:Yes})
else if ({['sign(W6)=1.0','abs(W9-W10)<2=True','abs(W3)>1=0','abs(W4-W5)<2=True','abs(W8)>1=0','abs(W9-W10)<3=True']}) then ({choose-first:Yes})
else ({choose-first:No})
   */
  class DavesComp : public SSCCompObj
  {
  public:
    float eval(const std::vector<float> &F)
    {
      if (F[0] == 1.0 && F[9]-F[10] == 0.0 && fabs(F[9]-F[10])<2)
	return -1.0;
      else if (F[6] > 0.0 && fabs(F[9]-F[10])<2.0 && (!fabs(F[3])>1) && fabs(F[4]-F[5])<2 && (!fabs(F[8])>1) && fabs(F[9]-F[10])<3)
	return -1.0;
      else
	return 1.0;
    }
  };
  
  


  class PlayComp : public SSCCompObj
  {
  public:
    float eval(const std::vector<float> &F)
    {
      /* float w[11] = {-0.496732950211,-0.591809511185,-0.0187621116638,0.611699640751,-0.68747985363,-0.63534116745,-0.127867639065,-0.444904476404,-0.147764503956,-0.0402657389641,-0.162889301777}; */
      /* double sum = 0.0; */
      /* for(int i = 0; i < 11; ++i) */
      /* 	sum += w[i]*double(F[i]); */
      /* return sum; */
	
      if (F[6] != 0) return F[6];
      if (F[7] != 0) return F[7];
      return F[8];
    }
  };

  class RandomComp : public SSCCompObj
  {
    std::vector<float> W;
  public:
    float eval(const std::vector<float> &F)
    {
      if (W.size() == 0)
	for(int i = 0; i < F.size(); ++i)
	  W.push_back(2.0*(rand()/float(RAND_MAX) - 0.5));
      float sum = 0.0f;
      for(int i = 0; i < F.size(); ++i)
	sum += W[i]*F[i];
      return sum;
    }
  };

  
  class NNetComp : public SSCCompObj
  {
    nnet_interpreter::Graph nn;
  public:
  NNetComp(const std::string &snet) : nn(snet.c_str()) { }
    float eval(const std::vector<float> &F)
    {
      nn.setInputs(F);
      double r = nn.calculate();
      return r;
    }
  };
  
  class DecisionListComp : public SSCCompObj
  {
    int decide(const std::vector<float> &features, std::istream& list);
    
    string postfixDecisionList;
  public:
  DecisionListComp(const std::string &postfixDL) : postfixDecisionList(postfixDL) { }
    float eval(const std::vector<float> &F)
    {
      istringstream sin(postfixDecisionList);
      double r = decide(F,sin) == 1 ? -1.0 : 1.0;
      return r;
    }
  };
  
  
  class FeatureChooser :  public SplitSetChooserConjunction
  {
    SSCCompRef comp;
  public:
    FeatureChooser(TAndRef C, SSCCompRef comp) : SplitSetChooserConjunction(C) { this->comp = comp; }
    IntPolyRef chooseNextPoly(set<IntPolyRef> &S, VarOrderRef X, NodeRef node);
  };
  /******************************************/

 class SplitSetChooserDNF : public SplitSetChooserObj
 {
 public:
   SplitSetChooserDNF(TFormRef F) 
   {
     TOrRef G = asa<TOrObj>(F);
     if (G.is_null()) { throw TarskiException("mkNuCADDNF requires an OR at the top level!"); }    
     for(TOrObj::disjunct_iterator itr = G->begin(); itr != G->end(); ++itr)
     {
       TAndRef a = new TAndObj();
       a->AND(*itr);
       if (!isConjunctionOfAtoms(a))
	 throw TarskiException("mkNuCADDNF requires an OR of conjunctions of atoms");      
       C.push_back(a);
     }
   }
   void chooseSplit(VarOrderRef X, NodeRef node, int dim,
		    set<IntPolyRef> &Q, int &tvAtAlpha, int &targetTruthValue);
 private:
   vector<TAndRef> C;
 };

 
class SearchQueueObj; typedef GC_Hand<SearchQueueObj> SearchQueueRef;
class SearchQueueObj : public GC_Obj
{
  queue<NodeRef> Q;
 public:
  virtual void push(NodeRef n) { Q.push(n); }
  virtual void pop() { Q.pop(); }
  virtual NodeRef front() { return Q.front(); }
  virtual int size() { return Q.size(); }
  virtual bool empty() { return Q.empty(); }
  virtual bool stopSearch() { return empty(); }
};

class EarlyTerminationException : public TarskiException
{
private:
  GCWord alpha;
public:
  EarlyTerminationException(const std::string &msg, Word alpha) throw() : TarskiException(msg) { this->alpha = alpha; }
  virtual ~EarlyTerminationException() throw() { }
  Word getAlpha() { return alpha; }
};

class EarlyTerminateSearchQueueObj; typedef GC_Hand<EarlyTerminateSearchQueueObj> EarlyTerminateSearchQueueObjRef;
class EarlyTerminateSearchQueueObj : public SearchQueueObj
{
  VarOrderRef X;
  TAndRef F;
  GCWord SATAlpha;
 public:
  EarlyTerminateSearchQueueObj(VarOrderRef X, TAndRef F) { this->X = X; this->F = F; SATAlpha = 0; }
  VarOrderRef getX() { return X; }
  TAndRef getF() { return F; }
  void notify(NodeRef n) { if (n->truthValueAtAlpha == TRUE) SATAlpha = n->getData()->getCell()->getAlpha(); }
  virtual void push(NodeRef n) 
  { 
    Word alpha = n->getData()->getCell()->getAlpha();
    if (n->truthValueAtAlpha == UNDET)
      n->truthValueAtAlpha = evalStrictConjunctionAtPoint(X, alpha , F);
    notify(n);    
    SearchQueueObj::push(n);
  }
  virtual bool stopSearch() { return SATAlphaFound() || empty(); }
  virtual bool SATAlphaFound() { return SATAlpha != 0; }
  virtual Word getSATAlpha() { return SATAlpha; }
};

class PriorityAEarlyTerminateSearchQueueObj : public EarlyTerminateSearchQueueObj
{
  class NodePriorityPair
  {
    NodeRef node;
    int priority;
  public:
    NodePriorityPair(NodeRef n, int p) { node = n; priority = p; }
    bool operator<(const NodePriorityPair& b) const { return this-> priority < b.priority; }
    NodeRef getNode() const { return node; }
  };

  priority_queue<NodePriorityPair,std::vector<NodePriorityPair> > PQ;

 public:
  PriorityAEarlyTerminateSearchQueueObj(VarOrderRef X, TAndRef F) : EarlyTerminateSearchQueueObj(X,F) { }
  virtual void push(NodeRef n) 
  {
    int count = countUNSATStrictConjunctionAtPoint(getX(), n->getData()->getCell()->getAlpha(), getF());
    n->truthValueAtAlpha = (count == 0 ? TRUE : FALSE);
    notify(n);    
    PQ.push(NodePriorityPair(n,-count)); 
  } 
  virtual void pop() { PQ.pop(); }
  virtual NodeRef front() { return PQ.top().getNode(); }
  virtual int size() { return PQ.size(); }
  virtual bool empty() { return PQ.empty(); }
};

class ONuCADObj; typedef GC_Hand<ONuCADObj> ONuCADRef;

class ONuCADObj : public GC_Obj
{
 private:
  BuilderRef builder;
  int dim;
  VarOrderRef X;
  GCWord alpha; 
  TAndRef C;

  NodeRef root;
  SearchQueueRef nodeQueue;
  SplitSetChooserRef chooser;

  void init(VarOrderRef X, TAndRef F, int dim, Word alpha, SearchQueueRef nodeQueue, SplitSetChooserRef chooser)
  {
    this->X = X; this->C = F; this->dim = dim; this->alpha = alpha; this->root = NULL;
    this->nodeQueue = nodeQueue;
    this->chooser = chooser;
  }
  
 public:
  void testTree(NodeRef c); // JUST FOR DEBUG
  ONuCADObj(VarOrderRef X, TAndRef F, int dim, Word alpha)
  {
    init(X,F,dim,alpha,new SearchQueueObj(),new SplitSetChooserConjunction(F));
  }  
  // ONuCADObj(VarOrderRef X, TAndRef F, int dim, Word alpha, SplitSetChooserRef chooser)
  // {
  //   init(X,F,dim,alpha,new SearchQueueObj(),chooser);
  // }  
  ONuCADObj(VarOrderRef X, TAndRef F, int dim, Word alpha, SearchQueueRef nodeQueue)
  {
    init(X,F,dim,alpha,nodeQueue,new SplitSetChooserConjunction(F));
  }
  ONuCADObj(VarOrderRef X, TAndRef F, int dim, Word alpha, SearchQueueRef nodeQueue, SplitSetChooserRef chooser)
  {
    init(X,F,dim,alpha,nodeQueue,chooser);
  }
  
 class LeafIterator
 {
   std::stack<NodeRef> S;
  public:
    LeafIterator(NodeRef start) { if (!start.is_null()) S.push(start); }
    bool hasNext() { return !S.empty(); }
    NodeRef next()
    {
      NodeRef n = S.top(); S.pop();
      if (!n->hasChildren()) { return n; }
      for(int i = n->Uchild.size() - 1; i >= 0; i--)
	S.push(n->Uchild[i]);
      S.push(n->XYchild);
      for(int i = n->Lchild.size() - 1; i >= 0; i--)
	S.push(n->Lchild[i]);
      return next();
    }
  };

  LeafIterator iterator() { return LeafIterator(root); }
  LeafIterator iterator(NodeRef sub) { return LeafIterator(sub); }

  void mergeLow(NodeRef P, int k, int truthValue);
  void mergeHigh(NodeRef P, int k, int truthValue);

  // makes propogations of truth value down onto level k in the sub-tree rooted at P
  // NOTE: if strictFlag is true, the NuCAD after projection will have the property that
  //       all points in a cell marked true have a solution point above them from the
  //       "original" NuCAD.  If strictFlag is false, there may be lower dimensional regions
  //       within a true cell over which there is no solution point. However, regardless of
  //       strictFlag's value, it is guaranteed that there is no open std::set in k-space that is
  //       miscategorized.
  //       Example: (def F [ y^2 - x^2 (x + 1) < 0 ])
  //                (def D (make-NuCADConjunction '(x y) '(0 0) F))
  //                (msg D 'project 1)
  //                If strictFlag is false, x=0 will be in a true cell after projection.
  void expropDownTo(NodeRef P, int k, bool strictFlag);

  // TO APPEAR
  void projectionResolve(NodeRef C1, NodeRef C2, int k, int dim);

  void writeDot(const std::string &label, std::ostream& out);
  void writeDot(NodeRef start, std::ostream& out);

  BuilderRef getCell(const std::string &label) { return getNode(label)->data; }
  NodeRef getNode(const std::string &label);
  NodeRef getNode(const char* p);
  NodeRef getRoot() { return root; }
  TAndRef getCurrentFormula() { return C; }
  VarOrderRef getVarOrder() { return X; }
  int getDim() { return dim; }

  void negateTruthValues();


  // A is a point (possibly of lower level than dim), return std::vector of all
  // NodeRef's for cells that contain point A.  NOTE:  if A is of level dim,
  // the result will contain one or zero cells - zero if the point lies on
  // a cell boundary
  // THOUGHT: should I return something to indicate that the point fell on some
  // cell boundaries?
  std::vector<NodeRef> locate(Word A);

  pair<NodeRef,int> locate(NodeRef C, Word beta);



  // make an open-NuCAD in which F (a conjunction of atomic formulas that are sign conditions on irreducible polynomials)
  // is truth invariant.
  // V is std::set to a std::vector of cells
  // L is std::set to a std::vector of labels
  // TV is std::set to a std::vector of truth-values
  void mkNuCADConjunction(VarOrderRef X, TAndRef F, int dim, Word alpha);

  void mkNuCADDNF(VarOrderRef X, TFormRef F, int dim, Word alpha);

  // Does one refinement step on cell given by label.
  void refineNuCADConjunction(const std::string &label);

  // refines subtree rooted at label all the way til truth invariant.
  void refineSubtreeNuCADConjunction(NodeRef node);
  void refineSubtreeNuCADConjunction(const string &label);
  
  // Takes whatever is enqueued and keeps refining until the queue is empty
  void fullRefine();
  
  // Revert's cell given by label to it's original form (e.g. remove children)
  void revertCell(const std::string &label);

  
  // just a helper
  void NuCADSplitConjunction(VarOrderRef X, TAndRef C, int dim, NodeRef node);
  void NuCADSplit(VarOrderRef X, SplitSetChooserRef chooser, int dim, NodeRef node);

  // another helper
  static void dealWithFail(VarOrderRef X, BuilderRef D, BuilderRef DX, int dim);

  // TEMPORARY
  // Report statics on truth-invariant sub-trees
  // For each truth-invariant subtree T such that C is the root of T and the tree rooted
  // at parent(C) is not truth-invariant, print out C's label, truth value, and the number 
  // of nodes in T.
  void truthInvariantSubtreeStats();  

  static const int m_tcells = 1, m_fcells = 2, m_nxcells = 4, m_ntcells = 8, m_nfcells = 16, m_pfsumm = 32, m_acells = 64;
  int nx, nt, nf;
  std::string toString() { return toString(m_acells); }
  std::string toString(int mask);

  TAndRef getUNSATCore();
  //-- This is the HPC learning 2019 trial
  void trial(NodeRef node, vector<vector<float>> &X, vector<vector<float>> &y);
  int getCandidateNodes(NodeRef node, std::vector<pair<int,NodeRef>> &candidates, int leafThreshold, int choicesThreshold);
  
};

class OpenNuCADSATSolverObj; typedef GC_Hand<OpenNuCADSATSolverObj> OpenNuCADSATSolverRef;
class OpenNuCADSATSolverObj : public GC_Obj
{
private:
  VarOrderRef V;
  GCWord A;
  TAndRef C;
  EarlyTerminateSearchQueueObjRef nodeQueue;
  ONuCADRef nucad;
  bool SATFound;
  
public:
  //-- Must call this with F that is a conjunction of atoms or an atom
  OpenNuCADSATSolverObj(TFormRef F)
  {
    // Create VarOrder for this problem
    std::vector<VarSet> X = getBrownVariableOrder(F);
    V = new VarOrderObj(F->getPolyManagerPtr());
    for(int i = 0; i < X.size(); ++i)
      V->push_back(X[i]);

    // choose the origin for "point alpha"
    A = NIL;
    for(int i = 0; i < X.size(); ++i)
      A = COMP(0,A);
    
    // Try making F a conjunction
    C = asa<TAndObj>(F);
    if (C.is_null())
    {
      TAtomRef a = asa<TAtomObj>(F);
      TExtAtomRef b = asa<TExtAtomObj>(F);
      if (a.is_null() && b.is_null()) 
	throw TarskiException("OpenNuCADSATSolver requires a conjunction or atomic formula.");      
      C = new TAndObj();
      C->AND(F);
    }

    // Build the OpenNuCAD up until a SAT point is found (or NuCAD is complete)
    nodeQueue = new PriorityAEarlyTerminateSearchQueueObj(V,C);
    nucad = new ONuCADObj(V,C,V->size(),A,nodeQueue);
    nucad->mkNuCADConjunction(V,C,V->size(),A);
    SATFound = nodeQueue->SATAlphaFound();
  }

  VarOrderRef getVarOrder() { return V; }
  
  bool isSATFound() const { return SATFound; }

  //-- returns VarKeyedMap M such that M[x] is the SACLIB rational number representing the
  //-- value of variable x in the satisfying assignment.  All non-assigned variables are given
  //-- the value zero (as a SACLIB rational number)
  VarKeyedMap<GCWord> getSatisfyingAssignment()
  {
    if (!isSATFound()) { throw TarskiException("getSatisfyingAssignment called on UNSAT OpenNuCAD"); }

    VarKeyedMap<GCWord> res(0);
    Word alpha = nodeQueue->getSATAlpha();
    for(Word P = alpha, level = 1; P != NIL; P = RED(P), ++level)
      res[V->get(level)] = FIRST(P);

    return res;
  }

  //-- returns the UNSAT core as a conjunction of atoms, all from the original input F, that is
  //-- UNSAT.  TODO:  For now this just returns the original formula as the core.  I will work on
  //-- doing better. 
  TAndRef getUNSATCore();


};

}//end namespace tarski

#endif
