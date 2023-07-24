#include "deduction.h"
#include "minwtbasis.h"
#include "dmatrix.h"
#include "../formula/monomialinequality.h"
#include "../formula/formula.h"
#include "../poly/factor.h"
#include "formrepconventions.h"
#include <set>
#include <ctime>
#include <string>
#include "blackbox-solve.h"

namespace tarski {

  DedExp BBSolver::deduce(TAndRef t, bool& res) {
    PM = t->getPolyManagerPtr();
    if (deductions.empty()) {
      if (!once)  {
        once = true;
        deductions.clear();
        res = false;
        DedExp d;
        return d;
      }
      else {
	cerr << "BEFORE bbsat: "; M.write(); cerr << endl;
	deductions = bbsat(t);
	cerr << "AFTER  bbsat: "; M.write(); cerr << endl;
        if (deductions.empty())  {
          res = false;
          DedExp d;
          return d;
        }
        else once = false;
      }
    }

    DedExp d = deductions.front();
    deductions.pop_front();
    return d;
  }

  /**
   * This gets called prior to the solver being called, in order to inform it of any new facts
   * that were learned since the last call.
   **/
  void BBSolver::update(std::vector<Deduction>::const_iterator begin, std::vector<Deduction>::const_iterator end)
  {
    // NOTE! This is broken!  This issue is currently sidestepped by having solver recreate the
    //       BBSolver from scratch each time, which obviates the problem 
    while (begin != end) {
      //ignore constants!
      if (begin->getDed()->getFactors()->numFactors() != 0)
        if (begin->getDed()->getFactors()->numFactors() != 1 ||
            !begin->getDed()->getFactors()->factorBegin()->first->isConstant()) {
          M.addAtom(begin->getDed());
        }
      ++begin;
    }
  }


  /*
    BBSat performs the bbsat algorithm
    If the function determines UNSAT, then BBSAT returns the reason
    in the form of a vector of deductions
    Otherwise, the function attempts to make deductions on the formula
    with the Blackbox algorithm
  */
  bblist BBSolver::bbsat(TAndRef t) {
    // Process the formula
    BBChecker bc(&M, PM);
    if (!bc.checkSat()) {
      return bc.explainUnsat();
    }

    else {
      BBDeducer bd(&M, PM);
      return bd.getDeductions();
    }
  }

  //returns the index of the UNSAT row
  //returns -1 if no such row exists

  int BBSolver::findRow(const DMatrix& d) {
    const std::vector<std::vector<char> >& matrix = d.getMatrix();
    for (size_t i = 0; i < matrix.size(); i++ ) {
      const std::vector<char>& row = matrix.at(i);
      if (row.at(0) == false) continue;
      bool isSat = false;
      for (size_t j = 1; j < row.size(); j++) 
        if (row.at(j) == true ) { isSat = true; break; }
      if (!isSat) { return i; }
    }
    return -1;
  }

  list<DedExp> BBSolver::deduceTarget(std::vector<Deduction>::const_iterator begin, std::vector<Deduction>::const_iterator end) {
    BBDeducer B(&M, PM);
    return B.attemptDeduce(begin, end);
  }


  //returns false if unsat
  bool BBChecker::checkSat() {
    //cerr << "BEFORE:" << endl; M->write();
    M->strictElim();
    //cerr << "AFTER:" << endl; M->write();
    const DMatrix& d = M->getStrict();
    unsatRow = BBSolver::findRow(d);
    return (unsatRow == -1) ? true : false;
  }

  /*
    If bbsat finds UNSAT, this returns the explanation in the form of a vector of BBDeds with a single BBDed which is the explanation
  */
  bblist BBChecker::explainUnsat() {
    if (false)
    {
      std::cerr << "Unsat row is " << unsatRow << std::endl;
      M->getAtom(unsatRow,true)->write(true); cerr << endl;
      M->write();
    }
    
    std::forward_list<TAtomRef> conflict = getConflict();
    std::set<IntPolyRef> weakFacts = findWeak(conflict);
    std::set<TAtomRef> additions = strengthenWeak(weakFacts);
    conflict.insert_after(conflict.begin(), additions.begin(), additions.end());
    DedExp d(Deduction::BBSTR, conflict);
    return {d};
  }

  /*
    Gets all the atoms involved in the conflict by using reasons (the indices in the matrix)
    and traceRow (which maps the new rows to original rows) to find the corresponding atom
    in the MonoIneqRep
  */
  std::forward_list<TAtomRef> BBChecker::getConflict() {
    std::forward_list<TAtomRef> conflict;
    const std::vector<bool>& exp = M->getStrict().getComp().at(unsatRow);
    for (size_t i = 0; i < exp.size(); i++) {
      if (exp.at(i) == true) {
        conflict.push_front(M->getAtom(i, true));
      }
    }
    return conflict;
  }

