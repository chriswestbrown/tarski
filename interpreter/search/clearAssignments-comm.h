#ifndef _CLEAR_ASSIGNENTS_COMM_H_
#define _CLEAR_ASSIGNENTS_COMM_H_

#include "../shell/einterpreter.h"
#include "../poly/variable.h"
#include "../poly/poly.h"
#include "../formula/formula.h"
#include <map>
#include <vector>

namespace tarski {

/*******************************************************************
 ** Clear Assignments:  In a conjunction, what we might mean by 
 ** "solving" can be a little ambiguous, and what moves we want to
 ** make to work towards a solution may involve many decisions. 
 ** However, one kind of move that requires no decisions is the move
 ** to clear "assignments".  Here we consider only assignments of
 ** the form a x = b and a x = b y.  These are assignments of constants
 ** to variables or of one variable to a constant multiple of another.
 ** 
 ** Clear-assignments efficiently detects and makes such assignments.
 ** We note, however, that making the assignments may create new
 ** opportunities to make more assignments ... though to take advantage
 ** of them, clear-assignments would have to be called again.
 ** 
 ** This command works by creating a graph of such assignments (see
 ** comments in the code) and finding connected components in that
 ** graph, each of which represents a std::set of variables that are all
 ** constant multiples of one another (or are assigned constant values).
 *******************************************************************/



// makeAssignments is defined below.
TFormRef makeAssignments(TFormRef F,
			 VarKeyedMap<GCWord> &constants, 
			 const pair<GCWord,VarSet> &nada, 
			 VarKeyedMap< pair<GCWord,VarSet> > &multiples);


class ClearAssignmentsComm : public EICommand
{
private:
  class MarkLog
  {
    std::vector<int> pos, neg;
  public:
    MarkLog(int N, int initial) : pos(N + 1,initial), neg(N + 1,initial) { }
    int setMark(int i, int val) // returns previous mark value
    {
      if (i < 0) { int t = neg[-i]; neg[-i] = val; return t; }
      else { int t = pos[i]; pos[i] = val; return t; }
    }
    int getMark(int i) { return i < 0 ? neg[-i] : pos[i]; }
  };

  class Graph
  {
    // The vertices of this graph are 1,2,..,n - which correspond to variables via the vertices std::vector
    // For vertex i, if hasShadowVertex[i] == 1, then -i is also a vertex in the graph
    // If (i,j) is an edge with weight (a,b), then (j,i) is an edge with weight (b,a).
    // the meaning of (i,j,(a,b)) where i,j > 0 is that a x_i + b x_j = 0 is an atom of conjunction F
    // the meaning of (i,-i,(a,b)) where i,i > 0 is that a x_i + b = 0 is an atom of conjunction F
    // there are no other kinds of edges involving negative vertices.
    std::vector<VarSet> vertices;
    std::vector<int> hasShadowVertex;
    VarKeyedMap<int> var2vert;
    std::vector< std::vector<int> > Nbrs, ShadowNbrs;
    std::map< pair<int,int> , GCWord> edgeWeight;
    std::map<int,GCWord> value;
  public:
    int maxVertexIndex() { return vertices.size() - 1; }
    Word getValue(int i) { if (value.find(i) == value.end()) return NIL; else return value[i]; }
    void setValue(int i, Word val) { value[i] = val; }
    Graph() : var2vert(-1) 
    { 
      VarSet v; 
      vertices.push_back(v); 
      hasShadowVertex.push_back(0); 
      Nbrs.push_back(std::vector<int>()); 
      ShadowNbrs.push_back(std::vector<int>()); 
      ShadowNbrs.push_back(std::vector<int>()); 
    }

    Word getEdgeWeight(int i, int j) { return edgeWeight[ pair<int,int>(i,j) ]; }

    std::vector<int>::iterator nbr_begin(int v) { return v > 0 ? Nbrs[v].begin() : ShadowNbrs[-v].begin(); }
    std::vector<int>::iterator nbr_end(int v) { return v > 0 ? Nbrs[v].end() : ShadowNbrs[-v].end(); }

