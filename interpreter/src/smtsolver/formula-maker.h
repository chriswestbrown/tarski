#ifndef FORMULA_MAKER_H
#define FORMULA_MAKER_H
#include "../../../minisat/core/CallBack.h"
#include "../../../minisat/core/SolverTypes.h"
#include "../formula/formula.h"
#include "idx-manager.h"
#include "box-solver.h"
#include <vector>
#include <set>
namespace tarski {
  
  class FormulaMaker {
  protected:
    bool topLevel;
    short type; //0 if AND, 1 if OR, 2 if ATOM
    int varNum; //the variable reserved for the tseitin transform of each node
    std::vector<int> atoms; //the indices of the atoms in this sub-formula
    std::vector<FormulaMaker> oppNodes; //It makes no sense to have an AND in an AND, or an OR in an OR
    //Return 0 if AND, 1 if OR, 2 if ATOM
    short processSubForm(tarski::TFormRef formula);
    FormulaMaker  mkMaker(IdxManager * IM, tarski::TFormRef formula);
    FormulaMaker() {}
    tarski::TAtomRef extractAtom(tarski::TFormRef formula, short code);
    void getItrs(tarski::TFormRef formula,
                 std::set<tarski::TFormRef, tarski::ConjunctOrder>::iterator& beg,
                 int& size);
  public:
    FormulaMaker(tarski::TFormRef formula, IdxManager * IM);
    std::vector<std::vector<Minisat::Lit> > mkFormula();
    void mkFormulaHelper(std::vector<std::vector<Minisat::Lit> >& formula);
  };

}

#endif