  /* determines whether each factor in MIR is weak or strong and classifies them appropriately
     Adds all the atoms involved in the conflict to a std::vector called conflict
     Output:
     weakFacts contains all the weak factors in the formula which need to be strengthened
     strongFacts contains all the strong factors in the formula
     conflict contains all the atoms which are in the conflict.
  */
  std::set<IntPolyRef> BBChecker::findWeak(std::forward_list<TAtomRef>& conflict ) {
    std::set<IntPolyRef> strongFacts;
    std::set<IntPolyRef> weakFacts;
    for (auto itr = conflict.begin(); itr != conflict.end(); ++itr ) {
      TAtomRef A = *itr;
      if (A->getRelop() == LTOP || A->getRelop() == GTOP ||
          A->getRelop() == NEOP) {
        for(auto fitr = A->factorsBegin(); fitr != A->factorsEnd(); ++fitr) {
          strongFacts.insert(fitr->first);
        }
      }
      else {
        for(auto fitr = A->factorsBegin(); fitr != A->factorsEnd(); ++fitr) {
          weakFacts.insert(fitr->first);
        }
      }
    }
    for (auto itr = strongFacts.begin(), end = strongFacts.end(); itr != end; ++itr) {
      weakFacts.erase(*itr);
    }
    return weakFacts;
  }

  /*
    Given a set of intpolyrefs which represent the "weak" factors in some explanation, generate a set of
    atoms which strengthen that intpolyref
  */
  std::set<TAtomRef> strengthenWeakM(const std::set<IntPolyRef>& weakFacts, MatrixManager* M)
  {
    set<TAtomRef> sAtoms;
    for (auto itr = weakFacts.begin(); itr != weakFacts.end(); ++itr){
      TAtomRef t = M->strengthenPoly(*itr);
      sAtoms.insert(t);
    }
    return sAtoms;
  }

  std::set<TAtomRef> BBChecker::strengthenWeak(const std::set<IntPolyRef>& weakFacts)
  {
    return strengthenWeakM(weakFacts,M);
  }



  void BBChecker::printAfterGauss() {
    std::cout << "After gauss:" << std::endl;
    std::cout << "M: " << std::endl; M->write();
  }


  /**
     return ku such that if row has exactly one non-zero factor-entry, 
     ku is the column index of that entry, if there are no non-zero
     factor entries, ku is zero, otherwise ku is -1.
   **/
  int BBDeducer::getNonZero(const std::vector<char>& row) {
    int ku = 0;
    for (size_t i = 1; i < row.size() && ku >= 0; i++)
      if (row.at(i))
	ku = (ku == 0 ? i : -1);
    return ku;
  }


