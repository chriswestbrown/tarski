#include "box-solver.h"
#include "boxer.h"
#include "formula-maker.h"
#include "../formula/formmanip.h"
#include "idx-manager.h"
#include "../shell/qepcad-inter/qepcad-session.h"
#include <algorithm>
#include <vector>
#include <string>

//So it turns out that the "using namespace" command is additive
using namespace Minisat;
using namespace tarski;
using namespace std;



inline bool intSort (int i, int j) { return (i < j);}

BoxSolver::BoxSolver(TFormRef formula) :  limit(5), count(0), isPureConj(true), numAtoms(-1) {
  this->formula = formula;
  IM = new IdxManager();
  pm = formula->getPolyManagerPtr();
  processAtoms(formula);
  if (!isPureConj){
    S = new Solver(this);
    S->mkProblem(makeFormula(formula));
    M = (numAtoms > 5) ? new MHSGenerator(form, numAtoms) : NULL;
  }
}
BoxSolver::~BoxSolver() {
  if (!isPureConj) {
    delete S;
    if (numAtoms > 5) delete M;
  }
}

vector<vector<Lit> > BoxSolver::makeFormula(TFormRef formula) {
  FormulaMaker  f(formula, IM);
  form = f.mkFormula();
  return form;
}

bool BoxSolver::solve(string& err) {
  if (isPureConj)
    try {return directSolve();}
    catch (TarskiException& e) {
      err = string(e.what()); return false;
    }
  err = "";
  if (!S->simplify()) return false;
  Minisat::vec<Minisat::Lit> dummy;
  Minisat::lbool ret;
  try {
    ret = S->solveLimited(dummy);
  }
  catch (TarskiException& e){
    err = string(e.what());
    return false;
  }
  return (ret == l_True) ? true : false;
}

//0 = false
//1 = true
//2 = err
short BoxSolver::solve() {
  if (isPureConj)
    try {return directSolve();}
    catch (TarskiException& e) {
      return 2;
    }
  if (!S->simplify()) return 0;
  Minisat::vec<Minisat::Lit> dummy;
  Minisat::lbool ret;
  try {
    ret = S->solveLimited(dummy);
  }
  catch (TarskiException& e){
    return 2;
  }
  return (ret == l_True) ? 1 : 0;
}

bool BoxSolver::directSolve() {
  TAndRef t = asa<TAndObj>(formula);
  if (t->constValue() == true) return true;
  Boxer b(t);
  b.deduceAll();
  if (b.isUnsat()) return false;
  TAndRef t2 = asa<TAndObj>(formula);
  TFormRef res;
  try  {
    QepcadConnection q;
    res = q.basicQepcadCall(exclose(t2), true);
  }
  catch (TarskiException& e) {
    throw TarskiException("QEPCAD timed out");

  }
  if (res->constValue() ==0) return false;
  else return true;
}

//Accesses each atom recursively
//Assigns an idx to each atomref
void BoxSolver::processAtoms(TFormRef formula) {
  switch (formula->getTFType()) {
  case TF_ATOM: {
    TAtomRef a = asa<TAtomObj>(formula);
    if (IM->getIdx(a) == -1) {
      IM->mkIdx(a);
      numAtoms++;
    }
    break;
  }
    //Don't know what these are
  case TF_EXTATOM: {
    throw TarskiException("Unexpected TF_EXTATOM in processAtoms!");
    break;
  }
  case TF_CONST:  break;
  case TF_AND: {
    //recursively access each element here of the AND block
    TAndRef C = asa<TAndObj>(formula);
    for(set<TFormRef, ConjunctOrder>::iterator it = C->begin(); it != C->end(); ++it ) {
      processAtoms(*it);
    }
    break;
  }

  case TF_QB: throw TarskiException("Unexpected Quantifier!");

  case TF_OR: {
    //recursively access each element here of the OR block
    TOrRef C = asa<TOrObj>(formula);
    for (set<TFormRef, ConjunctOrder>::iterator it = C->begin(); it != C->end(); ++it ) {
      processAtoms(*it);
    }
    isPureConj = false;
    break;
  }
  }
}



