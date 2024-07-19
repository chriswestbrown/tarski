#include "writeForQE.h"
#include "formmanip.h"
#include "normalize.h"
#include <stack>
#include <algorithm>

using namespace std;

namespace tarski {
// Factors a conjunction into chunks that are disjoint
// with respect to the given set of variables.
// Returns vector<TFormRef> res with the property that
// 1) each element is an atom or conjunction of atoms
// 2) the conjunction of all elements of res is exactly F
// 3) if i != j, then res[i] and res[j] do not share any variable in S
// The upshot of this is that if Q is a subset of S,
// ex Q [ F ] <==> ex Q[res[0]] /\ ... /\ ex Q[res[k-1]] and
// all Q [ F ] <==> all Q[res[0]] /\ ... /\ all Q[res[k-1]] and
vector<TFormRef> factorDisjointVars(TAndRef F, VarSet S)
{
  vector<TFormRef> res;

  // A[i] is ith conjunct in F
  // Sf[i] is the set of elements of S occuring in A[i]
  vector<TFormRef> A;
  vector<VarSet> Sf;
  for(set<TFormRef>::iterator itr = F->conjuncts.begin(); itr != F->conjuncts.end(); ++itr)
  {
    A.push_back(*itr);
    Sf.push_back(((*itr)->getVars()) & S);
  }

  // Find connected components.
  unsigned int s = 0, e = 0; /// A[s],...,A[e] will grow to include an entire componant
  do {
    unsigned int k = e + 1;
    VarSet curr = Sf[s];
    while(k < A.size())
    {
      if ((Sf[k] & curr).any())
      {
	curr |= Sf[k];
	swap(Sf[e+1],Sf[k]);
	swap(A[e+1],A[k]);
	++e;
	k = e+1;
      }
      else
	++k;
    }
    
    // Set C to the "and" of each conjunct of F in the current componant
    TAndRef C = new TAndObj();
    for(unsigned int i = s; i <= e; ++i)
      C->AND(A[i]);
    res.push_back(C);

    s = e + 1;
    e = s;
  } while(s < A.size());
  return res;  
}

// Returns a set of references to atoms in F that contain
// the unique occurence of an element of S. Requires an AND!!!!
map<TFormRef,VarSet> atomsContainingUniqueVar(TAndRef F, VarSet S)
{
  map<TFormRef,VarSet> res;
  // A[i] is ith conjunct in F
  vector<TFormRef> A;
  for(set<TFormRef>::iterator itr = F->conjuncts.begin(); itr != F->conjuncts.end(); ++itr)
    A.push_back(*itr);

  // Collect info about vars occuring in only one conjunct
  VarKeyedMap<int> M(-1);
  VarSet U; // Used in more than one atom
  VarSet I; // In formula F and set S
  VarSet all; all.flip();
  for(unsigned int i = 0; i < A.size(); ++i)
  {
    VarSet N = ((A[i]->getVars() & S) & (all ^ U));
    if (!N.none())
    {
      I |= N;
      for(VarSet::iterator vitr = N.begin(); vitr != N.end(); ++vitr)
	if (M[*vitr] == -1)
	  M[*vitr] = i;
	else
	{
	  M[*vitr] = -2;
	  U |= *vitr;
	}
    }
  }

  // Construct result to return
  VarSet G = I & U.flip();
  for(VarSet::iterator vitr = G.begin(); vitr != G.end(); ++vitr)
    res[A[M[*vitr]]] |= *vitr;
  return res;
}

  /*
 (suggest-qepcad [ex a,b [ all c,d[ c < _root_1 c^2 - d /\ a b < c + d /\ b < _root_2 b^2 + a b + c]]])
 (suggest-qepcad [ a < _root_1 a^2 - b /\ b < _root_1 b^2 - c d /\ d < _root_1 d^2 - a ])
 (suggest-qepcad [ex a,b,c,d[ a < _root_1 a^2 - b /\ b < _root_1 b^2 - c d /\ d < _root_1 d^2 - a ]])
  */
 /****
  ** class Process provides function DFS that takes a set V of variables
  ** and map E where E[u] is the set of varibles u depends on, and a) determines
  ** whether the associated graph is acyclic, and b) if so does a topological
  ** sort of the vertices.
  ****/
  class Process {
  public:
    const int infty = 1 << 30, unvisited = 0, active = 1, processed = 2;
    typedef pair<VarSet,VarSet::iterator> spair;

    VarKeyedMap<int> mark, starttime, endtime;
    int count;
    bool iscycle;
    vector<VarSet> cycle;
    PolyManager* pPM;
    Process(  PolyManager* pm) : mark(unvisited), starttime(infty), endtime(infty), count(0), iscycle(false), pPM(pm) {  }