  /*
    This function actually makes the deductions returned by getDeductions 
    for the strict part of the matrix
  */
  void BBDeducer::strictDeds(bblist& deds)
  {
    // Here, we iterate through everything that we learned by doing gaussian
    // elimination (AKA, all rows which were of the form X, 0, 0, ..., 1, 0, ..., 0
    // which were not of that form before gaussian elimination)
    const std::vector<std::vector<char> >& matrix = M->getStrict().getMatrix();
    const std::vector<std::vector<bool> >& comp = M->getStrict().getComp();
    for (size_t i = 0; i < matrix.size(); i++)
    {
      // We only consider "deductions" to be cases where the row has a unique
      // non-zero entry - i.e. it represents a known sign on a factor.
      // Note: dr brown - I still think we need to add deductions from rows that
      //       get zero'd out!
      short idx = getNonZero(matrix.at(i));
      if (idx == -1) continue;

      TAtomRef dedAtom; //-- deduced atom
      std::forward_list<TAtomRef> atoms; // atoms we use to deduce dedAtom
      TAtomRef strengthener = NULL; // atom that strengthens dedAtom to strict (if needed)
      if (idx == 0)
      {
	//Iterating through dependencies
	const std::vector<bool>& deps = comp.at(i);
	for (size_t a = 0; a < deps.size(); a++)
	{
	  if (deps[a] == 0) continue; 
	  TAtomRef A = M->getAtom(a, true);
	  atoms.push_front(A);
	}
      }
      else
      {
	IntPolyRef dedP = M->getPoly(idx);
	short dedSign = (matrix.at(i).at(0) == 0 ? GTOP : LTOP);
	FactRef F = new FactObj(PM);
	F->addFactor(dedP, 1);
	dedAtom = new TAtomObj(F, dedSign);

	//-- Add the atoms used to derive dedAtom
	//Iterating through dependencies
	const std::vector<bool>& deps = comp.at(i);
	for (size_t a = 0; a < deps.size(); a++)
	{
	  if (deps[a] == 0) continue; 
	  TAtomRef A = M->getAtom(a, true);
	  atoms.push_front(A);
	}
      }
         
      if (idx > 0) {
	if (!atoms.empty()) // && ++atoms.begin() != atoms.end()) // |atoms| > 1
	{
	  // CHRIS FIX
	  std::list<TAtomRef> extraStrictReqs; // f /= 0 for each factor f not covered by the others
	  if (!relopIsNonStrict(dedAtom->getRelop())) {
	    set<IntPolyRef> explicitNonzero;
	    for(auto itr = dedAtom->getFactors()->factorBegin(); itr != dedAtom->getFactors()->factorEnd(); ++itr)
	      explicitNonzero.insert(itr->first);
	    for(auto itr = atoms.begin(); itr != atoms.end(); ++itr)
	      if (!relopIsNonStrict((*itr)->getRelop()))
		for(auto pitr = (*itr)->getFactors()->factorBegin(); pitr != (*itr)->getFactors()->factorEnd(); ++pitr)
		  explicitNonzero.erase(pitr->first);
	    for(auto itr = explicitNonzero.begin(); itr != explicitNonzero.end(); ++itr)
	      extraStrictReqs.push_back(makeAtom(*PM,*itr,NEOP));
	  }
	  

	  //-- This actually records the newly deduced fact
	  list<TAtomRef> reqs;
	  for(auto itr = atoms.begin(); itr != atoms.end(); ++itr)
	    reqs.push_back(*itr);
	  for(auto itr = extraStrictReqs.begin(); itr != extraStrictReqs.end(); ++itr)
	    reqs.push_back(*itr);
	  auto D = DedExp(dedAtom, Deduction::BBSTR, reqs);
	  deds.emplace_back(D);
	  if (verbose /*&& false*/)//DRBROWN disabled
	  {
	    cout << "Added[a]: " << D.toString() << endl;
	  }
	  
	  //-- We also need to record the reverse facts dr brown added
	  for(std::forward_list<TAtomRef>::iterator itr = atoms.begin(); itr != atoms.end(); ++itr)
	  {
	    TAtomRef &Latom = *itr;
	    TAtomRef tmp;
	    tmp = dedAtom; dedAtom = Latom; Latom = tmp;
	    deds.emplace_back(dedAtom,Deduction::BBSTR,atoms);
	    if (verbose /*&& false*/ )//DRBROWN disabled
	      cerr << "Added[b]: " << DedExp(dedAtom, Deduction::BBSTR, atoms).toString() << endl;
	    tmp = dedAtom; dedAtom = Latom; Latom = tmp;
	  }
	}
      }
      else if (idx == 0 && (!atoms.empty() && ++atoms.begin() != atoms.end())) // |atoms| > 1
      {
	std::forward_list<TAtomRef> tatoms;
	swap(atoms,tatoms);
	std::forward_list<TAtomRef>::iterator itr = tatoms.begin();
	dedAtom = *itr;
	for(++itr; itr != atoms.end(); ++itr)
	  atoms.push_front(*itr);
	
	//-- This actually records the newly deduced fact
	deds.emplace_back(dedAtom, Deduction::BBSTR, atoms);
	if (verbose /*&& false*/)//DRBROWN disabled
	  cout << "Added***[c]: " << DedExp(dedAtom, Deduction::BBSTR, atoms).toString() << endl;
	
	//-- We also need to record the reverse facts dr brown added
	for(std::forward_list<TAtomRef>::iterator itr = atoms.begin(); itr != atoms.end(); ++itr)
	{
	  TAtomRef &Latom = *itr;
	  TAtomRef tmp;
	  tmp = dedAtom; dedAtom = Latom; Latom = tmp;
	  deds.emplace_back(dedAtom,Deduction::BBSTR,atoms);
	  if (verbose /*&& false*/)//DRBROWN disabled
	    cout << "Added***[d]: " << DedExp(dedAtom, Deduction::BBSTR, atoms).toString() << endl;
	  tmp = dedAtom; dedAtom = Latom; Latom = tmp;
	}
      }
    }
  }

  /*
    If bbsat finds BBSAT, this returns all deductions which can be made in the form 
    of a vector of BBDeds Note that MinWtBasis is not yet implemented, so no deductions 
    on the nonstrict part of the function 
    NOTE: Assumes that the matrix manager has already reduced the strict matrix
    NOTE: Ordering matters. strictDeds should always be called first, and its
    deductions should always be returned first
  */
  bblist BBDeducer::getDeductions() {

    bblist deds;
    //Size check - a formula of size one won't let BB strict deduce anything
    // short m = M->getStrict().getMatrix().size();
    // if (m < 2) {
    //   minWtMain(deds);
    //   return deds;
    // }
    strictDeds(deds);
    minWtMain(deds);
    jointDeds(deds);
    return deds;
  }


  bool BBDeducer::isStrictRow(int cutoff, const std::vector<char>& vc) {
    for (size_t i = cutoff; i < vc.size(); i++) {
      if (vc[i] != 0) return false;
    }
    return true;
  }

  int BBDeducer::weight(const std::vector<char>* vc, int cutoff) {
    int wt = 0;
    for (auto itr = vc->begin()+cutoff; itr != vc->end(); ++itr) {
      if (*itr != 0) ++wt;
    }
    return wt;
  }

