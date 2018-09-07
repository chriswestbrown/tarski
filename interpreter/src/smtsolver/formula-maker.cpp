#include "formula-maker.h"

namespace tarski{

  using namespace Minisat;
  using namespace std;

  void FormulaMaker::getItrs(TFormRef formula, set<TFormRef, ConjunctOrder>::iterator& beg, int& size) {
    if (formula->getTFType() == TF_OR) {
      TOrRef o = asa<TOrObj>(formula);
      beg = o->begin();
      size = o->size();
      return;
    }
    else if (formula->getTFType() == TF_AND) {
      TAndRef a = asa<TAndObj>(formula);
      beg = a->begin();
      size = a->size();
      return;
    }
  }


  short FormulaMaker::processSubForm(tarski::TFormRef formula) {
    if (formula->getTFType() == TF_ATOM)  {
      return 2;
    }
    else if (formula->getTFType() == TF_OR) {
      TOrRef o = asa<TOrObj>(formula);
      if (o->size() == 1) return 3;
      return 1;
    }
    else {
      TAndRef a = asa<TAndObj>(formula);
      if (a->size() == 1) return 4;
      return 0;
    }
  }


  TAtomRef FormulaMaker::extractAtom(tarski::TFormRef formula, short code) {
    if (code == 3) {
      TOrRef o = asa<TOrObj>(formula);
      set<TFormRef, ConjunctOrder>::iterator itr = o->begin();
      TAtomRef a = asa<TAtomObj>(*itr);
      return a;
    }
    else {
      TAndRef an = asa<TAndObj>(formula);
      set<TFormRef, ConjunctOrder>::iterator itr = an->begin();
      TAtomRef a = asa<TAtomObj>(*itr);
      return a;
    }
  }

  FormulaMaker  FormulaMaker::mkMaker(IdxManager * IM, tarski::TFormRef formula) {
    FormulaMaker  f;
    f.topLevel = false;
    f.varNum = IM->size();
    IM->incSize();
    if (formula->getTFType() == TF_AND) f.type = TF_AND;
    else f.type = TF_OR;
    set<TFormRef, ConjunctOrder>::iterator begin;
    int size; int num = 0;
    getItrs(formula, begin, size);
    while ( num < size) {
      int res = processSubForm(*begin);
      if (res == 3 || res == 4) {
        TAtomRef a = extractAtom(*begin, res);
        f.atoms.push_back(IM->getIdx(a));
      }
      else if (res == 2) {
        TAtomRef a = asa<TAtomObj>(*begin);
        f.atoms.push_back(IM->getIdx(a));
      }

      else {
        f.oppNodes.push_back(mkMaker(IM, *begin));
      }
      ++begin;
      num++;
    }
    return f;
  }

  FormulaMaker::FormulaMaker(TFormRef formula, IdxManager * IM) {
    topLevel = true;
    varNum = -1;
    if (formula->getTFType() == TF_AND) type = TF_AND;
    else type = TF_OR;
    set<TFormRef, ConjunctOrder>::iterator begin;
    int size; int num = 0;
    getItrs(formula, begin, size);
    while ( num < size) {
      int res = processSubForm(*begin);
      if (res == 3 || res == 4) {
        TAtomRef a = extractAtom(*begin, res);
        atoms.push_back(IM->getIdx(a));
      }
      else if (res == 2) {
        tarski::TAtomRef a = asa<TAtomObj>(*begin);
        atoms.push_back(IM->getIdx(a));
      }
      else {
        oppNodes.push_back(mkMaker(IM, *begin));
      }
      num++;
      ++begin;
    }
  }


  listVec FormulaMaker::mkFormula(){
    listVec toRet;
    mkFormulaHelper(toRet);
    return toRet;
  }


  void FormulaMaker::mkFormulaHelper(listVec& l) {
    if (type == TF_AND && topLevel) {
      //This pushes the atoms directly, so that they are on decision level 0 of a DPLL Stack
      for (vector<int>::iterator itr = atoms.begin(), end = atoms.end(); itr != end; ++itr) {
        Lit p = mkLit(*itr, false);
        l.emplace_front(1);
        l.front()[0] = p;
      }
      for (vector<FormulaMaker>::iterator itr = oppNodes.begin(), end = oppNodes.end(); itr != end; ++itr) {
        Lit p = mkLit(itr->varNum, false);
        l.emplace_front(1);
        l.front()[0] = p;
      }
    }
    else if (type == TF_AND) {
      //This makes a size 2 clause for every atom, so that they are all true or the node itself is false
      for (vector<int>::iterator itr = atoms.begin(), end = atoms.end(); itr != end; ++itr) {
        Lit p = mkLit(*itr, false);
        Lit q = mkLit(varNum, true);
        l.emplace_front(2);
        l.front()[0] = p;
        l.front()[1] = q;
      }
      for (vector<FormulaMaker>::iterator itr = oppNodes.begin(), end = oppNodes.end(); itr != end; ++itr) {
        Lit p = mkLit(itr->varNum, false);
        Lit q = mkLit(varNum, true);
        l.emplace_front(2);
        l.front()[0] = p;
        l.front()[1] = q;
      }
    }
    else if (type == TF_OR && topLevel) {
      //This makes one big disjunct for all the topLevel atoms
      l.emplace_front(atoms.size());
      for (unsigned int i = 0; i < atoms.size(); i++) {
        Lit p = mkLit(atoms[i], false);
        l.front()[i] = p;
      }
      cout << endl;
    }
    else if (type == TF_OR) {
      //Again, we make one big disjunct, but this time include the varNum of the node itself
      l.emplace_front(atoms.size()+oppNodes.size()+1);
      unsigned int i, j;
      for (i = 0; i < atoms.size(); i++) {
        Lit p = mkLit(atoms[i], false);
        l.front()[i] = p;
      }
      for (j = 0; j < oppNodes.size(); j++) {
        Lit p = mkLit(oppNodes[j].varNum, false);
        l.front()[i+j] = p;
      }
      Lit p = mkLit(varNum, true);
      l.front()[i+j] = p;
    }

    for (auto itr = oppNodes.begin(); itr != oppNodes.end(); ++itr) {
      itr->mkFormulaHelper(l);
    }
  }

}
