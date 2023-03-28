#ifndef __CLEAR__ASSIGN_H
#define __CLEAR__ASSIGN_H

#include "../formula/formula.h"
#include "deduction.h"
#include "solver-manager.h"
#include <list>

/*
  The idea behind "clear-assignments" is that formulas often come with
  "assignments" like x = 2 or y = 3 x.  In contrast with more general
  linear equality constraints, there is no real decision to make about
  what to do with these: if you see an assignment of a constant to
  a variable, evaluate the formula with that variable assigned that
  constant value.   If you see a constraint like 3 x = 5 y, choose one
  variable, it matters not which, and substitute e.g. x <- 5/3 y throughout
  the formula.  
*/



namespace tarski {


  class Graph
  {
    // The vertices of this graph are 1,2,..,n - which correspond to variables via the vertices vector
    // For vertex i, if hasShadowVertex[i] == 1, then -i is also a vertex in the graph
    // If (i,j) is an edge with weight (a,b), then (j,i) is an edge with weight (b,a).
    // the meaning of (i,j,(a,b)) where i,j > 0 is that a x_i + b x_j = 0 is an atom of conjunction F
    // the meaning of (i,-i,(a,b)) where i,i > 0 is that a x_i + b = 0 is an atom of conjunction F
    // there are no other kinds of edges involving negative vertices.
  private:
    std::vector<VarSet> vertices;
    std::vector<int> hasShadowVertex;
    VarKeyedMap<int> var2vert;
    std::vector< std::vector<int> > Nbrs, ShadowNbrs;
    std::map<int,GCWord> value;
    std::map< pair<int,int> , GCWord> edgeWeight;

  public:
    Graph();
    inline int maxVertexIndex() { return vertices.size() - 1; }
    inline Word getValue(int i) { if (value.find(i) == value.end()) return NIL; else return value[i]; }
    inline void setValue(int i, Word val) { value[i] = val; }
    inline Word getEdgeWeight(int i, int j) { return edgeWeight[ pair<int,int>(i,j) ]; }

    inline std::vector<int>::iterator nbr_begin(int v) {
      return v > 0 ? Nbrs[v].begin() : ShadowNbrs[-v].begin();
    }
    inline std::vector<int>::iterator nbr_end(int v) {
      return v > 0 ? Nbrs[v].end() : ShadowNbrs[-v].end();
    }
    inline bool hasShadow(int i) { return hasShadowVertex[i]; }
    inline VarSet getVarFromVertex(int i) { return vertices[abs(i)]; }
    bool checkValid(int i, int j, Word a, Word b);
    void dump(PolyManager* PMptr);
    void writeShadow(int i, PolyManager* PMptr);
    void writeEdge(int i, PolyManager* PMptr);
    void writeEdge(int i, int j, PolyManager* PMptr);
    int getVertex(VarSet x);
    void addEdge(VarSet x, Word a, Word b); // a x + b = 0
    void addEdge(VarSet x, VarSet y, Word a, Word b); // a x + b y = 0
    std::vector<int> vertexList();
  };

  class ExpGraph : public Graph {
  private:
    vector<TAtomRef> shadowAtoms;
    std::map< pair<int, int>, TAtomRef> edgeAtoms;
    PolyManager* PMptr; //-- Chris added
  public:
    ExpGraph(TAndRef F);
    void addEdge(VarSet x, Word a, Word b, TAtomRef t);
    void addEdge(VarSet x, VarSet y, Word a, Word b, TAtomRef t);
    inline TAtomRef getEdgeAtom(int i, int j) {
      pair<int, int> p(i, j);
      return edgeAtoms[p];
    }
  };

  class MarkLog
  {
  private:
    std::vector<int> pos, neg;
  public:
    inline MarkLog(int N, int initial) : pos(N + 1,initial), neg(N + 1,initial) { }
    inline int setMark(int i, int val) // returns previous mark value
    {
      if (i < 0) { int t = neg[-i]; neg[-i] = val; return t; }
      else { int t = pos[i]; pos[i] = val; return t; }
    }
    inline int getMark(int i) { return i < 0 ? neg[-i] : pos[i]; }
  };

