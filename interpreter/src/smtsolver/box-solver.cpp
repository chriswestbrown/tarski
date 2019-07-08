#include "solver-manager.h"
#include "bbwb.h"
#include "blackbox-solve.h"
#include "whitebox-solve.h"
#include "formula-maker.h"
#include "../formula/formmanip.h"
#include "idx-manager.h"
#include "normalize.h"
#include "../shell/qepcad-inter/qepcad-session.h"
#include "opennucad.h"
#include <algorithm>
#include <vector>
#include <string>
#include <fstream>

//So it turns out that the "using namespace" command is additive
using namespace Minisat;
using namespace tarski;
using namespace std;


/*
  L1 Normalizes the formula
  Then does splitting on all nonstrict atoms with which
  substitution can be applied

  Initializes the index manager on the split L1 normalized formula
  Constructs a Minisat Solver if the formula is NOT a pure conjuction
 */
BoxSolver::BoxSolver(TFormRef formula) : unsat(false), ranOnce(false), M(NULL), isPureConj(true),
					 numAtoms(0), limit(5000), count(0), numGetFinalClauseCalls(0), numDNFDisjuncts(0)
{
  Normalizer* p = new Level1();
  RawNormalizer R(*p);
  R(formula);
  delete p;
  TFormRef l1norm = R.getRes();

  IM = new IdxManager();
  pm = formula->getPolyManagerPtr();
  if (l1norm->getTFType() == TF_CONST) {
    TConstRef C = asa<TConstObj>(l1norm);
    if (C->constValue() == FALSE) {
      unsat = true;
      return;
    }
  }
  int s = 0;
  bool res = doSplit(l1norm, s);
  if (unsat) return;
  if (res) {
    TAndRef splitT = new TAndObj();
    processAtoms(l1norm, splitT);
    this->formula = splitT;
  }
  else {
    processAtoms(l1norm);
    this->formula = l1norm;
  }
  if (verbose)
  {
    cout << "The orig formula is " << toString(l1norm) << endl;
    cout << "Formula to solve is " << toString(this->formula) << endl;
  }
  numDNFDisjuncts = getDNFNumDisjuncts(this->formula);
  if (!isPureConj){
    S = new Solver(this);
    listVec l = makeFormula(this->formula);
    /*
    for (auto itr = l.begin(); itr != l.end(); ++itr) {
      for (int i = 0; i < itr->size(); i++) {
        write((*itr)[i]); cerr << " ";
      }
      cerr << endl;
    }
    */
    //    M = (numAtoms > 5) ? M = new MHSGenerator(l, numAtoms) : NULL;
    M = new MHSGenerator(l, numAtoms); //-- Dr Brown modified
    S->mkProblem(l.begin(), l.end());
  }
}

BoxSolver::~BoxSolver() {
  if (verbose) {
    cout << "BoxSolver: numGetFinalClauseCalls = " << numGetFinalClauseCalls << endl;
    cout << "BoxSolver: numDNFDisjuncts = " << numDNFDisjuncts << endl; }
  
  delete IM;
  if (!isPureConj) {
    delete S;
  }
  if (ranOnce)
    delete SM;
  if (M != NULL) delete M;
}


listVec BoxSolver::makeFormula(TFormRef formula) {
  FormulaMaker  f(formula, IM);
  listVec l = f.mkFormula();
  return l;
}