//This method is used to verify the complete solution with QEPCAD
//TODO: Generate learned clauses from QEPCAD Unsat Core
void BoxSolver::getFinalClause(vec<Lit>& lits, bool& conf) {
  //cout << "Called getFinalClause\n";
  getClauseMain(lits, conf);
  //Only if BB/WB cannot detect unsat do we need to call QEPCAD"
  if (conf == false) {

    QepcadConnection q;
    //cout << "LastSatisfying() is " << x << ", qhead is " << getQhead() << endl;
    TAndRef tand = (M != NULL) ? genMHS() : genTAnd(getQhead());
    //cout << "Final Conjunct is: "; tand->write(); cout << endl;
    //cout << "getqhead is " << getQhead() << endl;
    //cout << "Final Result: "; tand->write(); cout << endl;
    if (tand->constValue() == 1) return;

    TFormRef res;
    try {

      res = q.basicQepcadCall(exclose(tand), true);
      //cerr << "Call Done!\n";
    }
    catch (TarskiException& e) {
      throw TarskiException("QEPCAD timed out");
    } 
    if (res->constValue() == 0) {
      conf = true;
      vector<int> core = q.getUnsatCore();
      //NOTE: Core is not guaranteed to come out in sorted order!!!
      sort(core.begin(), core.end());
      /*
      cout << "CORE: ";
      for (int i = 0; i < core.size(); i++) {
        cout << core[i] << " ";
      }
      cout << endl;
      */
      getQEPUnsatCore(lits, core, tand);
      //cerr << "CORE FORMULA:"; tand->write(); cerr << endl;
    }
  }
}

void BoxSolver::getQEPUnsatCore(vec<Lit>& lits, vector<int> indices, TAndRef tand) {
  int curr = 0;
  int currVec = 0;
  //TAndRef t = new TAndObj();
  for (set<TFormRef, ConjunctOrder>::iterator begin = tand->begin(), end = tand->end();
       begin != end; ++begin) {
    if (indices[currVec] == curr) {
      TAtomRef a = asa<TAtomObj>(*begin);
      if (a.is_null()) throw TarskiException("Unexpected non-atom in getQEPUnsatCore");
      //t->AND(a);
      int varNum = IM->getIdx(a);
      Lit l = litFromInt(varNum, true);
      lits.push(l);
      currVec++;
      if (currVec >= indices.size()) break;
    }
    curr++;
  }
  //cerr << "UNSAT CORE IS: "; t->write(); cerr << endl;
}

TAndRef BoxSolver::genMHS() {
  TAndRef tand = new TAndObj();
  vector<Lit> res = M->genMHS(getTrail());
  for (vector<Lit>::iterator itr = res.begin(); itr != res.end(); ++itr) {
    Lit p = *itr;
    if (sign(p) == true) continue;
    int v = var(p);
    TAtomRef tatom;
    bool res = IM->getAtom(v, tatom);
    if (res) {
      tand->AND(tatom);
    }
  }
  return tand;
 
}


TAndRef BoxSolver::genTAnd(int maxIdx) {
  TAndRef tand = new TAndObj();
  const vec<Lit>& trail = getTrail();
  for (int i = 0; i < maxIdx; i++) {
    Lit p = trail[i];
    //write(p); cout << " ";
    if (sign(p) == true) continue;
    int v = var(p);
    TAtomRef tatom;
    bool res = IM->getAtom(v, tatom);
    if (res) {
      tand->AND(tatom);
    }
  }
  //cout << "genTAND: ";
  //tand->write();
  //cout << endl;
  return tand;
}

void BoxSolver::getClause(vec<Lit>& lits, bool& conf) {
  if (limit <= count) {
    count = 0;
    getClauseMain(lits, conf);
  }
  else {
    count++;
    conf = false;
  }
}

void BoxSolver::getClauseMain(vec<Lit>& lits, bool& conf) {
  lits.clear();
  //Step 1: Add to tand
  //Step 1a: For each lit in trail
  //Step 1a.1: Map to the corresponding TAtomRef
  //Step 1a.2: AND it with tand
  TAndRef tand = genTAnd(getQhead());

  cout << "PROBLEM: "; tand->write(); cout << endl;
  if (tand->constValue() == TRUE) {

    conf = false;
    return;
  }

  
  //Step 2: Construct a BB/WB Solver bbwb with tand and solve
  Boxer b(tand);
  Result res = b.deduceAll();
  if (b.isUnsat()) {   //Step 3: IF UNSAT: construct conflict and other learned clauses
    //cout << "Foudn unsat\n" << endl;
    constructClauses(lits, b, res.count());
    conf = true;
  }
  else {
    //cout << "Foudn sat\n" << endl;
    conf = false;
  }
}



//returns false when learned is empty
void BoxSolver::getAddition(vec<Lit>& lits, bool& conf) {
  conf = false;
  //cout << "Conf is " << conf << endl;
  return;
  /*
  if (learned.size() == 0) { conf = false; return; }
  else {
    //Takes the top of learned
    //Places all of its elements in lits
    //This is necessary because vec's copy constructor is disabled
    conf = true;
    stack<Lit> toAdd = learned.top();
    if (toAdd.size() == 0) {conf = false; return;}
    learned.pop();
    while (toAdd.size() > 0) {
      Lit l = toAdd.top();
      toAdd.pop();
      lits.push(l);
    }
  }
  */
}

