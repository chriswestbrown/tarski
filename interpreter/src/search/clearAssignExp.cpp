#include "clearAssignExp.h"

namespace tarski {

  SpanTree::nada(0, 0);


  void ExpGraph::dump(PolyManager* PMptr) {
    int N = maxVertexIndex();
    for(int i = 0; i < N;  i++) {
      writeShadow(i, PMptr);
      if (hasShadow(i)) {
        std::cout << "From: ";
        shadowAtoms[i]->write();
        std::cout << std::endl;
      }
    }
    for(int i = 1; i <= N; ++i) {
      for (int j = 0; j <= N; ++j) {
        writeEdge(i, j, PMptr);
        std::cout << " from ";
        edgeAtoms[i][j]->write();
      }
    }
  }

  void ExpGraph::addEdge(VarSet x, Word a, Word b, TAtomRef t) { //a x + b  = 0
    Graph::addEdge(x, a, b);
    int i = getVertex(x);
    if (i >= shadowAtoms.size()) shadowAtoms.resize(i+1, NULL);
    shadowAtoms[i] = t;
  }


  void ExpGraph::addEdge(VarSet x, VarSet y, Word a, Word b, TAtomRef t) { // a x + b y = 0 
    Graph::addEdge(x, y, a, b);
    int i = getVertex(x);
    int j = getVertex(y);
    if (!Graph::checkValid(i, j, a, b)) {
      addEdge(x, 1, 0, t);
      addEdge(y, 1, 0, t);
    }
    else {
      if (i >= edgeAtoms.size()) edgeAtoms.resize(i+1);
      if (j >= edgeAtoms[i].size()) edgeAtoms[i].resize(j+1, NULL);
      edgeAtoms[i][j] = t;
      if (j >= edgeAtoms.size()) edgeAtoms.resize(j+1);
      if (i >= edgeAtoms[j].size()) edgeAtoms[j].resize(i+1, NULL);
      edgeAtoms[j][i] = t;
    }
  }


  ExpGraph ExpGraph::genExpGraph(TAndRef F) {
    ExpGraph G;
    PolyManager* PMptr = F->getPolyManagerPtr();
    for(TAndObj::conjunct_iterator itr = F->begin(); itr != F->end(); ++itr)
      {
        TAtomRef A = asa<TAtomObj>(*itr);
        if (A.is_null())
          throw TarskiException("Exp-Graph requires a pure conjunction!");

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
                G.addEdge(x,1,0, A);
                if (verbose) { std::cout << PMptr->getName(x) << " = 0" << std::endl; }
                continue; 
              }
            FIRST2(Pp,&d,&b);
            G.addEdge(x,a,b, A);
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
            G.addEdge(x,y,a,b, A);
            if (verbose) {
              IWRITE(a); std::cout << " " << PMptr->getName(x) << " + "; 
              IWRITE(b); std::cout << PMptr->getName(y) << " = 0" << std::endl; }
          } break;
          default: break;
          }
      }
    if (verbose) { G.dump(PMptr); }
    return G;
  }


  vector <Deduction *> ExpGraph::genDeductions() {
    MarkLog rootFor(Graph::maxVertexIndex(),0);
    // rootFor[i] = j means that vertex j was the root
    // of the search for vertex i's connected component.
    // rootFor[i] = 0 means vertex i has not been found.

    vector<Deduction *> deductions;

    std::vector<int> V = Graph::vertexList(); 


    // std::set all shadowVertex values to 1
    for(int i = 0; i < V.size() && V[i] < 0; ++i) 
      Graph::setValue(V[i],RNINT(1));

    int nexti = 0;
    while(nexti < V.size())
      {
        int startVertex = V[nexti++];
        if (rootFor.getMark(startVertex) != 0) continue;
        rootFor.setMark(startVertex,startVertex);
        Graph::setValue(startVertex,RNINT(1));
        queue<int> Q;
        Q.push(startVertex);
      
        while(!Q.empty())
          {
            int i = Q.front(); Q.pop();
            for(std::vector<int>::iterator itr = Graph::nbr_begin(i); itr != Graph::nbr_end(i); ++itr)
              {
                int j = *itr;
                Word L = Graph::getEdgeWeight(i,j), a, b;
                FIRST2(L,&a,&b); // a xi + b xj = 0
                Word vi = Graph::getValue(i), vj = Graph::getValue(j);
	  
                if (vj == NIL) 
                  { // at this point xj can't be a shadowVertex (because it has no value) so b can't be 0  
                    Word newVal = RNPROD(RNNEG(RNRED(a,b)),vi);
                    Graph::setValue(j,newVal);
                    rootFor.setMark(j,startVertex);
                    Q.push(j); 
                  }
                else
                  {
                    if (b == 0 && vi != 0) { throw TarskiException("Contradiction!"); }
                    else if (b != 0)
                      {
                        Word newVal = RNPROD(RNNEG(RNRED(a,b)),vi);
                        if (verbose) {
                          std::cout << i << ' ' << j << ' ';
                          RNWRITE(vi); std::cout << ' '; RNWRITE(vj);
                          std::cout << " "; RNWRITE(newVal); std::cout << std::endl;
                        }
                        if (!EQUAL(newVal,vj)) 
                          { 
                            if (startVertex < 0)
                              throw TarskiException("Contradiction (constant)!"); 
                            else
                              Graph::setValue(startVertex,0);
                          }
                      }
                  }
              }
          }      
      }


  }

}//end namespace tarski
