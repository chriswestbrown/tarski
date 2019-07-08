#include "clearAssignments-comm.h"

namespace tarski {

  TFormRef makeAssignments(TFormRef F,
                           VarKeyedMap<GCWord> &constants, 
                           const pair<GCWord,VarSet> &nada, 
                           VarKeyedMap< pair<GCWord,VarSet> > &multiples) {
    MakeAssignments MA(constants,nada,multiples);
    MA.actOn(F);
    return MA.getRes();
  }

  SRef ClearAssignmentsComm::execute(SRef input, std::vector<SRef> &args)
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
    
      MarkLog rootFor(G.maxVertexIndex(),0);
      // rootFor[i] = j means that vertex j was the root
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
	    { // xj not yet assigned a value
	      // NOTE: at this point xj can't be a shadowVertex (because it has no value) so b can't be 0  
	      Word newVal = RNPROD(RNNEG(RNRED(a,b)),vi);
	      G.setValue(j,newVal);
	      rootFor.setMark(j,startVertex);
	      Q.push(j); 
	    }
	    else
	    { // at this point, xj already has a value.  If the new value derived for xj conflicts
	      // with the old value vj, then the only way we might not get a conflict is if the
	      // start vertex is not a shadow vertex, and we set its value to zero.  So we try it.
	      // Otherwise we ignore the conflict, since it will come out when the substitutions
	      // happen.

	      // if xj is not a shadow vertex and startVertex is not a shadow vertex and
	      //    xj already has a value vj s.t. vj and newVal disagree, then set startVertex
	      //    value to 0.  Else do nothing!
	      Word newVal = b == 0 ? 0 : RNPROD(RNNEG(RNRED(a,b)),vi);
	      if (j > 0 && startVertex > 0 && vj != NIL && !EQUAL(newVal,vj))
	      {
		G.setValue(startVertex,0);
	      }
	    }
	  }      
	}
      }

      /*************************************************************
       ** Step 3: Actually make the assignments.  This means prep-
       **         the structures that allow makeAssignments to be
       **         called.
       *************************************************************/
      VarKeyedMap<GCWord> constants(NIL);
      pair<GCWord,VarSet> nada(0,0);
      tarski::VarKeyedMap< pair<GCWord,VarSet> > multiples(nada);
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
	    if (verbose){
	      if (RNCOMP(val,RNINT(-1)) == 0)       { SWRITE("- "); }
	      else if (RNCOMP(val,RNINT(1)) != 0)   { RNWRITE(val); SWRITE(" "); }
	      std::cout << PMptr->getName(G.getVarFromVertex(r));
	    }
	    multiples[x] =  pair<GCWord,VarSet>(val,G.getVarFromVertex(r));
	  }
	  if (verbose){std::cout << std::endl;}
	}
      }    
      TFormRef Fnew = makeAssignments(F,constants,nada,multiples);
    
      return new TarObj(Fnew);
    }

  SRef ClearExpComm::execute(SRef input, std::vector<SRef>& args) {
    TarRef T = args[0]->tar();
    TFormRef TF = T->val;
    TAndRef F = asa<TAndObj>(TF);
    if (F.is_null()) {
      return new ErrObj("Requires pure conjunct");
    }
    SolverManager s( SolverManager::SS, F);
    LisRef l = s.genLisResult();
    return l;
  }


}//end namespace tarski
