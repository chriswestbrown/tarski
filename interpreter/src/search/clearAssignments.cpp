#include "clearAssignments.h"
#include "formula.h"

namespace tarski {
  Graph::Graph() : var2vert(-1) 
  { 
    VarSet v; 
    vertices.push_back(v); 
    hasShadowVertex.push_back(0); 
    Nbrs.push_back(std::vector<int>()); 
    ShadowNbrs.push_back(std::vector<int>()); 
    ShadowNbrs.push_back(std::vector<int>()); 
  }

  void Graph::dump(PolyManager* PMptr)
  {
    int N = vertices.size()-1;
    for(int i = -N; i < 0;  ++i)
      writeShadow(-i, PMptr);
    for(int i = 1; i <= N; ++i)
      writeEdge(i, PMptr);
  }

  void Graph::writeShadow(int i, PolyManager* PMptr) {
    if (i > 0 && i < vertices.size()-1 && hasShadowVertex[i]) {
      std::cout << i << "(-" << PMptr->getName(vertices[i]) << ")"
                << ": " << -i << "(" << PMptr->getName(vertices[i]) << ")"  << std::endl;
    }

  }

  void Graph::writeEdge(int i, PolyManager* PMptr) {
    if (i > 0 && i < vertices.size()-1) {
      std::cout << i << "(" << PMptr->getName(vertices[i]) << ")" 
                << ":";

      for(int j = 0; j < Nbrs[i].size(); ++j)
        writeEdge(i, j, PMptr);
    }
  }

  void Graph::writeEdge(int i, int j, PolyManager* PMptr) {
    std::cout << " " << Nbrs[i][j]
              << "(" << PMptr->getName(vertices[abs(Nbrs[i][j])]) << ")";
    std::cout << std::endl;
  }

  int Graph::getVertex(VarSet x)
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
  void Graph::addEdge(VarSet x, Word a, Word b) // a x + b = 0
  {
    int i = getVertex(x);
    hasShadowVertex[i] = 1;
    Nbrs[i].push_back(-i);
    ShadowNbrs[i].push_back(i);
    edgeWeight[ pair<int,int>(i,-i) ] = LIST2(a,b);
    edgeWeight[ pair<int,int>(-i,i) ] = LIST2(b,a);
  }
  void Graph::addEdge(VarSet x, VarSet y, Word a, Word b) // a x + b y = 0
  {
    int i = getVertex(x), j = getVertex(y);
    if (!checkValid(i, j, a, b)) {
      addEdge(x,1,0); 
      addEdge(y,1,0); 
    }
    else {
      Nbrs[i].push_back(j);
      Nbrs[j].push_back(i);
      edgeWeight[ pair<int,int>(i,j) ] = LIST2(a,b);
      edgeWeight[ pair<int,int>(j,i) ] = LIST2(b,a);
    }
  }

  bool Graph::checkValid(int i, int j, Word a, Word b) {
    std::map< pair<int,int> , GCWord>::iterator itr = edgeWeight.find(pair<int,int>(i,j));
    if (itr != edgeWeight.end())
      {
        Word Lp = itr->second, ap, bp;
        FIRST2(Lp,&ap,&bp);
        if (ICOMP(IPROD(a,bp),IPROD(ap,b)) != 0)
          { // in this case, we can only satisyfy a x + b y = 0 = ap x + bp y if x = y = 0
            return false;
          }
        return true;
      }
    return true;
  }

  std::vector<int> Graph::vertexList()
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






  void MakeAssignments::action(TAtomObj* p) 
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
  void MakeAssignments::action(TAndObj* p) 
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




} //end namespace
