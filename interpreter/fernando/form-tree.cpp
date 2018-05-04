

#include "form-tree.h"
//Takes an arbitrary formula as a TFormRef and turns it into a binary tree

namespace Minisat {
  using namespace tarski;
  using namespace std;

  const int ATOM = 0;
  const int AND = 1;
  const int OR = 2;


  void FormTree::process(TFormRef formula, IdxManager * IM) {
    switch (formula->getTFType()) {
    case TF_ATOM: {
      TAtomRef a = asa<TAtomObj>(formula);
      this->left = NULL;
      this->right = NULL;
      type = ATOM;
      varNum = IM->getIdx(a);
      return;
    }
    case TF_EXTATOM: {
      throw TarskiException("Error! Unexpected TExtAtomRef\n");
      break;
    }
    case TF_CONST: {
      TAtomRef a = asa<TAtomObj>(formula);
      this->left = NULL;
      this->right = NULL;
      type = ATOM;
      varNum = IM->getIdx(a);
      break;
    }
      //I want to avoid the duplicate code, but in order to do that, TAndRef and TOrRef need to be part of the same superclass
      //Which supports the size, begin, and end methods
    case TF_AND: {
      TAndRef C = asa<TAndObj>(formula);
      if (C.is_null()) throw TarskiException ("Something went wrong");
      if (C->size() == 1) {
        process(*(C->begin()), IM);
        return;
      }
      varNum = IM->size();
      IM->incSize();
      type = AND;
      bool earlyTerm;
      int size = C->size();
      vector<FormTree *> nodeList = mkNodes(AND, size, C->begin(), C->end(), IM, earlyTerm);
      if (earlyTerm) return; //if the earlyterm condition is true, no further processing required
      int maxNodes;
      if (C->size()%2 == 1) maxNodes = 1; //if its odd, we need to collapse to one node
      else maxNodes = 2; //if its even, we need to fill both left and right of this node
      collapseList(nodeList, AND, maxNodes, IM);
      if (C->size()%2 == 1 && nodeList.size() == 2) {
        left = new FormTree(type, nodeList[0], nodeList[1], IM);
      }
      else if (C->size()%2 == 1 && nodeList.size() == 1) {
        left = nodeList[0];
      }
      else if (C->size()%2 == 0 && nodeList.size() ==1) {
        throw TarskiException("Error in creating CNF Formula");
      }
      else {
        this->left = nodeList[0];
        this->right = nodeList[1];
      }
      return;
    }
    case TF_QB: throw TarskiException("Unexpected Quantifier!");
    case TF_OR: {
      TOrRef C = asa<TOrObj>(formula);
      if (C.is_null()) throw TarskiException ("Something went wrong");
      if (C->size() == 1) {
        process(*(C->begin()), IM);
        return;
      }
      type = OR;
      varNum = IM->size();
      IM->incSize();

      bool earlyTerm;
      int size = C->size();
      vector<FormTree *> nodeList = mkNodes(OR, size, C->begin(), C->end(), IM, earlyTerm);
      if (earlyTerm) return;
      int maxNodes;
      if (C->size()%2 == 1) maxNodes = 1;
      else maxNodes = 2;
      collapseList(nodeList, OR, maxNodes, IM);
      if (C->size()%2 == 1 && nodeList.size() == 2) {
        left = new FormTree(type, nodeList[0], nodeList[1], IM);
      }
      else if (C->size()%2 == 1 && nodeList.size() == 1) {
        left = nodeList[0];
      }
      else if (C->size()%2 == 0 && nodeList.size() ==1) {
        throw TarskiException("Error in creating CNF Formula");
      }
      else {
        this->left = nodeList[0];
        this->right = nodeList[1];
      }
      return;
    }
      break;
    }
  }

