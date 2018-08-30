#include "solver-manager.h"
#include "blackbox-solve.h"
#include "whitebox-solve.h"
#include "formula-maker.h"
#include "../formula/formmanip.h"
#include "idx-manager.h"
#include "normalize.h"
#include "../shell/qepcad-inter/qepcad-session.h"
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>

//So it turns out that the "using namespace" command is additive
using namespace Minisat;
using namespace tarski;
using namespace std;


//TODO: Refactor SMTSolver to do more efficient SAT problem generation by using new function call


inline bool intSort (int i, int j) { return (i < j);}

BoxSolver::BoxSolver(TFormRef formula) :  isPureConj(true),  numAtoms(-1), limit(5), count(0), lastVec(0), ranOnce(false), unsat(false) {
  Normalizer* p = new Level1();
  RawNormalizer R(*p);
  R(formula);
  delete p;
  this->formula = R.getRes();

  IM = new IdxManager();
  pm = formula->getPolyManagerPtr();
  processAtoms(this->formula);
  if (unsat) return;
  if (!isPureConj){
    S = new Solver(this);
    S->mkProblem(makeFormula(this->formula));
    M = (numAtoms > 5) ? new MHSGenerator(form, numAtoms) : NULL;
  }
}
BoxSolver::~BoxSolver() {
  delete IM;
  if (!isPureConj) {
    delete S;
    if (numAtoms > 5) delete M;
  }
  if (ranOnce)
    delete SM;
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
  if (unsat) return 0;
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
  return (ret == l_True) ? true : false;
}

bool BoxSolver::directSolve() {
  //std::cerr << "Direct solving formula\n";
  if (unsat) return false;
  TAndRef t = asa<TAndObj>(formula);
  TAndRef t2;
  if (t->constValue() == true) return true;
  SolverManager b( SolverManager::BB |
                   SolverManager::WB |
                   SolverManager::SS, t);
  b.deduceAll();
  if (b.isUnsat()) return false;
  else t2 = b.simplify();
  //writeSimpToFile(t, t2);
  TFormRef res;
  try  {
    QepcadConnection q;
    res = q.basicQepcadCall(exclose(t2), true);
  }
  catch (TarskiException& e) {
    throw TarskiException("QEPCAD timed out" + toString(t2));

  }
  if (res->constValue() == 0) return false;
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
  case TF_CONST:   {
    TConstRef C = asa<TConstObj>(formula);
    if (C->constValue() == FALSE) unsat = true;
    break;
  }
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
  default:
    cout << "DEFAULT CASE!!\n";
    unsat = true;
    return;
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
    //TAndRef told = (M != NULL) ? genMHS() : genTAnd(getQhead());
    TAndRef tand = SM->simplify();
    //cout << "Old formula: " << toString(told) << "\nSimplified formula: " << toString(tand) << endl;
    //writeSimpToFile(told, tand);
    if (tand->constValue() == 1) return;
    TFormRef res;
    try {
      res = q.basicQepcadCall(exclose(tand), true);
    }
    catch (TarskiException& e) {
      throw TarskiException("QEPCAD timed out" + toString(tand));
    }
    std::set<TAtomRef> allAtoms;
    if (res->constValue() == 0) {
      conf = true;
      vector<int> core = q.getUnsatCore();
      //NOTE: Core is not guaranteed to come out in sorted order!!!
      //cout << "CORE: ";
      sort(core.begin(), core.end());
      int currAtom = 0, curr = 0;
      for (set<TFormRef, ConjunctOrder>::iterator
             begin = tand->begin(), end = tand->end();
           begin != end; ++begin) {
        if (core[curr] == currAtom) {
          curr++;
          TAtomRef A = asa<TAtomObj>(*begin);
          Result r = SM->explainAtom(A);
          for (std::vector<TAtomRef>::iterator atom = r.atoms.begin();
               atom != r.atoms.end(); ++atom) {
            if (allAtoms.find(*atom) == allAtoms.end()) {
              allAtoms.insert(*atom);
              int varNum = IM->getIdx(*atom);
              Lit l = litFromInt(varNum, true);
              lits.push(l);
            }
          }
          if (curr >= core.size()) break;
        }
        currAtom++;
      }
      /*
        QepcadConnection q2;
        cout << "COREOLD: ";
        res = q2.basicQepcadCall(exclose(told), true);
        vector<int> core2 = q.getUnsatCore();
        sort(core2.begin(), core2.end());
        getQEPUnsatCore(lits, core2, told);
      */
    }
  }
}


