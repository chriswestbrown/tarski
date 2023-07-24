#include "solver-manager.h"
#include "blackbox-solve.h"
#include "whitebox-solve.h"
#include "clearAssignments.h"

namespace tarski {

  SolverManager::SolverManager(int codes, TAndRef tand) : hasRan(false), hasSimplified(false) {
    if (true) {
      cout << "Solver manager instantiated on ";
      tand->write(true);
      cout << endl;
    }
    dedM = new DedManager(tand);
    if (dedM->isUnsat()) return;
    t = dedM->getInitConjunct();
    if ((codes & BB) == BB) solvers.push_back(new BBSolver(t));
    if ((codes & WB) == WB) solvers.push_back(new WBSolver(t));
    if ((codes & SS) == SS) solvers.push_back(new Substituter(t));
    for (size_t i = 0; i < solvers.size(); i++) {
      solvers[i]->setDedM(dedM);
    }
    lastDeds.resize(solvers.size(), 0);
  }


  /*
    A useful method for return a Tarski object which contains the results of deduceAll
   */
  LisRef SolverManager::genLisResult() {
      Result r = deduceAll();
      prettyPrintResult(); //-- CHRIS DEBUG
      LisRef l = new LisObj();
      if (isUnsat()) {

	//-- CHRIS DEBUG
	if (true) {
	  if (solvers.size() == 0)
	    cout << "Solvers never instantiated, no sorted print!" << endl;
	  else
	    dedM->debugWriteSorted(*this);
	}
	  
        l->push_back(new SymObj("UNSAT"));
        vector<TAtomRef>& vec = r.atoms;
        TAndRef res = new TAndObj();
        for (vector<TAtomRef>::iterator itr = vec.begin();
             itr != vec.end(); ++itr) {
          res->AND(*itr);
        }
        l->push_back(new TarObj(res));
      }
      else  {
        l->push_back(new SymObj("SAT"));
        TAndRef t = simplify();
        l->push_back(new TarObj(t));
      }
      return l;
    }


  //********************** ORDERER STUFF *************************/


    /*
    Heuristic for simplest is
    Score 0 for best possible case, strict sign on variable
    Add 1 if sign can somehow be strengthened (AKA, LEOP, GEOP, NEOP)
    Add 6 for every additional term in a polynomial
    Add 2 for every variable in a term
    Add 12 for every factor
   */
  int scoreDed(const Deduction& d) {
    int score = 0;
    TAtomRef a = d.getDed();
    FactRef f = a->getFactors();
    if (a->getRelop() == LEOP || a->getRelop() == GEOP ||
        a->getRelop() == NEOP) {
      score += 1;
    }
    score += 12 * (f->numFactors() - 1);
    for (FactObj::factorIterator itr = f->factorBegin();
         itr != f->factorEnd(); ++itr) {
      IntPolyRef poly = itr->first;
      if (poly->isVar()) continue;
      VarKeyedMap<int> M;
      FernPolyIter F(poly, M);
      while (!F.isNull()) {
        score += 2 + (F.getVars().size()-1);
        F.next();
        if (!F.isNull()) score += 6;
      }
    }
    return score;
  }


  class SpecialPair : public std::pair<int,int>
  {
  public:
    SpecialPair(int a, int b) : std::pair<int,int>(a,b) { }
    bool operator<(const SpecialPair &A) { return first < A.first || first == A.first && second < A.second; }
  };
  
  std::vector<size_t> SolverManager::proxy_sorted_indices(std::vector<Deduction> &deds)
  {
    
    Substituter* sp = NULL;
    for(size_t i = 0; i < solvers.size() && sp == NULL; ++i)
      sp = dynamic_cast<Substituter*>(solvers[i]);
    
    vector<size_t> indices;
    {
      vector< SpecialPair > vScores(deds.size(), SpecialPair(0,0));
      for (size_t i = 0; i < deds.size(); i++)
      {
	if (deds[i].isUnsat()) //-- if we deduce UNSAT, that deduction should always be ordered last
	{
	  vScores[i] = SpecialPair(INT_MAX,INT_MAX);
	  continue;
	}
	VarSet S = deds[i].getDed()->getVars();
	int maxL = 0;
	for(auto itr = S.begin(); itr != S.end(); ++itr)
	  maxL = std::max(maxL,sp == NULL ? 0 : sp->getSubstitutionLevel(*itr));
	vScores[i] = SpecialPair(maxL,scoreDed(deds[i]));
      }
      indices = sort_indices(vScores);
    }
    return indices;
  }
  
  //***************************************************************/
  


  
  /*
    Assumes deduceAll has already been called
   */
  TAndRef SolverManager::simplify() {
    if (!hasSimplified) {
      deduceOrig();
      simp = dedM->getSimplifiedFormula(*this);
      hasSimplified = true;
      return simp;
    }
    return simp;
  }