    void DFS(VarSet V, VarKeyedMap<VarSet> E) {
      for(VarSet::iterator itr = V.begin(); itr != V.end(); ++itr)
	if (mark[*itr] == unvisited)
	  DFSVisit(V,E,*itr);
    }
  
    // VarSet V are the "vertices" of graph G, E the "edges", s is a single variables - the start "vertex"
    void DFSVisit(VarSet V, VarKeyedMap<VarSet> &E, VarSet s) {    
      stack<spair> S; //init S to empty stack [ count=0; set starttime and endtime for all vertices to +∞]
      mark[s] = active; // mark s active [ set starttime of s to count; count++ ]
      starttime[s] = count++;    
      S.push(spair(s,E[s].begin()));
      
      while(!S.empty()) {
	VarSet u = S.top().first; VarSet::iterator i = S.top().second; S.pop();
	//cerr << "dequeued " << pPM->getName(u) << endl;
	if (i != u.end()) {
	  VarSet v = *i;
	  //cerr << "looking at child " << pPM->getName(v) << endl;
	  S.push(spair(u,++i));
	  if (mark[v] == unvisited) {
	    mark[v] = active;
	    starttime[s] = count++;    
	    S.push(spair(v,E[v].begin()));
	  }
	  else if (mark[v] == active && !iscycle) { // found a cycle!
	    iscycle = true;
	    while(true) {
	      VarSet w = S.top().first; S.pop();
	      cycle.push_back(w);
	      if (w == v) {
		std::reverse(cycle.begin(), cycle.end());
		break;
	      }
	    }
	  }
	}
	else {
	  mark[u] = processed;
	  endtime[u] = count++;
	}
      }
    }
  };

 /**
  ** The application is to determine constraints on variable order implied by
  ** extended tarski atoms and quantifier block structure
  ** NOTE: the initial formulas must be prenex!
  ** NOTE: within the context of this code, "before" means more inner
  **/
class VarOrderConstraints : public TFPolyFun
{
private:
  // block[0] is VarSet for the *inneermost* block of quantified variables.
  vector<VarSet> blocks;

  // maps variable to position in "blocks"
  VarKeyedMap<int> varToBlockIndex;
  VarKeyedMap<VarSet> etrequires; // requires[z] is set of vars z must be innerer than based on extended tarski atoms

  bool violation;
  string violationExplanation;
  PolyManager* pPM;
  int nodeCount, etfAtomCount;
  
public:
  VarOrderConstraints() { violation = false; violationExplanation = "";  pPM = NULL; }
  bool process(TFormRef F, vector<VarSet> &prefOrd) { // returns true if there is a valid order, false otherwise
    
    // Setup and process formula
    nodeCount = 0;
    etfAtomCount = 0;
    pPM = F->getPolyManagerPtr();
    if (isQuantifierFree(F))
      blocks.push_back(F->getVars()); // handles quantifier-free!
    actOn(F);
    
    if (etfAtomCount == 0) return true;
    if (violation) return false;
    vector<VarSet> ord;
    for(int k = blocks.size()-1; k >= 0; --k) {
      Process P(pPM);
      P.DFS(blocks[k],etrequires);
      if (P.iscycle) {
	//cerr << "Cycle! ";
	//for(int i = 0; i < P.cycle.size(); i++)
	//  cerr << " " << pPM->getName(P.cycle[i]);
	//cerr << endl;
	violation = true;
	ostringstream sout;
	sout << "Formula is not proper with respect to any variable order! Constraints imposed by _root_ expressions form a cycle for variables:";
	for(int i = 0; i < P.cycle.size(); i++)
	  sout << " " << pPM->getName(P.cycle[i]);
	sout << ".";
	violationExplanation = sout.str();	
	return false;
      }
      else {
	//cerr << "No Cycle!" << endl;
	int N = P.count;
	vector<VarSet> tmp(N), bord;
	for(int i = 0; i < N; i++)
	  tmp[i] = 0;
	VarSet V = blocks[k];
	for(VarSet::iterator itr = V.begin(); itr != V.end(); ++itr)
	  tmp[P.endtime[*itr]] = *itr;
	for(int i = 0; i < tmp.size(); i++)
	  if (tmp[i] != 0)
	    bord.push_back(tmp[i]);
	orderWithPrefs(bord,prefOrd,V);
	for(int i = 0; i < bord.size(); i++)
	  ord.push_back(bord[i]);
      }
    }

    //cerr << "Valid order:";
    //for(int i = 0; i < ord.size(); i++)
    //  cerr << " " << pPM->getName(ord[i]);
    //cerr << endl;
    swap(ord,prefOrd);
    return true;
  }