    void dump(PolyManager* PMptr)
    {
      int N = vertices.size()-1;
      for(int i = -N; i < 0;  ++i)
	if (hasShadowVertex[-i])
	  std::cout << i << "(-" << PMptr->getName(vertices[-i]) << ")" 
	       << ": " << -i << "(" << PMptr->getName(vertices[-i]) << ")"  << std::endl;
      for(int i = 1; i <= N; ++i)
      {
	std::cout << i << "(" << PMptr->getName(vertices[i]) << ")" 
	     << ":";
	
	for(int j = 0; j < Nbrs[i].size(); ++j)
	  std::cout << " " << Nbrs[i][j]
	       << "(" << PMptr->getName(vertices[abs(Nbrs[i][j])]) << ")";
	std::cout << std::endl;
      }
    }

    VarSet getVarFromVertex(int i) { return vertices[abs(i)]; }

    int getVertex(VarSet x)
    {
      if (var2vert[x] == -1) { 
	int i = vertices.size(); 
	vertices.push_back(x); 
	hasShadowVertex.push_back(0);
	Nbrs.push_back(std::vector<int>());
	ShadowNbrs.push_back(std::vector<int>());
	var2vert[x] = i;
	return i; }
      return var2vert[x];
    }
    void addEdge(VarSet x, Word a, Word b) // a x + b = 0
    {
      int i = getVertex(x);
      hasShadowVertex[i] = 1;
      Nbrs[i].push_back(-i);
      ShadowNbrs[i].push_back(i);
      edgeWeight[ pair<int,int>(i,-i) ] = LIST2(a,b);
      edgeWeight[ pair<int,int>(-i,i) ] = LIST2(b,a);
    }
    void addEdge(VarSet x, VarSet y, Word a, Word b) // a x + b y = 0
    {
      int i = getVertex(x), j = getVertex(y);
      std::map< pair<int,int> , GCWord>::iterator itr = edgeWeight.find(pair<int,int>(i,j));
      if (itr != edgeWeight.end())
      {
	Word Lp = itr->second, ap, bp;
	FIRST2(Lp,&ap,&bp);
	if (ICOMP(IPROD(a,bp),IPROD(ap,b)) != 0)
	{ // in this case, we can only satisyfy a x + b y = 0 = ap x + bp y if x = y = 0
	  addEdge(x,1,0); 
	  addEdge(y,1,0); 
	}
	return;
      }
      Nbrs[i].push_back(j);
      Nbrs[j].push_back(i);
      edgeWeight[ pair<int,int>(i,j) ] = LIST2(a,b);
      edgeWeight[ pair<int,int>(j,i) ] = LIST2(b,a);
    }
    
    std::vector<int> vertexList()
    {
      std::vector<int> V;
      int N = vertices.size()-1;
      for(int i = -N; i < 0; ++i)
	if (hasShadowVertex[-i])
	  V.push_back(i);
      for(int i = 1; i <= N; ++i)
	V.push_back(i);
      return V;
    }
  };
  

public:
  ClearAssignmentsComm(NewEInterpreter* ptr) : EICommand(ptr) { }