  vector<FormTree *>  FormTree::mkNodes(short type,  int size, set<TFormRef, ConjunctOrder>::iterator begin, set<TFormRef, ConjunctOrder>::iterator end, IdxManager * IM, bool& earlyTerm ) {
    std::vector<FormTree *> nodeList;
    if (size == 1) {
      this->left = NULL;
      this->right = NULL;
      earlyTerm = true;
      TAtomRef a = asa<TAtomObj>(*begin);
      type = ATOM;
      if (a.is_null()) throw TarskiException("Multi Wrapped Object in mkNodes in box-solver.cpp");

      varNum = IM->getIdx(a);
      return nodeList;
    }
    if (size == 2) {
      //there are only two nodes, so we can easily process this by just setting the left and right
      FormTree * left = new FormTree(*begin, IM);
      ++begin;
      FormTree * right = new FormTree(*begin, IM);
      this->left = left;
      this->right = right;
      earlyTerm = true;
      return nodeList;
    }

    //We want there to be an even number for us to process, so we take one element out arbitrarily and set it to the right
    if (size%2 == 1) {
      this->right = new FormTree(*begin, IM);
      ++begin;
    }

    //Create the nodelist, one pair at a time
    while (begin != end) {
      TFormRef t1 = *begin;
      ++begin;
      TFormRef t2 = *begin;
      ++begin;
      FormTree * l = new FormTree(t1, IM);
      FormTree * r = new FormTree(t2, IM);
      FormTree * par = new FormTree(type, l, r, IM);
      nodeList.push_back(par);
    }
    earlyTerm = false;
    return nodeList;
  }

  void FormTree::collapseList(vector<FormTree *>& nodeList, short type, int maxNodes, IdxManager * IM) {
    //takes two nodes at a time, combines them into one
    while (nodeList.size() > maxNodes) {
      vector<FormTree *> nodeListNew;
      vector<FormTree *>::iterator begin = nodeList.begin();
      vector<FormTree *>::iterator end = nodeList.end();
      while (begin != end) {
        FormTree * f1 = *begin;
        ++begin;
        if (begin == end) {
          nodeListNew.push_back(f1);
          nodeList = nodeListNew;
          break;
        }
        FormTree * f2 = *begin;
        ++begin;
        FormTree * f3 = new FormTree(type, f1, f2, IM);
        nodeListNew.push_back(f3);
      }
      nodeList = nodeListNew;
    }
  }

  void FormTree::mkFormulaHelper(vector < vector < Lit > >& formula) {
    //Follows sub-expressions on wikipedia page for Tseytin transformation
    //Left is A
    //Right is B
    //this is C
    if (type == ATOM) return;
    int l = abs(left->varNum);   bool lsgn = false;
    int r = abs(right->varNum);  bool rsgn = false;
    int t = abs(this->varNum);   bool tsgn = false;
    if (type == AND) {
      //cout << "type is AND, my num is " << varNum << ", left num is " << left->varNum << ", right num is " << right->varNum << endl;
      vector<Lit> first(3);

      first[0] = mkLit(l, !lsgn);
      first[1] = mkLit(r, !rsgn);
      first[2] = mkLit(t, tsgn);

      vector<Lit> second(2);
      second[0] = mkLit(l, lsgn);
      second[1] = mkLit(t, !tsgn);

      vector<Lit> third(2);
      third[0] = mkLit(r, rsgn);
      third[1] = mkLit(t, !tsgn);

      formula.push_back(first);
      formula.push_back(second);
      formula.push_back(third);
      left->mkFormulaHelper(formula);
      right->mkFormulaHelper(formula);
    }
    else if (type == OR) {
      //cout << "type is OR, my num is " << varNum << ", left num is " << left->varNum << ", right num is " << right->varNum << endl;
      vector<Lit> first(3);
      first[0] = mkLit(l, lsgn);
      first[1] = mkLit(r, rsgn);
      first[2] = mkLit(t, !tsgn);

      vector<Lit> second(2);
      second[0] = mkLit(l, !lsgn);
      second[1] = mkLit(t, tsgn);

      vector<Lit> third(2);
      third[0] = mkLit(r, !rsgn);
      third[1] = mkLit(t, tsgn);

      formula.push_back(first);
      formula.push_back(second);
      formula.push_back(third);
      left->mkFormulaHelper(formula);
      right->mkFormulaHelper(formula);
    }

  }






  FormTree::~FormTree() {
    delete left;
    delete right;
  }

}