  void BBDeducer::writeChar(const std::vector<char>& vc, int cutOff = 0) {
    for (size_t i = cutOff; i < vc.size(); i++)
      std::cout << (int) vc[i] << " ";
    std::cout << std::endl;
  }

  //Checks if the support of v1 is in the support of v2
  //-1 indicates no
  //0 indicates v1 is a subspace of v2
  //1 indicates equal support
  int BBDeducer::support(const std::vector<char> * v1,
                         const std::vector<char> * v2, int cutoff) {
    bool eq = true;
    for (size_t i = cutoff; i < v1->size(); i++) {
      if ((bool) v1->at(i) == (bool) v2->at(i)) continue;
      else if ((bool) !v1->at(i) && (bool) v2->at(i)) eq = false;
      else return -1;
    }
    if (eq) return 1;
    else return 0;
  }

  void BBDeducer::fillMatrices(vector<AtomRow>& Blt, vector<AtomRow>& Beq,
                               const vector<AtomRow>& container,
                               const AtomRow& a, int cutoff) {
    for (size_t i = 0; i < container.size(); i++) {
      int res = support(container[i].vc, a.vc, cutoff);
      if (res == -1) continue;
      else if (res == 0) Blt.push_back(container[i]);
      else if (res == 1) Beq.push_back(container[i]); 
    }
  }

  vector<BBDeducer::AtomRow> BBDeducer::mkB() {
    vector<AtomRow> B;
    for (size_t i = 0; i < M->getAll().getNumRows(); i++) {
      TAtomRef t = M->getAtom(i, false);
      const vector<char>& vc = M->getAll().getRow(i);
      AtomRow a(vc, t);
      B.push_back(a); 
    }

    return B;
  }

  DMatrix BBDeducer::mkMatrix(const vector<AtomRow>& rows) {
    const int numCols = rows[0].vc->size();
    DMatrix d(numCols);
    for (auto itr = rows.begin(); itr != rows.end(); ++itr) {
      d.addRow(*(itr->vc));
    }
    return d;
  }


  /*
    Note: Only called in Step 10/11 of MinWtBasis
    Returns i_0 >= 0 if once reduction occurs with wp, we have a row of the form
    1 0 0 ... 0 0, and i_0 is the index of such a row.  Else returns -1.
    Input: wp - the mod 2 image of w           
   */
  int BBDeducer::reduceRow(AtomRow& wp, vector<char>& vc,
                            forward_list<TAtomRef>& sources,
                            const DMatrix& beq, const vector<AtomRow>& va)
  {
    //Doing the reduction
    vector<char> vtmp(*(wp.vc));
    vc = vtmp;
    DMatrix reducer(beq);
    reducer.addRow(vc);
    int i_wp = reducer.getNumRows() - 1; //-- index of wp as a row of DMatrix reducer
    reducer.doElim();
    int i_0 = BBSolver::findRow(reducer);
    if (i_0 == -1) return -1; //-- no row of form 1 0 0 ... 0 0

    //Now adding the sources
    const vector<bool>& idxs = reducer.getComp()[i_0];
    for (size_t i = 0; i < idxs.size()-1; i++)
    {
      if (idxs[i] != 0)
        sources.push_front(va[i].atom);
    }
    if (idxs.back() != 0)
      sources.push_front(wp.atom);
    return i_0;
  }

  bool checkValid(TAtomRef t,
                  const forward_list<TAtomRef>& sources) {
    if (t->getFactors()->canonCompare(sources.front()->getFactors()) == 0) {
      if ((t->getRelop() & sources.front()->getRelop()) == sources.front()->getRelop()){
        return false;
      }
    }
    return true;

  }

  void BBDeducer::mkMinWtDed(AtomRow& a,
                             const forward_list<TAtomRef>& sources,
                             bblist& deds) {

    bool res;
    if (sources.empty()) return;
    TAtomRef t = M->mkNonStrictAtom(*(a.vc), res);
    if (res) {

      if (checkValid(t, sources)) {
        deds.emplace_back(t, Deduction::MINWT, sources);
        //Equivalence case - we need to identify that
        //x = 0 implies y*x <= 0 explicitly
        if (t->getRelop() == EQOP) {
          //DR BROWN THINKS IM WRONG
          forward_list<TAtomRef> fl({t});
          deds.emplace_back(a.atom, Deduction::MINWT, fl);
        }
      }
    }
  }

  

  