bool BoxSolver::solve(string& err) {
  if (isPureConj)
  {
    try { return directSolve(); }
    catch (TarskiException& e) {
      err = string(e.what()); return false;
    }
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
  {
    try { return directSolve(); }
    catch (TarskiException& e) { return 2; }
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

//Runs under the explicit assumtion that
//the formula is a pure conjunction!
bool BoxSolver::directSolve()
{
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
  if (t2->constValue() == 1) return true;

  //-- AT THIS POINT: t2 is a simplified equivalent formula, but we don't know SAT/UNSAT

  // strip out any atoms that we eliminated by substitutions
  TAndRef t3 = b.filterOut(t2);

  if (verbose)
  {
    cout << "Simplified formula is : ";
    t3->write(true);
    cout << endl;
  }

  // call a full solver to decide this problem
  if (classifyConj(t3) == 0) {
    OpenNuCADSATSolverRef nuCadSolver = new OpenNuCADSATSolverObj(t3);
    if (nuCadSolver->isSATFound())
      return true;
    else return false;
  }
  //writeSimpToFile(t, t3);
  TFormRef res;
  try  {
    QepcadConnection q;
    res = q.basicQepcadCall(exclose(t3), true);
  }
  catch (TarskiException& e) {
    throw TarskiException("QEPCAD failure on "  + toString(t3) +  \
                          ".\nErr: " + e.what());
  }
  if (res->constValue() == 0) return false;
  else return true;
}

/*
  True if we should do clause splitting
  false otherwise
 Returns false when the formula blow up is too large
  For now, it is when the resulting formula would be > 50 atoms
 */
bool BoxSolver::doSplit(TFormRef t, int& s)
{
  const int threshold = 50000000; //50000000; //fernando set to 50
  switch (t->getTFType()) {
  case TF_ATOM: {
      TAtomRef a = asa<TAtomObj>(t);
      if (a->getRelop() == LEOP || a->getRelop() == GEOP) {
        s += 2;
      }
      if (s >= threshold) return false;
      break;
  }
  case TF_OR: {
    TOrRef o = asa<TOrObj>(t);
    for (auto itr = o->begin(); itr != o->end(); ++itr) { 
      doSplit(*itr, s);
      if (s >= threshold) return false;
    }
    return true;
  }
  case TF_AND: {
    TAndRef a = asa<TAndObj>(t);
    for (auto itr = a->begin(); itr != a->end(); ++itr) {
      doSplit(*itr, s);
      if (s >= threshold) return false;
    }
    return true;
  }
  case TF_CONST:   {
    TConstRef C = asa<TConstObj>(formula);
    if (C->constValue() == FALSE) unsat = true;
    break;
  }
  default: {
      throw TarskiException("Unexpected non-atom/non-and/non-or");
    }
  }
  return (s >= threshold) ? false : true;
}

//if no splitting is done, the result is of size 1
//else, the result is of size 2, containing the split atoms
vector<TAtomRef> BoxSolver::splitAtom(TAtomRef t)
{
  vector<TAtomRef> res;
  if (t->getFactors()->numFactors() != 1 &&false)//DRBROWN ... disabling this test
  {
    res.push_back(t); return res;
  }

  if (!(t->getRelop() == LEOP || t->getRelop() == GEOP))
  {
    res.push_back(t); return res;
  }
  IntPolyRef p = t->factorsBegin()->first;
  if (p->numVars() > 2 && false)//DRBROWN ... disabling this to test
  {
    res.push_back(t); return res;
  }
  TAtomRef t1 = new TAtomObj(t->getFactors(), EQOP);
  TAtomRef t2 = new TAtomObj(t->getFactors(), t->getRelop() ^ EQOP);
  res.resize(2);
  res[0] = t1; res[1] = t2;
  return res;
}

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
  case TF_OR: {
    isPureConj = false; //-- Dr. Brown Added
    TOrRef o = asa<TOrObj>(formula);
    for (auto itr = o->begin(); itr != o->end(); ++itr) {
      processAtoms(*itr);
    }
    break;
  }
  case TF_AND: {
    TAndRef c = asa<TAndObj>(formula);
    for (auto itr = c->begin(); itr != c->end(); ++itr) {
      processAtoms(*itr);
    }
    break;
  }
  case TF_CONST:   {
    TConstRef C = asa<TConstObj>(formula);
    if (C->constValue() == FALSE) unsat = true;
    break;
  }
  }
}

//Accesses each atom recursively
//Assigns an idx to each atomref
void BoxSolver::processAtoms(TFormRef formula, TFormRef out) {
  switch (formula->getTFType()) {
  case TF_ATOM: {
    TAtomRef a = asa<TAtomObj>(formula);
    vector<TAtomRef> res = splitAtom(a);
    if (res.size() == 1) {
      if (IM->getIdx(a) == -1) {
        IM->mkIdx(a);
        numAtoms++;
      }
      if (out->getTFType() == TF_AND){
        TAndRef out2 = asa<TAndObj>(out);
        out2->AND(a);
      }
      else {
        TOrRef out2 = asa<TOrObj>(out);
        out2->OR(a);
      }
    }
    else if (res.size() == 2) {
      if (IM->getIdx(res[0]) == -1) {
        IM->mkIdx(res[0]);
        numAtoms++;
      }
      if (IM->getIdx(res[1]) == -1) {
        IM->mkIdx(res[1]);
        numAtoms++;
      }
      if (out->getTFType() == TF_AND) {
        TAndRef out2 = asa<TAndObj>(out);
        TOrRef split = new TOrObj();
        split->OR(res[0]);
        split->OR(res[1]);
        out2->AND(split);
      }
      else {
        TOrRef out2 = asa<TOrObj>(out);
        out2->OR(res[0]);
        out2->OR(res[1]);
      }
      isPureConj = false;
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
    TAndRef res = new TAndObj();
    TAndRef C = asa<TAndObj>(formula);
    for(auto it = C->begin(); it != C->end(); ++it ) {
      if ((*it)->getTFType() == TF_OR) {
        TOrRef tmp = asa<TOrObj>(*it);
        //to remove double brackets - jump to the inner level of brackets (AKA, when [[atom]] appears)
        if (tmp->size() == 1) {
          processAtoms(*(tmp->begin()), res);
        }
        else {
          processAtoms(*it, res);
        }
      }
      else if ((*it)->getTFType() == TF_AND) {
        TAndRef tmp = asa<TAndObj>(*it);
        if (tmp->size() == 1) {
          processAtoms(*(tmp->begin()), res);
        }
        else {
          processAtoms(*it, res);
        }
      }
      else
        processAtoms(*it, res);

    }
    if (out->getTFType() == TF_AND){
      TAndRef out2 = asa<TAndObj>(out);
      out2->AND(res);
    }
    else {
      TOrRef out2 = asa<TOrObj>(out);
      out2->OR(res);
    }
    break;
  }

  case TF_QB: throw TarskiException("Unexpected Quantifier!");

  case TF_OR: {
    //recursively access each element here of the OR block
    TOrRef C = asa<TOrObj>(formula);
    TOrRef res = new TOrObj();
    for (auto it = C->begin(); it != C->end(); ++it ) {
      if ((*it)->getTFType() == TF_OR) {
        TOrRef tmp = asa<TOrObj>(*it);
        if (tmp->size() == 1) {
          processAtoms(*(tmp->begin()), res);
        }
        else {
          processAtoms(*it, res);
        }
      }
      else if ((*it)->getTFType() == TF_AND) {
        TAndRef tmp = asa<TAndObj>(*it);
        if (tmp->size() == 1) {
          processAtoms(*(tmp->begin()), res);
        }
        else {
          processAtoms(*it, res);
        }
      }
      else
        processAtoms(*it, res);
    }
    if (out->getTFType() == TF_AND){
      TAndRef out2 = asa<TAndObj>(out);
      out2->AND(res);
    }
    else {
      TOrRef out2 = asa<TOrObj>(out);
      out2->OR(res);
    }
    isPureConj = false;
    break;
  }
  default:
    throw TarskiException("Unexpected case in processAtoms!");
    return;
  }
}

//Assumes pure conjunction!
//0 - OpenNUCAD is a perfect solver
//1 - OpenNUCAD can determine SAT but not UNSAT
int BoxSolver::classifyConj(TAndRef T) {
  for (auto itr = T->begin(); itr != T->end(); ++itr) {
    TAtomRef t = asa<TAtomObj>(*itr);
    if ((t->getRelop() & EQOP) == EQOP) {
      return 1;
    }
  }
  return 0;
}


TAndRef BoxSolver::genMHS() {
  TAndRef tand = new TAndObj();
  vector<Lit> res = M->genMHS(getTrail());
  for (auto itr = res.begin(); itr != res.end(); ++itr) {
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

// This is called by MiniSat when it has a full propositional solution, which we
// Then must check for satisfiablilty w.r.t. the theory.
void BoxSolver::getFinalClause(vec<Lit>& lits, bool& conf)
{

  if (verbose) { cout << "S||"; this->printStack(); cout << endl; }
  
  numGetFinalClauseCalls++;
  conf = false;
  if (M != NULL) {
    if (ranOnce) delete SM;
    TAndRef t = genMHS();
    if (verbose) { cout << endl << "box-solve: Tarski formula produced from sufficient assignment is "
			<< toString(t) << endl; }

    SM = new SolverManager(SolverManager::BB |
                           SolverManager::WB |
                           SolverManager::SS, t);

    Result res = SM->deduceAll();

    if (SM->isUnsat()) {
      if (verbose) {
	cout << "Unsat! core : ";
	res.write();
	cout << endl;
      }
      constructClauses(lits, res);
      conf = true;
      return;
    }
  }
  else {
    getClauseMain(lits, conf);
  }
  //Only if BB/WB cannot detect unsat do we need to call QEPCAD"
  if (conf == false) {
    QepcadConnection q;
    TAndRef tsimp = SM->simplify();

    // strip out any atoms that we eliminated by substitutions
    TAndRef tand = SM->filterOut(tsimp);

    
    if (verbose) { cout << endl << "box-solve: simplified to " << toString(tand) << endl; }


    if (tand->constValue() == 1) return;

    //NUCAD
    //Only for pure strict conjunctions
    //Does nothing if Nucad discovers UNSAT, as it doesn't produce UNSAT Cores
    //yet
    if (classifyConj(tand) == 0) {
      OpenNuCADSATSolverRef nuCadSolver = new OpenNuCADSATSolverObj(tand);
      if (nuCadSolver->isSATFound())
      {
	if (verbose) { cout << "Open-NuCAD found SAT!" << endl; }
        return;
      }
      else {
        conf = true;
        TAndRef t = nuCadSolver->getUNSATCore();
	if (verbose) { cout << "Open-NuCAD found UNSAT! core : "; t->write(true); cout << endl; }
        std::set<TAtomRef> allAtoms;
        for (auto itr = t->begin(); itr != t->end(); ++itr) {
          TAtomRef A = asa<TAtomObj>(*itr);
          Result r = SM->explainAtom(A);
          for (auto atom = r.atoms.begin(); atom != r.atoms.end(); ++atom) {
            if (allAtoms.find(*atom) == allAtoms.end()) {
              allAtoms.insert(*atom);
              int varNum = IM->getIdx(*atom);
              Lit l = litFromInt(varNum, true);
              lits.push(l);
            }
          }
        }
        return;
      }
    }
    //END NUCAD, START QEPCAD
    TFormRef res;
    try {
      res = q.basicQepcadCall(exclose(tand), true);
    }
    catch (TarskiException& e) {
      throw TarskiException("QEPCAD failure on "  + toString(tand) +  \
                            ".\nErr: " + e.what());
    }
    std::set<TAtomRef> allAtoms;
    if (res->constValue() == 0) {
      conf = true;
      vector<int> core = q.getUnsatCore();
      sort(core.begin(), core.end());
      TAndRef F_core= new TAndObj();
      int currAtom = 0, curr = 0;
      for (auto begin = tand->begin(); begin != tand->end(); ++begin) {
        if (core[curr] == currAtom) {
          curr++;
          TAtomRef A = asa<TAtomObj>(*begin);
	  F_core->AND(A);
          Result r = SM->explainAtom(A);
          for (auto atom = r.atoms.begin(); atom != r.atoms.end(); ++atom) {
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
      if (verbose) { cout << "QEPCADB found UNSAT! core : "; F_core->write(true); cout << endl; }
    }
  }
}


/*
  DEPRECATED
*/
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
    //cout << "Formula: " << toString(tand) << endl;
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
    if (verbose) {
      cout << "Unsat in getClauseMain! core : ";
      res.write();
      cout << endl;
    }
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
//var - the index value of the atom
//val - true, which is lFalse, or false, which is lTrue
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
  vector<TAtomRef> atoms = r.atoms;

  if (true) {
    cout << "MAPPING: ";
    printMapping(); cout << endl;
  cout << "ASSIGNMENTS: "; printStack(); cout << endl;

  cerr << "CONFLICT   : ";
  for (int i = 0; i < atoms.size(); i++) {
    if (atoms[i]->relop != ALOP) {
      atoms[i]->write();
      cerr << "(" << IM->getIdx(atoms[i]) <<  ")";
      if (i != atoms.size()-1) cerr << " /\\ ";
    }
  }
  cerr << endl;
  }
  for (auto itr = atoms.begin(); itr != atoms.end(); ++itr) {
    TAtomRef t = *itr;
    if (t->getRelop() == ALOP) continue;
    int idx = IM->getIdx(t);
    Lit l = litFromInt(idx, true);
    lits.push(l);
  }

  if (true){
  cout << "LEARNED    :";
  for (int i = 0; i < lits.size(); i++) {
    write(lits[i]); cout << " ";
  }
  cout << endl;
  writeLearnedClause(lits);
  }
  
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