  /*
    The main body of the solver manager
    Calles deduceLoop to actually make the deduction by each solver

    Remembers the last solver to learn something new. Halts when it is that
    solver's turn again but nothing new has been learned

    Also halts immediately when UNSAT is deduced
   */
  Result SolverManager::deduceAll() {
    if (hasRan) return finResult;
    hasRan = true;
    if (dedM->isUnsat()) { finResult = dedM->traceBack(); return finResult; }
    for (int i = 0; i < lastDeds.size(); i++) lastDeds[i] = dedM->size();
    int i = 0, lastChange = 0, N = solvers.size();
    while (true)
    {
      //case where solvers can't deduce UNSAT and all deductions exhausted
      if (i - lastChange == N && solvers[i % N]->isIdempotent() ||  i - lastChange > N)
	return (finResult = Result());

      // cerr << "i = " << i << endl;

      switch(deduceLoop(i % N))
      {
      case 1: lastChange = i; break;
      case 2: return (finResult = dedM->traceBack()); break;
      default: break;
      }
      i++;
    }
  }
  
  /*
    Updates the solver by getting iterators from the deduction manager
    from the last new index of the solver to the end of all known deductions
   */
  void SolverManager::updateSolver(int i) {
    vector<Deduction>::const_iterator itr, end;
    dedM->getItrs(lastDeds[i], itr, end);
    solvers[i]->update(itr, end);
  }

  /*
    
   */
  void SolverManager::updateSolver(const vector<TAtomRef>& v) {

    if (dedM->isUnsat()) { return; }
    hasRan = false;
    int oldLast = dedM->size();
    for (std::vector<TAtomRef>::const_iterator itr = v.begin();
           itr != v.end(); ++itr) {
      dedM->addGiven(*itr);
    }
    for (int i  = 0; i < solvers.size(); i++) {
      updateSolver(i);
    }
  }
  /*
    return 0 to indicate the solver learned nothing
    return 1 to indicate a solver learned something (but not UNSAT)
    return 2 to indicate the solver deduced UNSAT
   */
  short SolverManager::deduceLoop(int i) {
    short retCode = 0;
     cerr << endl << endl;
     dedM->writeAll();
     cerr << endl << endl;
     cerr << "Calling solvers[" << i << "]!!! (" << solvers[i]->name() << ")" << endl;
    QuickSolver * q = solvers[i];

    // HACK! This is a hack to simulate no incrementality
    if (solvers[i]->name() == "BBSolver") { delete q; q = solvers[i] = new BBSolver(t); solvers[i]->setDedM(dedM); }
    else if (solvers[i]->name() == "WBSolver") { delete q; q = solvers[i] = new WBSolver(t); solvers[i]->setDedM(dedM); }
    else
      updateSolver(i);

    bool res = true;
    DedExp d = q->deduce(t, res);
    while (res && !dedM->isUnsat()) {
      if (dedM->processDeduction(d.d, d.exp)) {
        if (dedM->isUnsat()) return 2;
        retCode = 1;
        t->AND(d.d.getDed());
        q->notify();
      }
      d = q->deduce(t, res);
    }
    if (dedM->isUnsat()) return 2;
    lastDeds[i] = dedM->size();
    return retCode;
  }

  void SolverManager::deduceOrig() {
    DedManager::dedItr beg, end;
    dedM->getOItrs(beg, end);
    for (auto& s : solvers) {
      DedManager::dedItr itr = beg;
      list<DedExp> v = s->deduceTarget(itr, end);
      for (auto& d : v) dedM->processDeduction(d.d, d.exp);
    }
  }

  /*
    Pretty print a result in human readable format
    Generates a proof and a list of all deductions in numbered lists
    Throws an exception if called before deduceAll
   */
  void SolverManager::prettyPrintResult() {
    if (!hasRan) throw TarskiException("DeduceAll not yet called, but prettyPrintResult called!");
    std::cout << "##################################################" << std::endl;
    if (isUnsat()) {
      Result res = finResult;
      std::cout << "UNSAT\n";
      std::cout << "Explanation: "; res.write(); std::cout << std::endl << std::endl;
      std::cout << "Proof: \n";
      dedM->writeProof();
      std::cout << std::endl;
      std::cout << "All Deductions: \n";
      dedM->writeAll();
    }
    else {
      TAndRef t = simplify();
      std::cout << "UNKNOWN\nSimplified Formula: " + toString(t) + "\n";
      prettyPrintSimplify(t);

      std::cout << "All Deductions: \n";
      dedM->writeAll();
    }
    std::cout << "##################################################" << std::endl;  
  }

  /*
    Assumes simplification has already been done
   */
  void SolverManager::prettyPrintSimplify(TAndRef a) {
    for (TAndObj::conjunct_iterator itr = a->begin(), end = a->end(); itr != end; ++itr) {
      TAtomRef t = asa<TAtomObj>(*itr);
      Result r = explainAtom(t);
      if (r.atoms.begin() == r.atoms.end()) {
        cout << toString(t) << ": ERROR\n";
      }
      else {
        cout << toString(t) << ": ";
        for (std::vector<TAtomRef>::iterator itr = r.atoms.begin();
             itr != r.atoms.end(); ++itr) {
          cout << toString(*itr); if (itr+1 != r.atoms.end()) cout << " /\\ ";
        }
        cout << endl;
      }
    }
  }
 
  SolverManager::~SolverManager() {
    for (std::vector<QuickSolver * >::iterator itr = solvers.begin();
         itr != solvers.end(); ++itr) {
      delete(*itr);
    }
    solvers.clear();
    delete dedM;
  }

  TAndRef SolverManager::filterOut(TAndRef t)
  {
    Substituter* S = dynamic_cast<Substituter*>(solvers[2]);
    if (S == NULL)
      throw TarskiException("Assumption in solver-manager failed!");
    return S->filter(t);
  }


}//end namespace tarski