  /*
    Consult the algorithm MinWtBasis in the paper
    "Fast Simplifications for Tarski Formulas
    based on Monomial Inequalities" for the formal
    description of this algorithm. Note that it has been modified
    in order to provide explanations!
  */
  void BBDeducer::minWtMain(bblist& deds)
  {
    // cerr << ">>>>>>>> In minWtMain!" << endl;
    //-- set ns to the index of the first non-strict entry in the "all" matrix.
    //-- NOTE: The is Dr Brown's understanding of what Fernando wants to accomplish,
    //--       which I'm also correcting here!
    int ns = M->getStrict().getNumCols();
    if (ns == 0) ns = 1; //-- the index 0 column of the all matrix is the relop entry!

    vector<AtomRow> B = mkB();
    sort(B.begin(), B.end(), weightCompare(ns));
    while (B.size() != 0) {
      /* STEP 2-3 */
      //Note: Comparator sorts it so that biggest element is at back!
      AtomRow w = B.back();
      B.pop_back();
      if (weight(w.vc, ns) == 0)  {
        bool b = true;
        mkMinWtDed(w, deds);
        for (AtomRow& a : B) {
          b = true;
          mkMinWtDed(a, deds);
        }
        return;
      }

      /* STEP 4-5 */
      vector<AtomRow> lt, eq;
      fillMatrices(lt, eq, B, w, ns);

      /* STEP 6 */
      if (lt.size() > 0)
      {
	DMatrix blt = mkMatrix(lt);
	blt.doElim();
	int i_0 = BBSolver::findRow(blt);
	if (i_0 != -1)
	{
	  // This means w is implied by blt.  Specifically, blt produces
	  // a row of all zeros except a one in the sigma position, which
	  // represents an equation with support contained within the
	  // support of w.  That implies w holds with equality.  So ...
	  // we need to determine which rows in blt sum to zero, and then
	  // we get the deduction that those rows (mapped back to atoms)
	  // imply X (w mapped back to an atom)
	  TAtomRef X = w.atom;
	  set<IntPolyRef> strictPolysInX;
	  {
	    FactRef F = X->F;
	    for (auto itr = F->factorBegin(); itr != F->factorEnd(); ++itr)
	    {
	      IntPolyRef p = itr->first;
	      if (getMatrixManager()->isStrictPoly(p)) 
		strictPolysInX.insert(p);
	    }
	  }
	
	  //-- get indices of the rows that contributed to i_0
	  set<IntPolyRef> needStrengthening; //-- set of polys that will need strengthening
	  //-- later call strengthenWeak(needStrengthening)
	  //-- to actually strengthen
	  forward_list<TAtomRef> sources;
	  auto comp = blt.getComp();
	  const std::vector<bool>& deps = comp.at(i_0);
	  for (size_t a = 0; a < deps.size(); a++)
	  {
	    if (deps[a] == 0) continue; 
	    TAtomRef A = lt[a].atom;
	    sources.push_front(A);

	    //-- If lt[a] has any strict factors, we need to be sure that the deduction includes
	    //-- their "strengthening", i.e. includes an atom that makes them non-zero
	    //-- specifically, if lt[a]'s atom has any strict factors that are not factors of X.
	    //-- If they are a factor of X (recall, X is non-strict)) then such a factor being zero
	    //-- will still imply X.

	    // collect factors that will need strengthening
	    FactRef F = A->F;
	    for (auto itr = F->factorBegin(); itr != F->factorEnd(); ++itr)
	    {
	      IntPolyRef p = itr->first;
	      if (getMatrixManager()->isStrictPoly(p) && strictPolysInX.find(p) == strictPolysInX.end())
		needStrengthening.insert(p);		
	    }
	  }

	  //-- get strenghtening atoms
	  set<TAtomRef> SA = strengthenWeakM(needStrengthening,getMatrixManager());
	  for(auto itr = SA.begin(); itr != SA.end(); ++itr)
	    sources.push_front(*itr);
	
	  //-- record that atom X, the image of 'w', is implied be 'atoms'
	  if (verbose)
	  {
	    cout << "Making a Step 6 MinWtBasis deduction: ";
	    for(auto itr = sources.begin(); itr != sources.end(); ++itr)
	    { cout << ' '; (*itr)->write(true); }
	    cout << " ==> ";  X->write(true);
	    cout << endl;
	  }
	  deds.emplace_back(X, Deduction::MINWT, sources);
	  continue;
	}
      }
      
      /* STEP 7 */
      eq.insert(eq.end(), lt.begin(), lt.end()); 
      forward_list<TAtomRef> sources;
      if (eq.size() > 0)
      {
	DMatrix beq = mkMatrix(eq);
	beq.toMod2();

	/* STEP 8 */
	beq.doElim();
			
	/* STEP 9 */
	vector<char> vc(*(w.vc)); //-- vc is the mod 2 image of w
	for(char& c : vc) { c %= 2; }			
	AtomRow wp(vc, w.atom);
	vector<char> wpReduced;
	int i_0 = reduceRow(wp, wpReduced, sources, beq, eq);
      
	/* STEP 10 */
	if (i_0 != -1) { /* reduced to 1 0 0 ... 0 */
	  /* STEP 11 */
	  vector<char> resRow(*(w.vc)); //-- set resRow to 2*w + [1 0 ... 0]op[0 ... 0]
	  resRow[0] = 1;
	  for (int i = 1; i < ns; i++) //-- all stricts will be zero'd out
	    resRow[i] = 0;
	  for (size_t i = ns; i < resRow.size(); i++)
	    resRow[i] = (resRow[i] != 0 ? 2 : 0);
	  FactRef factors = new FactObj(getPolyManager());
	  for (size_t i = ns; i < resRow.size(); i++)
	    if (resRow[i] != 0)
	      factors->addFactor(getMatrixManager()->getPoly(i),1);
	  TAtomRef Anew = new TAtomObj(factors,EQOP);
	  AtomRow res(resRow,Anew);

	  // check whether Anew is actually in 'sources' in which case it's not a new deduction
	  // and whether it's actually just w, in which case it is likewise not new.
	  bool isNew = !equals(w.atom,Anew);
	  for(auto itr = sources.begin(); isNew && itr != sources.end(); ++itr)
	    isNew = !equals(Anew,*itr);
	  if (isNew)
	  {
	    set<IntPolyRef> needStrong, alreadyStrengthened;
	    for(auto itr = sources.begin(); isNew && itr != sources.end(); ++itr)
	    {
	      TAtomRef A = (*itr);
	      if (!relopIsNonStrict(A->getRelop())) // i.e. this is a strict atom
	      {
		FactRef F = A->F;
		for (auto itr = F->factorBegin(); itr != F->factorEnd(); ++itr)
		  alreadyStrengthened.insert(itr->first);
	      }
	    }
	    for(auto itr = sources.begin(); isNew && itr != sources.end(); ++itr)
	    {
	      TAtomRef A = (*itr);
	      if (relopIsNonStrict(A->getRelop())) // i.e. this is a non-strict atom
	      {
		FactRef F = A->F;
		for (auto itr = F->factorBegin(); itr != F->factorEnd(); ++itr)
		  if (getMatrixManager()->isStrictPoly(itr->first))
		    needStrong.insert(itr->first);
	      }
	    }
	    set<TAtomRef> SA = strengthenWeakM(needStrong,getMatrixManager());
	    for(auto itr = SA.begin(); itr != SA.end(); ++itr)
	      sources.push_front(*itr);

	    if (verbose)
	    {
	      cerr << "Making a Step 11 MinWtBasis deduction: ";
	      for(auto itr = sources.begin(); itr != sources.end(); ++itr)
	      { cerr << ' '; (*itr)->write(true); }
	      cerr << " ==> ";  res.atom->write(true);
	      cerr << endl;
	    }
	    
	    mkMinWtDed(res, sources, deds); //-- this is the newly deduced equation

	    //-- must add the fact that the new equation implies w ... that's kind of the point
	    if (verbose)
	    {
	      cerr << "Making a Step 11 MinWtBasis deduction follow on: ";
	      res.atom->write(true); cerr << " ==> ";  w.atom->write(true);
	      cerr << endl;
	    }
	    
	    forward_list<TAtomRef> fl({res.atom});
	    deds.emplace_back(w.atom, Deduction::MINWT, fl);	  
	  }
	  
	  /* STEP 12 */
	  for(int i = 0; i < B.size(); ++i)
	  {
	    if (weight(B[i].vc, ns) > 0 && support(B[i].vc, w.vc, ns) != -1)
	    {
	      if (verbose) {
		cout << "Making a Step 12 MinWtBasis deduction: ";
		res.atom->write(true); cout << " ==> "; B[i].atom->write(true); cout << endl;
	      }
	      
	      forward_list<TAtomRef> fl({res.atom});
	      deds.emplace_back(B[i].atom, Deduction::MINWT, fl);	  

	      std::swap(B[i],B[B.size()-1]);
	      B.pop_back();
	    }
	  }

	  // int numPopped = 0;
	  // for (int i = B.size()-1; i >= 0; i--) {
	  //   if (weight(B[i].vc, ns) < weight(w.vc, ns)) break;
	  //   if (support(B[i].vc, w.vc, ns) == 1) {
	  //     size_t backIdx = B.size() - (1 + numPopped);
	  //     if (i != backIdx) {
	  //       cerr << ">>>>>>>> "; B[i].atom->write(true); cerr << endl;
	  //       AtomRow tmp = B[i];
	  //       B[i] = B[backIdx];
	  //       B[backIdx] = tmp;
	  //     }
	  //     numPopped++;
	  //     i++;
	  //   }
	  // }
	  // B.erase(B.end()-numPopped, B.end()); 
 
	  continue;
	}

	vector<int> rows;
	beq.reduceRow(vc,rows);
	bool isZero = true;
	for(int i = 0; isZero && i < vc.size(); i++)
	  isZero = (vc[i] == 0);
	if (isZero)
	{ /* STEP 15 */
	  forward_list<TAtomRef> sources;
	  for(int i = 0; i < rows.size(); i++)
	    sources.push_front(eq[rows[i]].atom);
	  mkMinWtDed(w,sources,deds);
	  continue;
	}
	
      }
      
      /* STEP 13-14 */
      mkMinWtDed(w, sources, deds);
      continue;
    }
    return;
  }
  