//makes a proper minisat lit from an index
inline Lit BoxSolver::litFromInt(int var, bool val) {
  var = abs(var);
  Lit l = mkLit(var, val);
  return l;
}


inline void BoxSolver::writeLearnedClause(vec<Lit>& lits) {
  for (int i = 0; i < lits.size(); i++) {
    TAtomRef t;
    bool x = IM->getAtom(lits[i], t);
    if (x){
      write(lits[i]); cerr << " maps to "; t->write(); cerr << endl;
    }
    else {
      write(lits[i]); cerr << " has no mapping \n";
    }

  }
}

void BoxSolver::constructClauses(vec<Lit>& lits, const Boxer& b, int numDeds) {
  //Traceback the last result (AKA, the conflict)
  //Place it into lits
  
  Result r = b.traceBack();
  //cout << "Conflict: "; r.write();
  //cout << "ASSIGNMENTS: "; printStack(); cout << endl;
  /*
    Result r = b.traceBack();
    vector<TAtomRef> atoms = r.atoms;
  cout << "CONFLICT   : ";
  for (int i = 0; i < atoms.size(); i++) {
    if (atoms[i]->relop != ALOP) {
      atoms[i]->write();
      cout << "(" << IM->getIdx(atoms[i]) <<  ")";
      if (i != atoms.size()-) cout << " /\\ ";
    }
  }
  cout << endl;
  */
  stack<Lit> litStack = mkClause(r);
  while (litStack.size() > 0) {

    lits.push(litStack.top());
    litStack.pop();
  }
  /*
  cout << "LEARNED    :";
  for (int i = 0; i < lits.size(); i++) {
    write(lits[i]); cout << " ";
  }
  cout << endl;
  */
  //writeLearnedClause(lits);
  //Traceback all other learned items
  //Place them into stack "learned"
  /*
  for (int i = b.size()-2; i >= b.size()-numDeds; i--) {
    int idx = IM->getIdx(b.getDed(i));
    if (idx == -1) {
      //cout << "REJECTING ADDITION\n";
      continue;
    }
    //cout << "ADDING AN ADDITION!\n";
    Result r = b.traceBack(i);
    stack<Lit> litStack = mkClause(r, i);
    learned.push(litStack);
  }
  */
}

inline stack<Lit> BoxSolver::mkClause(Result r, int idx) {
  stack<Lit> litStack;
  vector<TAtomRef> atoms = r.atoms;
  vector<TAtomRef>::iterator start, end;
  litStack.push(mkLit(idx, false));
  
  //cout << "LEARNED: ";
  for (start = atoms.begin(), end = atoms.end(); start != end; ++start) {
    TAtomRef t = *start;
    if (t->relop == ALOP) continue;
    //cout << "NOT"; t->write(); cout << " \\/ ";
    int idx = IM->getIdx(t);
    //cout << "the idx of "; t->write(); cout << " is " << idx << endl;
    Lit l = litFromInt(idx, true);
    litStack.push(l);
  }
  TAtomRef t;
  IM->getAtom(idx, t);
  //t->write(); cout << endl;
  //cout << endl;
  return litStack;
}

inline stack<Lit> BoxSolver::mkClause(Result r) {
  stack<Lit> litStack;
  vector<TAtomRef> atoms = r.atoms;
  vector<TAtomRef>::iterator start, end;
  //cout << "LEARNED: ";
  for (start = atoms.begin(), end = atoms.end(); start != end; ++start) {
    TAtomRef t = *start;
    if (t->relop == ALOP) continue;
    //t->write(); cout << " /\\ ";
    int idx = IM->getIdx(t);
    //cout << "the idx of "; t->write(); cout << " is " << idx << endl;
    Lit l = litFromInt(idx, true);
    litStack.push(l);
  }
  //cout << endl;
  return litStack;
}



/*
  Given two/three lits, makes a clause and adds it to the stack of clauses to be returned
*/
inline void BoxSolver::addToLearned(Lit a, Lit b) {
  stack<Lit> toPush;
  toPush.push(a);
  toPush.push(b);
  learned.push(toPush);
}

inline void BoxSolver::addToLearned(Lit a, Lit b, Lit c) {
  stack<Lit> toPush;
  toPush.push(a);
  toPush.push(b);
  toPush.push(c);
  learned.push(toPush);
}
