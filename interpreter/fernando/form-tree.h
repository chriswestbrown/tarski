#include <vector>
#include "../minisat/core/SolverTypes.h"
#include "../formula/formula.h"
#include "idx-manager.h"

namespace Minisat {

  //A legacy class which does a sub-optimal formula translation
  //Do not modify or intend to improve
  class FormTree {
  protected:
    short type; //0 = variable, 1 = negation, 2 = and, 3 = or
    int varNum; //the variable reserved for the tseitin transform of each node, or the actual variable if type is 0
    FormTree * left;
    FormTree * right;
    void mkFormulaHelper(vector<vector<Lit> >& formula);
    void process(tarski::TFormRef formula, IdxManager * IM);
    //maxNodes should be 1 or 2. No error checking done
    void collapseList(vector<FormTree *>& nodeList, short type, int maxNodes, IdxManager * IM);
    vector<FormTree *>  mkNodes(short type, int size, set<tarski::TFormRef, tarski::ConjunctOrder>::iterator begin, set<tarski::TFormRef, tarski::ConjunctOrder>::iterator end, IdxManager * IM, bool& earlyTerm);
    static vector<tarski::TAtomRef> topConjunctAtoms;
    inline FormTree(short t, FormTree * l, FormTree * r, IdxManager * IM) {
      left = l;
      right = r;
      varNum = IM->size();
      type = t;
      IM->incSize();
    }

  public:
    FormTree(tarski::TFormRef formula, IdxManager * IM) {
      left = NULL;
      right = NULL;
      process(formula, IM);
    }
    ~FormTree();
    //vector<FormTree *> getAllPtrs();
    //void getAllPtrsHelper(vector<FormTree *>& ptrs);
    vector<vector<Lit> > mkFormula() {
      vector<vector<Lit > > formula;
      vector<Lit> topLevel(1);
      topLevel[0] = mkLit(varNum, true);
      formula.push_back(topLevel);
      mkFormulaHelper(formula);
      return formula;
    };
  };
}