    void orderWithPrefs(vector<VarSet> &bord, vector<VarSet> &prefOrd, VarSet V) {
      // prefPos
      VarKeyedMap<int> prefPos(-1);
      for(int i = 0; i < prefOrd.size(); i++)
	prefPos[prefOrd[i]] = i;

      // improve order until no improving swap exists
      int i = 0;
      while(i < bord.size() - 1) {
	VarSet u = bord[i], v = bord[i+1];
	if (prefPos[u] > prefPos[v] && (etrequires[v] & u).isEmpty()) {
	  swap(bord[i],bord[i+1]);
	  i = (i == 0 ? 0 : i-1);
	}
	else
	  i++;
      }
  }
    
  string explanation() { return violationExplanation; }
  void action(TConstObj* p) { nodeCount++; }
  void action(TAtomObj* p) { nodeCount++; }
  void action(TExtAtomObj* p) {
    etfAtomCount++;
    nodeCount++; 
    VarSet z = p->getLHSVar();
    VarSet X = p->getVars() - z;
    for(VarSet::iterator itr = X.begin(); itr != X.end(); ++itr) {
      if (*itr == z) continue;
      if (!violation && varToBlockIndex[*itr] < varToBlockIndex[z]) {
	violation = true;
	violationExplanation = "Variable " + p->getPolyManagerPtr()->getName(z) + " depends on "
	  + p->getPolyManagerPtr()->getName(*itr) + " because of extended atom " + toString(TFormRef(p))
	  + ", which conflicts with the quantifier block structure.";
      }
      etrequires[z] = etrequires[z] | *itr;
    }
  }
  void action(TAndObj* p) {
    if (p->size() > 1) nodeCount++; 
    for(set<TFormRef>::iterator itr = p->conjuncts.begin(); itr != p->conjuncts.end(); ++itr) {
      actOn(*itr);
    }
  }
  void action(TOrObj* p)
  {
    if (p->size() > 1) nodeCount++; 
    for(set<TFormRef>::iterator itr = p->disjuncts.begin(); itr != p->disjuncts.end(); ++itr) {
      actOn(*itr);
    }
  }

  void action(TQBObj* p) {
    if (nodeCount != 0) {
      throw TarskiException("VarOrderConstraints in writeForQE requires a prenex formula!");
    }
    nodeCount++;
    
    // create "blocks" vector (see top of class doc)
    VarSet Vfree = p->getVars();
    for(int i = 0; i < p->numBlocks(); i++) {
      VarSet Vb = p->blockVars(i);
      blocks.push_back(Vb);
      Vfree = Vfree - Vb;
    }
    if (!Vfree.isEmpty())
      blocks.push_back(Vfree);
    
    // map s.t. for var v, varToBlockIndex[v] is the index i s.t. v is in blocks[i]
    for(unsigned int i = 0; i < blocks.size(); i++)
      for(VarSet::iterator itr = blocks[i].begin(); itr != blocks[i].end(); ++itr)
	varToBlockIndex[*itr] = i;
    
    actOn(p->formulaPart);
  }
};