  void BBDeducer::mkJointDed(vector<char>& row, vector<int>& sources, TAtomRef orig, bblist& deds) {
    bool res;
    TAtomRef t = M->mkNonStrictAtom(row, res);
    if (!res) return;
    forward_list<TAtomRef> proof;

    forward_list<DedExp> backup;
    for (size_t i = 0; i < sources.size(); i++) {
      //NOTE: This line may be dangerous
      //Because the guarantee that each atom has been deduced
      //Is because each atom is a pivot row in a succesful
      //Deduction...
      proof.push_front(M->getMeaning(sources[i]));
      backup.emplace_front(M->getMeaning(sources[i]),
                           Deduction::BBSTR, M->explainMeaning(sources[i]));
    }
    proof.push_front(orig);
    if (!equals(t, proof.front())) {
      for (auto itr = backup.begin(); itr != backup.end(); ++itr) {
        deds.emplace_back(*itr);
      }
      deds.emplace_back(t, Deduction::BBCOM, proof);
      //EQUIVALNCE
      //DR BROWN THINKS IM WRONG

      //removing orig
      auto begin = proof.begin(); ++begin;
      forward_list<TAtomRef> tmp(begin, proof.end());
      tmp.push_front(t);
      deds.emplace_back(orig, Deduction::BBCOM, tmp);
    }
  }


