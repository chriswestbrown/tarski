#ifndef __CLEAR_ASSIGN_EXP_H
#define __CLEAR_ASSIGN_EXP_H

#include "clearAssignments-comm.h"
#include "deduction.h"
#include <vector>
namespace tarski {
  
  class ExpGraph : Graph {

  private:


    class SpanTree {
    private:
      std::vector<SpanNode> entries;
      int begin; //the beginning of this spantree - the first spannode
      Word shadowVal;
      TAtomRef shadowSource;
      bool hasShadow;
      PolyManager * pm;
      VarKeyedMap<GCWord> *constants;
      static const pair<GCWord,VarSet> *nada;
      VarKeyedmap< pair<GCWord, VarSet> > multiples;
      //Each spannode three vectors, where each index represents a child
      //Words is what we must multiply by (IE 3x = y, so mult by 3 to get x from y)
      //indices is the integer index of all the children in the vector entries
      //Sources is the TAtomRef that links two nodes (IE, some atom 3x - y = 0)
      struct SpanNode {
        std::vector<GCWord> words; //What you have to multiply by to simplify
        std::vector<int> indices; //The indices of all of the children
        std::vector<TAtomRef> sources; //The TAtomRef that lets us make each simplification
        int shadowIndex;
        inline SpanNode s() : shadowIndex(-1) { }
        inline bool hasShadow() { return shadowIndex != -1; }
        inline int shadowIndex() { return shadowIndex; }
        inline void addChild(GCWord w, int i, TAtomRef t) {
          words.push_back(w);
          indices.push_back(i);
          sources.push_back(t);
        }
        vector<Deduction *> mkDeductions(vector<Deduction *>& v, Word w, IntPolyRef p) {

        }
      };

    public:

      inline SpanTree(int size, PolyManager * pm) : entries(size), multiples(nada) {this->pm = pm;}
      inline void addNode(int par, int child, GCWord w, TAtomRef t) { entries[par].addChild(w, i, t); }
      inline void addShadow(int par, Word w, TAtomRef t) {
        if (par != begin) throw TarskiException("Only the first node of a spantree may be the shadow");
        shadowVal = w;
        shadowSource = t;
        hasShadow = true;
      }
      inline vector<Deduction *> makeDeductions() {
        vector<Deduction *> v;
        SpanNode& beg = entries[begin];
        if (hasShadow) {
          IntPolyRef p = new IntPolyObj(shadowVal);
          for (int i = 0; i < beg.words.size(); i++) {
            beg.mkDeductions(v, w, p);
          }
          
        }
        else {

        }
        return v;
      }
      TAtomRef genAtom(IntPolyRef p, short relop) {
        FactRef f = new FactObj(pm);
        f->addFactor(p, 1);
        TAtomRef t = new TAtomObj(f, relop);
      }
      Deduction * genDeduction(IntPolyRef p, short relop) {
        
      }
    };

    std::vector< std::vector<TAtomRef> > edgeAtoms;
    std::vector< TAtomRef> shadowAtoms;

  public:

    ExpGraph() : Graph() { }
    ExpGraph genExpGraph(TAndRef TF);

    //add an edge representing an equality which is of the form ax = b, or ax + b = 0
    //and source the edge to an atom
    //store the atom in shadowAtoms
    void addEdge(VarSet x, Word a, Word b, TAtomRef t); //a x + b  = 0

    //add an edge representing an equality which is of the form ax = by, or ax + by = 0
    //and source the edge to an atom
    //store the atom in edgeAtoms
    void addEdge(VarSet x, VarSet y, Word a, Word b, TAtomRef t); // a x + b y = 0 


    void dump(PolyManager* PMptr);

    //inline methods - getters
    //getter for an atom of the form ax = b or ax + b = 0
    inline TAtomRef getEdgeAtom(int i) { return shadowAtoms[i]; }
    //getter for an atom of the form ax = by or  ax + by = 0 
    inline TAtomRef getEdgeAtom(int i, int j) { return edgeAtoms[i][j]; }


    vector <Deduction *> genDeductions();
  };
x
}

#endif