  void makeOrderProper(TFormRef F, vector<VarSet> &ord) {
    VarOrderConstraints VOC;
    if (!VOC.process(F,ord)) {
      throw TarskiException(VOC.explanation());
    }
  }
  

/***************************************************************
 *
 ***************************************************************/
string naiveButCompleteWriteForQepcad(TFormRef Fin,
				      TFormRef &introducedAssumptions,
				      bool endWithQuit,
				      bool trackUnsatCore,
				      char solFormType,
				      VarOrderRef ord
				      )
{
  try { return writeForQEPCADB(Fin,introducedAssumptions,endWithQuit,trackUnsatCore,
			       solFormType,ord); }
  catch(TarskiException e) { }

  // Set F to a prenex equivalent to Fin
  TFormRef F = Fin;
  if (!isPrenex(Fin)) F =  makePrenex(Fin);

  // determine which variables from F appear in non-strict inequalities (i.e. =,<=,>=)
  VarSet Vns = appearsInNonStrict(F);
  
  // Set Fqff to the quantifier-free part of F, 
  // Vall to the set of all variables appearing in F (both bound and quantified),
  // and blocks to the vector s.t. blocks[i] is the set of variables from the ith
  // quantifier-block (0 being innermost) and the last element of blocks being the
  // set of free variables.
  TFormRef Fqff;
  VarSet Vall = F->getVars(), Vq;
  vector<VarSet> blocks;
  TQBRef Q = dynamic_cast<TQBObj*>(&*F);
  if (!Q.is_null())
  {
    Fqff = Q->getFormulaPart();
    for(int i = 0; i < Q->numBlocks(); i++)
    {
      VarSet Vnext = Q->blockVars(i);
      blocks.push_back(Vnext);
      Vq = Vq + Vnext;
    } 
  }
  else
    Fqff = F;
  blocks.push_back(Vall - Vq);

  // If Fqff has no variables, we need to handle this separately.
  if (Fqff->getVars().isEmpty())
  {
    Level1 normalizer;
    RawNormalizer R(normalizer);
    R(F);
    int t = R.getRes()->constValue();
    if (t == TRUE) { return "true"; }
    else if (t == FALSE) { return "false"; }
    else throw TarskiException("Error in naiveButCompleteWriteForQepcad!");
  }

  // map s.t. for var v, varToBlockIndex[v] is the index i s.t. v is in blocks[i]
  VarKeyedMap<int> varToBlockIndex;
  for(unsigned int i = 0; i < blocks.size(); i++)
    for(VarSet::iterator itr = blocks[i].begin(); itr != blocks[i].end(); ++itr)
      varToBlockIndex[*itr] = i;
  
  // set blockOrder[i] to the vector of variable from the ith block in brown heuristic order
  vector<VarSet> V = getBrownVariableOrder(Fqff);
  if (!ord.is_null()) // override with manually specified order
  {
    ord->sort(V);
    std::reverse(V.begin(),V.end());
  }
  
  // Figure out constraints inherent in quantifier structure and indexed root expressions
  // and re-order V to account for it if possible
  makeOrderProper(F,V);
  
  vector< vector<VarSet> > blockOrders(blocks.size());
  for(unsigned int i = 0; i < V.size(); ++i)
    blockOrders[varToBlockIndex[V[i]]].push_back(V[i]);
  
  PolyManager &PM = *(F->getPolyManagerPtr());
  // cout << "preferred variable order: ";
  // for(int i = 0; i < V.size(); ++i)
  //   cout << PM.getName(V[i]) << ' ';
  // cout << endl;

  // cout << "adapted to quantifier matrix: ";
  // for(int k = blockOrders.size() - 1; k >= 0; --k)
  // {
  //   for(int j = 0; j < blockOrders[k].size(); ++j)
  //     cout << PM.getName(blockOrders[k][j]) << ' ';
  // }
  // cout << endl;

  ostringstream sout;
  sout << "[]\n(";
  for(int k = blockOrders.size() - 1, count = 0; k >= 0; --k)
  {
    for(unsigned int j = 0; j < blockOrders[k].size(); ++j)
      sout << (count++ > 0 ? "," : "") << PM.getName(blockOrders[k][j]);
  }
  sout << ")" << endl;
  sout << (Vall.numElements() - Vq.numElements()) << endl;

  // Set countNonStrict to the number of quantified variables *in the current order* that
  // must be treated as non-strict.  This means that the the first countNonStrict variables
  // must be treated as non-strict, but the remaining variables can be treated as strict.
  bool allStrictSoFar = true;
  int countNonStrict = 0;
  for(int k = 0; k < blockOrders.size() - 1; ++k)
    for(int j = blockOrders[k].size() - 1; j >= 0; --j)
    {
      VarSet x = blockOrders[k][j];
      allStrictSoFar = allStrictSoFar && !(Vns & x).any();
      if (!allStrictSoFar)
	countNonStrict++;
    }

  // Write the quantifier block
  //-- NOTE: I NEED TO FIX THIS!  1. universal requires =/<=/>= only, 2. what the heck
  //         happens if there are alternations!  I think it all breaks down!
  for(int k = blockOrders.size() - 2; k >= 0; --k)
  {
    for(unsigned int j = 0; j < blockOrders[k].size(); ++j, --countNonStrict)
    {
      sout << "(" << (Q->blockType(k) == EXIST
		      ? (countNonStrict > 0 ? "E" : "F")
		      : (countNonStrict > 0 ? "A" : "A") 
		      ) << " " << PM.getName(blockOrders[k][j]) << ")";
    }
  }
  sout << "[";
  PushOutputContext(sout);
  Fqff->write();
  PopOutputContext();
  sout << "].\n";
  //  sout << "cell-choice-bound (SR,LD,HL)\n"; // T E S T ! ! !
  sout << "go\ngo\ngo\n";
  if (solFormType != 0)
    sout << "sol " << solFormType << "\n";


  if (endWithQuit) { sout << "quit" << endl; }
  return sout.str();
}
}//end namespace tarski