  void BBDeducer::jointDeds(bblist& deds) {
    const DMatrix& ns = M->getAll();
    const DMatrix& s = M->getStrict();
    size_t j = 0;
    const vector<size_t>& dualRows = M->dualRows();
    for (size_t i = 0; i < ns.getNumRows(); i++) {
      if (j < dualRows.size() && dualRows[j] == i) {
        j++;
        continue;
      }
      vector<int> source;
      vector<char> toRed(ns.getRow(i));
      s.reduceRow(toRed, source);
      TAtomRef orig = M->getAtom(i, false);
      mkJointDed(toRed, source, orig, deds);
    }
  }

  //only attempts to deduce the strict atoms
  list<DedExp> BBDeducer::attemptDeduce(std::vector<Deduction>::const_iterator begin, std::vector<Deduction>::const_iterator end) {
    list<DedExp> deds;
    const DMatrix& s = M->getStrict();
    if (s.getNumCols() == 0) return deds;
    while (begin != end) {
      //Making sure that this atom is strict
      TAtomRef t = begin->getDed();


      //Reconstructing the original row
      FactRef f = t->getFactors();
      bool fail = false;
      std::vector<char> row(s.getNumCols());
      row[0] = (t->getRelop() == GTOP || t->getRelop() == GEOP)
        ? 1 : 0;
      for (auto itr = f->factorBegin(); itr != f->factorEnd(); ++itr) {
        int col = M->getIdx(itr->first);
        if (col == -1 || (size_t) col >= s.getNumCols()) {
          fail = true; break;
        }
        row[col] = 1;
      }
      if (fail) {
        ++begin;
        continue;
      }

      //Reducing the original row. If its null, we can deduce the
      //corresponding atom!
      vector<int> source;
      s.reduceRow(row, source);
      for (auto itr = row.begin()+1; itr != row.end(); ++itr) {
        if (*itr) {
          ++begin;
          fail = true;
          break;
        }
      }
      if (fail) continue;

      //Constructing the deduction
      std::forward_list<TAtomRef> atoms;
      for (size_t i = 0; i < source.size(); i++) {
        TAtomRef nu = M->getMeaning(source[i]);
        deds.emplace_back(nu, Deduction::BBSTR, M->explainMeaning(source[i]));
        atoms.push_front(nu);
      }
      if (!(source.size() == 1 && equals(t, atoms.front())))  {
        deds.emplace_back(t, Deduction::BBSTR, atoms);
      }

      ++begin;
    }
    return deds;
  }

  /*
    scoreFun2 strengthens a single intpolyref from LEOP or GEOP to LTOP or GTOP, respectively.
    MIR is the original representation of the formula
    p is the intpolyref in question we need to strengthen.
    purestrict is the index of all atoms which can strengthen something (AKA, all atoms with sign LTOP/GTOP)

    Outputs a TAtomRef which strengthens p
  
    TAtomRef scoreFun2(MonoIneqRep &MIR, IntPolyRef p, std::vector<short> pureStrict) {
    TAtomRef best;
    int bestScore = INT_MIN;
    std::vector<TAtomRef> cands = MIR.getCandidates().at(p);
    for (int i = 0; i < cands.size(); i++) {
    int score = 0;
    TAtomRef A = cands[i];
    FactRef F = A->F;

    bool hasp = false;
    for (std::map<IntPolyRef, int>::iterator itr = F->factorBegin(); itr != F->factorEnd(); ++itr) {
    if (itr->first->equal(p)) { hasp = true; break;}
    }
    if (hasp) {
    score = 7 + -2*(F->numFactors() - 1);
    }
    if (score > bestScore) {
    best = A;
    bestScore = score;
    }
    }
    if (bestScore == INT_MIN)  {
    throw TarskiException("ERROR IN SCOREFUN2 BBSAT.cpp - No strengthening candidates available");
    }
    return best;
    }
  */