  SRef execute(SRef input, std::vector<SRef> &args)
  {
    TarRef T = args[0]->tar();
    TFormRef TF = T->val;
    TAndRef F = asa<TAndObj>(TF);
    if (F.is_null()) { 
      return new ErrObj("Command clear-assignments requires a pure conjunction!"); }
    PolyManager* PMptr = F->getPolyManagerPtr();

    /*************************************************************
     ** Step 1: Create "the graph" as described above
     *************************************************************/
    Graph G;
    for(TAndObj::conjunct_iterator itr = F->begin(); itr != F->end(); ++itr)
    {
      TAtomRef A = asa<TAtomObj>(*itr);
      if (A.is_null())
	return new ErrObj("Command clear-assignments requires a pure conjunction!");
      
      if (A->getRelop() != EQOP || A->F->numFactors() != 1)
	continue;

      IntPolyRef p = A->factorsBegin()->first;
      if (verbose) { std::cout << "p = "; p->write(*PMptr); 
	std::cout << " :: "; OWRITE(p->sP);
	std::cout << std::endl; }
      switch(p->numVars())
      {
      case 1: {
	VarSet x = p->getVars();
	if (p->degree(x) != 1) continue;
	Word P = p->getSaclibPoly();
	Word d, a, Pp, b;
	ADV2(P,&d,&a,&Pp);
	if (Pp == NIL) 
	{ 
	  G.addEdge(x,1,0);
	  if (verbose) { std::cout << PMptr->getName(x) << " = 0" << std::endl; }
	  continue; 
	}
	FIRST2(Pp,&d,&b);
	G.addEdge(x,a,b);
	if (verbose) { IWRITE(a); std::cout << " " << PMptr->getName(x) << " + "; IWRITE(b); std::cout << " = 0" << std::endl; }
      } break;      
      case 2: {
	VarSet V = p->getVars();
	VarSet::iterator vitr = V.begin();
	VarSet y = *vitr;
	VarSet x = *(++vitr);
	Word P = p->getSaclibPoly();
	if (PDEG(P) != 1 || PRED(P) == 0) continue;
	Word d1, c1, d0, c0;
	FIRST4(P,&d1,&c1,&d0,&c0);
	if (PDEG(c1) != 0 || PDEG(c0) != 1 || PRED(c0) != 0) continue;
	Word a = PLDCF(c1), b = PLDCF(c0);
	G.addEdge(x,y,a,b);
	if (verbose) {
	  IWRITE(a); std::cout << " " << PMptr->getName(x) << " + "; 
	  IWRITE(b); std::cout << PMptr->getName(y) << " = 0" << std::endl; }
      } break;
      default: break;
      }
    }
    if (verbose) { G.dump(PMptr); }


    /*************************************************************
     ** Step 2: decompose G into connected components.  Each vertex
     **         in the connected componenet will be described as a
     **         multiple of the component's root vertex.
     *************************************************************/
    
    MarkLog rootFor(G.maxVertexIndex(),0); // rootFor[i] = j means that vertex j was the root 
                                           // of the search for vertex i's connected component.
                                           // rootFor[i] = 0 means vertex i has not been found.

    std::vector<int> V = G.vertexList(); 

    // std::set all shadowVertex values to 1
    for(int i = 0; i < V.size() && V[i] < 0; ++i) 
      G.setValue(V[i],RNINT(1));

    int nexti = 0;
    while(nexti < V.size())
    {
      int startVertex = V[nexti++];
      if (rootFor.getMark(startVertex) != 0) continue;
      rootFor.setMark(startVertex,startVertex);
      G.setValue(startVertex,RNINT(1));
      queue<int> Q;
      Q.push(startVertex);
      
      while(!Q.empty())
      {
	int i = Q.front(); Q.pop();
	for(std::vector<int>::iterator itr = G.nbr_begin(i); itr != G.nbr_end(i); ++itr)
	{
	  int j = *itr;
	  Word L = G.getEdgeWeight(i,j), a, b;
	  FIRST2(L,&a,&b); // a xi + b xj = 0
	  Word vi = G.getValue(i), vj = G.getValue(j);
	  
	  if (vj == NIL) 
	  { // at this point xj can't be a shadowVertex (because it has no value) so b can't be 0  
	    Word newVal = RNPROD(RNNEG(RNRED(a,b)),vi);
	    G.setValue(j,newVal);
	    rootFor.setMark(j,startVertex);
	    Q.push(j); 
	  }
	  else
	  {
	    if (b == 0 && vi != 0) { throw TarskiException("Contradiction!"); }
	    else if (b != 0)
	    {
	      Word newVal = RNPROD(RNNEG(RNRED(a,b)),vi);
	      if (verbose) { std::cout << i << ' ' << j << ' '; RNWRITE(vi); std::cout << ' '; RNWRITE(vj); 
		std::cout << " "; RNWRITE(newVal); std::cout << std::endl; }
	      if (!EQUAL(newVal,vj)) 
	      { 
		if (startVertex < 0)
		  throw TarskiException("Contradiction (constant)!"); 
		else
		  G.setValue(startVertex,0);
	      }
	    }
	  }
	}
      }      
    }

    /*************************************************************
     ** Step 3: Actually make the assignements.  This means prep-
     **         the structures that allow makeAssignements to be
     **         called.
     *************************************************************/
    VarKeyedMap<GCWord> constants(NIL);
    pair<GCWord,VarSet> nada(0,0);
    VarKeyedMap< pair<GCWord,VarSet> > multiples(nada);
    for(int i = 0; i < V.size(); ++i) 
    {
      if (V[i] < 0) continue;
      Word val = G.getValue(V[i]);
      if (val == 0 || rootFor.getMark(V[i]) != V[i])
      { 
	VarSet x = G.getVarFromVertex(V[i]);
	if (verbose) { std::cout << PMptr->getName(x) << " = ";  }
	int r = rootFor.getMark(V[i]);
	if (r < 0) { if (verbose){RNWRITE(val);} constants[x] = val;}
	else if (G.getValue(r) == 0) { if (verbose){RNWRITE(0);} constants[x] = 0; }
	else { 
	  if (verbose){RNWRITE(val); std::cout << " " << PMptr->getName(G.getVarFromVertex(r));}
	  multiples[x] =  pair<GCWord,VarSet>(val,G.getVarFromVertex(r));
	}
	if (verbose){std::cout << std::endl;}
      }
    }    
    TFormRef Fnew = makeAssignments(F,constants,nada,multiples);
    
    return new TarObj(Fnew);
  }


