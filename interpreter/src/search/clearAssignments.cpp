#include "clearAssignments.h"
#include "clearAssignments-comm.h"
#include "formula.h"
#include "normalize.h"
#include <queue>
#include <stack>
#include <utility>
#include <cassert> 
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
            if (curr.first != nada->first || curr.second != nada->second) {
              GCWord moreContent;
              A = p->getPolyManagerPtr()->evalAtRationalMultipleOfVariable
                (A,*vitr,curr.first,curr.second,moreContent);
              content = RNPROD(content,moreContent);
            }
          }

        F->addMultiple(A,itr->second);
        if (RNSIGN(content) < 0 && itr->second % 2 == 1) {
          F->negateContent();
        }
      }

    int relop = p->relop;
    TAtomRef A = new TAtomObj(F,relop);

    // constant LHS case
    if (A->F->isZero()) {
      res = new  TConstObj(relop == EQOP || relop == LEOP || relop == GEOP);
      return;
    }
    if (A->F->isConstant()) {
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


  ExpGraph::ExpGraph(TAndRef F) : Graph() {
    for(TAndObj::conjunct_iterator itr = F->begin(); itr != F->end(); ++itr)
      {
        TAtomRef A = asa<TAtomObj>(*itr);
        if (A.is_null())
          throw TarskiException
            ("Command clear-assignments requires a pure conjunction!");
      
        if (A->getRelop() != EQOP || A->F->numFactors() != 1)
          continue;

        IntPolyRef p = A->factorsBegin()->first;
        /*
        if (verbose) {
          std::cout << "p = "; p->write(*PMptr);
          std::cout << " :: "; OWRITE(p->sP);
          std::cout << std::endl;
          }
        */
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
                addEdge(x,1,0,A);
                /*
                if (verbose) {
                std::cout << PMptr->getName(x) << " = 0" << std::endl;
                }
                */
                continue; 
              }
            FIRST2(Pp,&d,&b);
            addEdge(x,a,b,A);
            /*
            if (verbose) {
            IWRITE(a);
            std::cout << " " << PMptr->getName(x) << " + ";
            IWRITE(b);
            std::cout << " = 0" << std::endl;
            }
            */
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
            addEdge(x,y,a,b,A);
            /*
            if (verbose) {
              IWRITE(a); std::cout << " " << PMptr->getName(x) << " + "; 
              IWRITE(b); std::cout << PMptr->getName(y) << " = 0" << std::endl;            }
            */
          } break;
          default: break;
          }
      }
  }

  void ExpGraph::addEdge(VarSet x, Word a, Word b, TAtomRef t) {
    Graph::addEdge(x, a , b);
    int i = getVertex(x);
    if (shadowAtoms.size() <= i) shadowAtoms.resize(i+1);
    shadowAtoms[i] = t;
    pair<int, int> p(-i, i);
    edgeAtoms[p] = t;
  }

  void ExpGraph::addEdge(VarSet x, VarSet y, Word a, Word b, TAtomRef t) {
    Graph::addEdge(x, y, a, b);
    int i = getVertex(x), j = getVertex(y);
    shadowAtoms.resize(shadowAtoms.size()+1);
    pair<int, int> p(i, j);
    pair<int, int> q(j, i);
    edgeAtoms[p] = t;
    edgeAtoms[q] = t;
  }


  MarkLogExp::MarkLogExp(ExpGraph& E)
    : MarkLog(E.maxVertexIndex(), 0), atoms(E.maxVertexIndex()+1) {

    //set all shadowVertex values to 1
    std::vector<int> V = E.vertexList(); 
    for (int i = 0; i < V.size() && V[i] < 0; ++i) 
      E.setValue(V[i],RNINT(1));

    int nexti = 0;
    while(nexti < V.size())
      {
        int startVertex = V[nexti++];
        if (getMark(startVertex) != 0) continue;
        MarkLog::setMark(startVertex,startVertex);
        E.setValue(startVertex,RNINT(1));
        stack<int> S;
        stack<int> numChildren;
        list<TAtomRef> T;
        S.push(startVertex);

        while (!S.empty()) {
          if (numChildren.size() != 0 && numChildren.top() == 0) {
            numChildren.pop();
            T.pop_front();
          }
          else if (numChildren.size() != 0) {
            numChildren.top()--;
          }
          int i = S.top(); S.pop();
          int num = 0;
          for (std::vector<int>::iterator itr = E.nbr_begin(i); itr != E.nbr_end(i); ++itr) {
            int j = *itr;
            Word L = E.getEdgeWeight(i, j), a, b;
            FIRST2(L, &a, &b); //a xi + b xj = 0
            Word vi = E.getValue(i), vj = E.getValue(j);
            if (vj == NIL) {
              Word newVal = RNPROD(RNNEG(RNRED(a, b)), vi);
              E.setValue(j, newVal);
              T.push_front(E.getEdgeAtom(i, j));
              setMark(j, startVertex, {T.begin(), T.end()});
              S.push(j);
              num++;
            }
            else {
              assert(!(b == 0 && vi != 0));
              if (b != 0) {
                Word newVal = RNPROD(RNNEG(RNRED(a,b)), vi);
                if (!EQUAL(newVal, vj)) {
                  assert(startVertex >= 0);
                  E.setValue(startVertex, 0);
                }
              }
            }
          }
          numChildren.push(num);
        }

      }
  }

  const pair<GCWord, VarSet> SubExp::nada = {0, 0};
  const list<TAtomRef> SubExp::empty = {};
  
  SubExp::SubExp(ExpGraph& E, MarkLogExp& rootFor, PolyManager * PMptr)
    : constants(NIL), multiples(nada), exp(empty), PM(PMptr) {
    vector<int> V = E.vertexList();
    for (size_t i = 0; i < V.size(); i++) {
      if (V[i] < 0) continue;
      Word val = E.getValue(V[i]);
      if (val == 0 || rootFor.getMark(V[i]) != V[i]) {
        VarSet x = E.getVarFromVertex(V[i]);
        int r = rootFor.getMark(V[i]);
        if (r < 0) {
          constants[x] = val;
          list<TAtomRef>& lt = rootFor.getSource(V[i]);
          exp[x] = rootFor.getSource(V[i]);
        }
        else if (E.getValue(r) == 0) {
          constants[x] = 0;
          exp[x] = rootFor.getSource(V[i]);
        }
        else {
          multiples[x] = pair<GCWord, VarSet>(val, E.getVarFromVertex(r));
          exp[x] = rootFor.getSource(V[i]);
        }
      }
    }
  }

  list<DedExp> SubExp::makeDeductions(TAndRef t) {
    list<DedExp> res;
    for (TAndObj::conjunct_iterator itr = t->begin(); itr != t->end(); ++itr) {
      forward_list<TAtomRef> source;
      TAtomRef atom = *itr;
      FactRef F = new FactObj(*PM, atom->F->content); 
      for(std::map<IntPolyRef,int>::iterator atomItr = atom->factorsBegin();
          atomItr != atom->factorsEnd(); ++atomItr) {
        GCWord content;
        IntPolyRef A = evalAtRat(atomItr->first, constants, content, exp, source);
        if (A->isZero()) {
          F = new FactObj(PM);
          F->addFactor(A, 1);
          break;
        }
        //TODO: L1 Normalize as it is read in
        VarSet V = A->getVars();
        for (VarSet::iterator vItr = V.begin(); vItr != V.end(); ++vItr) {
          pair<GCWord, VarSet> curr = (multiples)[*vItr];
          if (curr.first != nada.first || curr.second != nada.second) {
            GCWord moreContent;
            A = PM->evalAtRationalMultipleOfVariable
              (A,*vItr,curr.first,curr.second,moreContent);
            content = RNPROD(content,moreContent);
            list<TAtomRef>& t = exp[*vItr];
            source.insert_after(source.before_begin(), t.begin(), t.end());
          }
        }
        F->addMultiple(A,atomItr->second);
        if (RNSIGN(content) < 0 && atomItr->second % 2 == 1) {
          F->negateContent();
        }
      }
      source.push_front(atom);
      int relop = atom->relop;
      TAtomRef substituted = new TAtomObj(F,relop);
      TAndRef normalized = new TAndObj();
      bool isSat = level1_atom(substituted, normalized);
      if (!isSat) {
        list<DedExp> newRes;
        newRes.emplace_front(Deduction::SUBST, source);
        return newRes;
      }
      else {
        //We want to reject deductions of the form 0 = 0, becuase we dont need to remove constants
        //std::cerr << "Is the form " << toString(substituted) << endl;
        if (substituted->getRelop() == EQOP && substituted->getFactors()->isConstant() == 1 && substituted->getFactors()->isZero()) {
          continue;
        }

        //DR BROWN THINKS IM WRONG
        //This represents that something has been simplified out to a constant
        if (normalized->begin() == normalized->end()) {
          forward_list<TAtomRef> tmp(source);
          tmp.emplace_front(substituted);
          res.emplace_front(substituted, Deduction::SUBST, source);
          res.emplace_front(atom, Deduction::SUBST, tmp);


        }
        for (TAndObj::conjunct_iterator itr = normalized->begin();
             itr != normalized->end(); ++itr) {
          TAtomRef out = asa<TAtomObj>(*itr);
          forward_list<TAtomRef> tmp(source);
          tmp.emplace_front(out);
          res.emplace_front(out, Deduction::SUBST, source);
          res.emplace_front(atom, Deduction::SUBST, tmp);




        }
      }

    }
    return res;
  }

  IntPolyRef SubExp::evalAtRat(IntPolyRef p, VarKeyedMap<GCWord> &value,
                               GCWord &content,
                               VarKeyedMap<list<TAtomRef> >& sources,
                               forward_list<TAtomRef>& exp) {
    VarSet V = p->getVars(), Vremain = 0;
    Word perm1 = NIL, perm2 = NIL, Q = NIL;
    int i = 1;
    for(VarSet::iterator itr = V.begin(); itr != V.end(); ++itr, ++i)
      {
        Word a = value[*itr];
        if (a == NIL) { perm2 = COMP(i,perm2); Vremain = Vremain + *itr; }
        else {
          perm1 = COMP(i,perm1);
          Q = COMP(a,Q);
          list<TAtomRef>& t = sources[*itr];
          exp.insert_after(exp.before_begin(), t.begin(), t.end());
        }
      }

    // if Q == NIL, there are no values to evaluate at, so just return
    if (Q == NIL) { content = RNINT(1); return p; }

    Q = CINV(Q);

    Word perm = CINV(CCONC(perm2,perm1));
    Word sPnew = PPERMV(p->slevel,p->sP,perm);
    Word sPeval = IPRNME(p->slevel,sPnew,Q);
    Word sleveleval = p->slevel - LENGTH(Q);

    if (sPeval == 0) { content = RNINT(1); return new IntPolyObj(); }
    if (sleveleval == 0) { content = sPeval; return new IntPolyObj(1); }

    Word c, Cb;
    IPSRP(sleveleval,sPeval,&c,&Cb);
    content = RNINV(c);
    return new IntPolyObj(sleveleval,Cb,Vremain);
  }

  void Substituter::makeDeductions(TAndRef t) {
    PolyManager * PM = t->getPolyManagerPtr();

    ExpGraph E(t);
    MarkLogExp M(E);

    VarKeyedMap<GCWord> constants(NIL);
    pair<GCWord, VarSet> nada(0, 0);
    VarKeyedMap<pair<GCWord,VarSet> >multiples(nada);
    list<TAtomRef> empty;
    VarKeyedMap<list<TAtomRef> > exp(empty);
    SubExp S(E, M, PM);
    deductions = S.makeDeductions(t);
  }

  DedExp Substituter::deduce(TAndRef t, bool& res) {
    if (deductions.empty()) {
      if (!once) {
        once = true;
        deductions.clear();
        res = false;
        DedExp d;
        return d;
      }
      else {
        makeDeductions(t);
        if (deductions.empty()) {
          res = false;
          DedExp d;
          return d;
        }
        else once = false;

      }
    }
    DedExp d = deductions.back();
    deductions.pop_back();
    return d;
  }

} //end namespace