void BoxSolver::getQEPUnsatCore(vec<Lit>& lits, vector<int> indices, TAndRef tand) {
  int curr = 0;
  unsigned int currVec = 0;
  for (set<TFormRef, ConjunctOrder>::iterator begin = tand->begin(), end = tand->end();
       begin != end; ++begin) {
    if (indices[currVec] == curr) {
      TAtomRef a = asa<TAtomObj>(*begin);
      if (a.is_null()) throw TarskiException("Unexpected non-atom in getQEPUnsatCore");
      int varNum = IM->getIdx(a);
      Lit l = litFromInt(varNum, true);
      lits.push(l);
      currVec++;
      if (currVec >= indices.size()) break;
    }
    curr++;
  }
}

void BoxSolver::writeSimpToFile(TAndRef orig, TAndRef simp) {
  fstream file("simp.txt", fstream::app | fstream::out);
  file << "Original: " + toString(orig) +
    " Simplified: " + toString(simp) << "\n";
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


bool BoxSolver::atomFromLit(Lit p, TAtomRef& t) {
  if (sign(p) == true) {
    return false;
  }
  int v = var(p);
  return IM->getAtom(v, t);
}

TAndRef BoxSolver::genTAnd(int maxIdx) {
  TAndRef tand = new TAndObj();
  const vec<Lit>& trail = getTrail();
  for (int i = 0; i < maxIdx; i++) {
    TAtomRef t;
    if (atomFromLit(trail[i], t)) {
      tand->AND(t);
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


/*
  returns true if nuVec = lastVec + some new assumptions
  returns false otherwise
  Sets lastVec = nuVec in both cases
  in other words, checks if MINISAT made any retractions
 */
bool BoxSolver::compareVecs(vec<Lit>& nuVec) {
  bool retVal = true;
  if (nuVec.size() < lastVec.size() || lastVec.size() == 0) {
    retVal = false;
  }
  else {
    for (int i = 0; i < lastVec.size(); i++) {
      if (lastVec[i] != nuVec[i]) {
        retVal = false;
        break;
      }
    }
  }
  lastVec.clear();
  for (int i = 0; i < nuVec.size(); i++) {
    lastVec[i] = nuVec[i];
  }
  return retVal;
}

void BoxSolver::getClauseMain(vec<Lit>& lits, bool& conf) {
  lits.clear();
  int i = lastVec.size();
  if (compareVecs(lits)) {
    vector<TAtomRef > nuAtoms;
    for (; i < lits.size(); i++) {
      TAtomRef t;
      if (atomFromLit(lits[i], t)) {
        nuAtoms.push_back(t);
      }
    }
    SM->updateSolver(nuAtoms);

  }

  else {
    TAndRef tand = genTAnd(getQhead());
    if (tand->constValue() == TRUE) {
      conf = false;
      return;
    }
    //Step 2: Construct a BB/WB Solver bbwb with tand and solve
    if (ranOnce)
      delete SM;
    else ranOnce = true;
    SM = new SolverManager( SolverManager::BB |
                            SolverManager::WB |
                            SolverManager::SS, tand);
  }
  Result res = SM->deduceAll();
  if (SM->isUnsat()) {
    constructClauses(lits, res);
    conf = true;
  }
  else {
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
      Lit l = toAdd.top;
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

void BoxSolver::constructClauses(vec<Lit>& lits, Result& r) {
  //Traceback the last result (AKA, the conflict)
  //Place it into lits
  
  /*
  cout << "Conflict: "; r.write();
  cout << "ASSIGNMENTS: "; printStack(); cout << endl;
  vector<TAtomRef> atoms = r.atoms;
  cout << "CONFLICT   : ";
  for (int i = 0; i < atoms.size(); i++) {
    if (atoms[i]->relop != ALOP) {
      atoms[i]->write();
      cout << "(" << IM->getIdx(atoms[i]) <<  ")";
      if (i != atoms.size()-1) cout << " /\\ ";
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