  std::string testArgs(std::vector<SRef> &args) { return require(args,_tar); }
  std::string doc() { return "This command detects assignments in the input formula, and simplifies the formula by making those assignements.  Thus, \n\n   (clear-assignments [ x = y /\\ t = 3 /\\ x + y + t < 1]) \n\ngives y + 1 < 0 as a result.  Note that no attempt is made to detect are take advantage of new assignments that are created by these substitutions.  That would require calling clear-assignments a second time."; }
  std::string usage() { return "(clear-assignments <tar>)"; }
  std::string name() { return "clear-assignments"; }
};

class MakeAssignments : public TFPolyFun
{
private:
  /* class Multiple */
  /* { */
  /* public: */
  /*   VarSet x, y; // represents assignment x <--- q y */
  /*   GCWord q; */
  /*   Multiple(VarSet x, VarSet y, Word q) { this->x = x; this->y = y; this->q = q; } */
  /* }; */

  TFormRef res;
  VarKeyedMap<GCWord> *constants;
  const pair<GCWord,VarSet> *nada;
  VarKeyedMap< pair<GCWord,VarSet> > *multiples;
public:
  MakeAssignments(VarKeyedMap<GCWord> &constants, 
		  const pair<GCWord,VarSet> &nada, 
		  VarKeyedMap< pair<GCWord,VarSet> > &multiples) 
  {
    this->constants = &constants;
    this->nada = &nada;
    this->multiples = &multiples;
  }
  TFormRef getRes() { return res; }
  void action(TConstObj* p) { res = p; }
  void action(TAtomObj* p) 
  { 
    FactRef F = new FactObj(*(p->getPolyManagerPtr()),p->F->content); 
    for(std::map<IntPolyRef,int>::iterator itr = p->factorsBegin(); itr != p->factorsEnd(); ++itr)
    {
      //SWRITE("\nEvaluating factor:\n");
      //itr->first->write(*(p->getPolyManagerPtr())); SWRITE("\n");
      GCWord content;
      IntPolyRef A = itr->first->evalAtRationalPointMakePrim(*constants,content);
      //A->write(*(p->getPolyManagerPtr())); SWRITE(" content = "); RNWRITE(content); SWRITE("\n");
      VarSet V = A->getVars();
      for(VarSet::iterator vitr = V.begin(); vitr != V.end(); ++vitr) 
      {
	pair<GCWord,VarSet> curr = (*multiples)[*vitr];
	if (curr.first != nada->first || curr.second != nada->second)
	{
	  GCWord moreContent;
	  A = p->getPolyManagerPtr()->evalAtRationalMultipleOfVariable(A,*vitr,curr.first,curr.second,moreContent);
	  content = RNPROD(content,moreContent);
	}
      }

      F->addMultiple(A,itr->second);	
      if (RNSIGN(content) < 0 && itr->second % 2 == 1) { F->negateContent(); }
    }

    int relop = p->relop;
    TAtomRef A = new TAtomObj(F,relop);

    // constant LHS case
    if (A->F->isZero())
    { res = new  TConstObj(relop == EQOP || relop == LEOP || relop == GEOP); return; }
    if (A->F->isConstant())
    {
      int s = F->signOfContent();
      int truth = signSatSigma(s,relop);
      res = new TConstObj(truth); return; 
    }
    res = A;
  }
  void action(TAndObj* p) 
  { 
    TAndRef R = new TAndObj();
    for(TAndObj::conjunct_iterator itr = p->begin(); itr != p->end(); ++itr)
    {
      this->actOn(*itr);
      TFormRef a = res;
      if (constValue(a) == FALSE) { res = a; return; }
      R->AND(this->res);
    }
    res = R;
  }
};


TFormRef makeAssignments(TFormRef F,
			 VarKeyedMap<GCWord> &constants, 
			 const pair<GCWord,VarSet> &nada, 
			 VarKeyedMap< pair<GCWord,VarSet> > &multiples)
{
  MakeAssignments MA(constants,nada,multiples);
  MA.actOn(F);
  return MA.getRes();
}

}//end namespace tarski

#endif