  /* The dreaded scoringFunction(std::map<IntPolyRef, std::vector <TAtomRef> > candidates, std::map<IntPolyRef, bool> weakFactors)
     NOTE: DEPRECATED
     * Input:
     * candidates, a mapping from each factor in the formula to all the atoms which can strengthen them
     * weakFactors, all the factors in need of strengthening
     *
     * Output:
     * strength, the std::vector<IntPolyRef> which contains all the factors necessary to strengthen all the members of weakFactors
     
     std::vector<TAtomRef> scoringFunction(const std::map<IntPolyRef, std::vector<TAtomRef> > &candidates, const std::map<IntPolyRef, bool> &weakFactors)
     {g

     if (weakFactors.size() == 0) {
     std::vector<TAtomRef> toReturn;
     return toReturn;
     }
     //Step 0: Make a array out of weakFactors
     std::vector<IntPolyRef> arrWeakFactors(weakFactors.size());
     int i = 0;
     for (std::map<IntPolyRef, bool>::const_iterator iter = weakFactors.begin(); iter != weakFactors.end(); ++iter){
     arrWeakFactors[i] = iter->first;
     i++;
     }
     //Step 1: figure out which members of candidates we actually need, put them in an  std::vector of TAtomRef
     //Step 1a: Make a mapping from weakFactors to the AtomRefs in goodCandidates
     //Step 1b: Penalize it if we've seen it the first time
     //Step 1c: +7 every time we see it
     //Efficiency: Use goodCandFast so we dont have to iterate through goodCandidates. goodCandFast stores the indices of goodCandidates by using pairs.
     std::vector < TAtomRef > goodCandidates;
     std::vector < int > scores;

     std::map < TAtomRef, int, TAtomObj::OrderComp > goodCandFast;
     std::vector < std::vector<int> > factsToCand (weakFactors.size());
     std::vector < std::vector<int> > candToFacts;
     for  (i = 0; i < weakFactors.size(); i++){ //For every factor...
     bool done = false;
     std::vector<TAtomRef> toCheck = candidates.at(arrWeakFactors[i]);

     for (std::vector<TAtomRef>::iterator atomIt = toCheck.begin(); atomIt != toCheck.end(); ++atomIt) { //Check all its factors
     TAtomRef a = *atomIt;
     std::map<TAtomRef, int, TAtomObj::OrderComp>::iterator  iter2 = goodCandFast.find(a);
     if (iter2 != goodCandFast.end()) { //Meaning that it already appears in goodCandidates
     factsToCand[i].push_back(iter2->second); //Save the index so where we know where to find it when making our selections
     candToFacts[iter2->second].push_back(i);
     scores[iter2->second] += 7;
     continue;
     }
     else {
     goodCandidates.push_back(a);
     scores.push_back(a->getVars().numElements()*-2 + 7);
     factsToCand[i].push_back(goodCandidates.size()-1);
     std::vector <int> tmp (1, i);
     candToFacts.push_back(tmp);
     goodCandFast[a] = goodCandidates.size()-1;
     }
     }
     }


     //Step 2: Resize selected to the size of scores
     std::vector < bool > selected(scores.size());
     std::vector < TAtomRef > toReturn;
     //Step 3: Iterate through every element of arrWeakFactors
     //Step 3a: Check to make sure none of the potential inequalities have been chosen. If one has, skip this round (the inequality has already been strengthened)
     //Step 3b: Choose the highest scored inequality. Instert it into strength
     //Step 3c: For all other inequalities of the current member of arrWeakFactors, leverage a -7 penalty
     for (int i = 0; i < weakFactors.size(); i++) {
     bool alreadySelected = false;
     int highestIdx  = factsToCand[i][0];
     for (std::vector<int>::iterator iter = factsToCand[i].begin(); iter != factsToCand[i].end(); ++iter) {
     if (selected[*iter] == true) {
     alreadySelected = true;
     }
     scores[*iter] -= 7;
     if (!alreadySelected) {
     if (scores[highestIdx] < scores[*iter])
     highestIdx = *iter;
     }

     }

     if (!alreadySelected) {
     toReturn.push_back(goodCandidates[highestIdx]);
     for (std::vector<int>::iterator iter = candToFacts[highestIdx].begin(); iter != candToFacts[highestIdx].end(); ++iter) { //For all factors this candidate can strengthen
     for (std::vector<int>::iterator iter2 = factsToCand[*iter].begin(); iter2 != factsToCand[*iter].end(); ++iter2) {//Subtract the score since it's not necessary anymore
     scores[*iter2] -= 7;
     }
     }
     selected[highestIdx] = true;
     }
     }

     return toReturn;
     }
  */
  
  

} //end namespace 