  class MarkLogExp : public MarkLog {
  private:
    std::vector<std::list<TAtomRef> > atoms;
    void writeList(std::list<TAtomRef>& t) {
      std::cout << "list: ";
      for (std::list<TAtomRef>::iterator itr = t.begin(); itr != t.end();
           ++itr) {
        (*itr)->write(); std::cout << " ";
      }
      std::cout << " end";
    }
  public:
    MarkLogExp(ExpGraph& E);
    inline int setMark(int i, int val, list<TAtomRef> t) {
      atoms[i] = t;
      return MarkLog::setMark(i, val);
    }
    inline list<TAtomRef>& getSource(int i) { return atoms[i]; }
  };

  class SubExp;
  typedef GC_Hand<SubExp> SubExpRef;
  class SubExp : public GC_Obj
  {
  private:
    static const pair<GCWord, VarSet> nada;
    static const list<TAtomRef> empty;
    VarKeyedMap<GCWord> constants;
    VarKeyedMap<pair<GCWord, VarSet> > multiples;

    // value at Variable x is the list of atoms that provided the substitution eliminating x [maybe?]
    VarKeyedMap<list<TAtomRef> > exp;

    PolyManager * PM;
    ExpGraph E;
    MarkLogExp rootFor;
    TAndRef t;

    IntPolyRef evalAtRat(IntPolyRef p, VarKeyedMap<GCWord> &value, GCWord &content,
			 VarKeyedMap<list<TAtomRef> >& sources, forward_list<TAtomRef>& exp);

  public:
    SubExp(TAndRef t);
    list<DedExp> makeDeductions();
    vector<VarSet> getVarsEliminatedBySubstitutions(set<TAtomRef,TAtomObj::OrderComp> &usedToElim);
  };

  class Substituter : public QuickSolver
  {
  private:
    bool once;
    std::list<DedExp> deductions;
    void makeDeductions(TAndRef t);

    //-- for tracking substitution level
    VarKeyedMap<int> orderSubstituted; //-- 0 means not eliminated by substution
    int nextSubCounter;

    //-- Stack of Substitutions
    stack<SubExpRef> subStack;
    stack<VarSet> eliminatedVars;
    set<TAtomRef,TAtomObj::OrderComp> usedToElim;
    
  public:
    Substituter(TAndRef& t) : once(true), orderSubstituted(0), nextSubCounter(1) {}
    bool isIdempotent() { return false; }
    void update(std::vector<Deduction>::const_iterator begin,
                std::vector<Deduction>::const_iterator end) { }
    void notify() {}
    DedExp deduce(TAndRef t, bool& res);
    std::string name() const { return "Substituter"; }
    void dump();
    int getSubstitutionLevel(VarSet x) { int k = orderSubstituted[x]; return k == 0  ? 0 : nextSubCounter - k; }
    TAndRef filter(TAndRef t);
  };


  class MakeAssignments : public TFPolyFun
  {
  private:

    TFormRef res;
    VarKeyedMap<GCWord> *constants;
    const pair<GCWord,VarSet> *nada;
    VarKeyedMap< pair<GCWord,VarSet> > *multiples;

  public:
    inline MakeAssignments(VarKeyedMap<GCWord> &constants, 
                           const pair<GCWord,VarSet> &nada, 
                           VarKeyedMap< pair<GCWord,VarSet> > &multiples) {
      this->constants = &constants;
      this->nada = &nada;
      this->multiples = &multiples;
    }
    inline TFormRef getRes() { return res; }
    inline void action(TConstObj* p) { res = p; }
    void action(TAtomObj* p);
    void action(TAndObj* p);
  };


}//end namespace tarski

#endif
