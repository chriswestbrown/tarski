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
        deductions = bbsat(t);
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

  void BBSolver::update(std::vector<Deduction>::const_iterator begin, std::vector<Deduction>::const_iterator end) {
    while (begin != end) {
      M.addAtom(begin->getDed());
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
    M->strictElim();
    const DMatrix& d = M->getStrict();
    unsatRow = BBSolver::findRow(d);
    return (unsatRow == -1) ? true : false;
  }

  /*
    If bbsat finds UNSAT, this returns the explanation in the form of a vector of BBDeds with a single BBDed which is the explanation
  */
  bblist BBChecker::explainUnsat() {
    //std::cerr << "Unsat row is " << unsatRow << std::endl;
    //M->write();
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
    for (int i = 0; i < exp.size(); i++) {
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
    for (std::forward_list<TAtomRef>::const_iterator itr = conflict.begin(), end = conflict.end();
         itr != end; ++itr ) {
      TAtomRef A = *itr;
      if (A->getRelop() == LTOP || A->getRelop() == GTOP || A->getRelop() == NEOP) {
        for(std::map<IntPolyRef,int>::iterator fitr = A->factorsBegin(); fitr != A->factorsEnd(); ++fitr) {
          strongFacts.insert(fitr->first);
        }
      }
      
      else {
        for(std::map<IntPolyRef,int>::iterator fitr = A->factorsBegin(); fitr != A->factorsEnd(); ++fitr) {
          weakFacts.insert(fitr->first);
        }
      }
    }
    for (std::set<IntPolyRef>::iterator itr = strongFacts.begin(), end = strongFacts.end(); itr != end; ++itr) {
      weakFacts.erase(*itr);
    }
    return weakFacts;
  }

  /*
    Given a set of intpolyrefs which represent the "weak" factors in some explanation, generate a set of
    atoms which strengthen that intpolyref
  */
  std::set<TAtomRef> BBChecker::strengthenWeak(const std::set<IntPolyRef>& weakFacts) {
    set<TAtomRef> sAtoms;
    for (std::set<IntPolyRef>::const_iterator itr = weakFacts.begin(), end = weakFacts.end(); itr != end; ++itr){
      TAtomRef t = M->strengthenPoly(*itr);
      sAtoms.insert(t);
    }
    return sAtoms;
  }




  void BBChecker::printAfterGauss() {
    std::cout << "After gauss:" << std::endl;
    std::cout << "M: " << std::endl; M->write();
  }


  
  int BBDeducer::getNonZero(const std::vector<char>& row) {
    int unique = -1;
    for (int i = 1; i < row.size(); i++)  {
      if (row.at(i) && unique == -1) unique = i;
      else if (row.at(i)) return -1;
    }
    return unique;
  }


  /*
    This function actually makes the deductions returned by getDeductions for the strict part of the matrix
  */
  void BBDeducer::strictDeds(bblist& deds) {
    //Here, we iterate through everything that we learned by doing gaussian elimination (AKA, all rows which were of the form X, 0, 0, ..., 1, 0, ..., 0 which were not of that form before gaussian elimination)
    const std::vector<std::vector<char> >& matrix = M->getStrict().getMatrix();
    const std::vector<std::vector<bool> >& comp = M->getStrict().getComp();
    for (int i = 0; i < matrix.size(); i++) {
      short idx = getNonZero(matrix.at(i));
      if (idx == -1) continue;
      std::forward_list<TAtomRef> atoms;
      IntPolyRef dedP = M->getPoly(idx);
      short dedSign = (matrix.at(i).at(0) == 0 ? GTOP : LTOP);
      FactRef F = new FactObj(PM);
      F->addFactor(dedP, 1);
      TAtomRef dedAtom = new TAtomObj(F, dedSign);
      const std::vector<bool>& deps = comp.at(i);
      bool strengthen = false;
      //Iterating through dependencies
      for (int a = 0; a < deps.size(); a++) {
        if (deps[a] == 0) continue; 
        TAtomRef A = M->getAtom(a, true);
        atoms.push_front(A);
        //check if an existing dependency strengthens
        if (strengthen) continue;
        if (A->relop == LEOP || A->relop == GEOP || A->relop == EQOP) continue;
        FactRef F = A->F;
        for (std::map<IntPolyRef, int>::iterator itr = F->factorBegin(), end = F->factorEnd(); itr != end; ++itr) {
          if (dedP->equal(itr->first)) {
            strengthen = true;
            break;
          }
        }
      }
      if (!strengthen)  {
        atoms.push_front(M->strengthenPoly(dedP));
      }
      if (!(equals(dedAtom, atoms.front())))  {
        deds.emplace_back(dedAtom, Deduction::BBSTR, atoms);
      }
    }
  }

  /*
    If bbsat finds BBSAT, this returns all deductions which can be made in the form of a vector of BBDeds
    Note that MinWtBasis is not yet implemented, so no deductions on the nonstrict part of the function
    NOTE: Assumes that theh matrix manager has already reduced the strict matrix
    NOTE: Ordering matters. strictDeds should always be called first, and its
    deductions should always be returned first
  */
  bblist BBDeducer::getDeductions() {

    bblist deds;
    //Size check - a formula of size one won't let BB strict deduce anything
    short m = M->getStrict().getMatrix().size();
    if (m < 2) {
      minWtMain(deds);
      return deds;
    }
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
    for (std::vector<char>::const_iterator itr = vc->begin()+cutoff;
         itr != vc->end(); ++itr) {
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
    DMatrix d;
    for (vector<AtomRow>::const_iterator itr = rows.begin();
         itr != rows.end(); ++itr) {
      d.addRow(*(itr->vc));
    }
    return d;
  }


  /*
    Returns true if once reduction occurs with wp, we have a row of the form
    1 0 0 ... 0 0
   */
  bool BBDeducer::reduceRow(AtomRow& wp, vector<char>& vc,
                            forward_list<TAtomRef>& sources,
                            const DMatrix& beq, const vector<AtomRow>& va) {
    //Doing the reduction
    vector<char> vtmp(*(wp.vc));
    vc = vtmp;
    DMatrix reducer(beq);
    reducer.addRow(vc);
    reducer.doElim();
    if (BBSolver::findRow(reducer) == -1) return false;
    //Now adding the sources
    sources.push_front(wp.atom);
    const vector<bool>& idxs = reducer.getComp().back();
    for (size_t i = 0; i < idxs.size()-1; i++) {
      if (idxs[i]) {
        sources.push_front(va[i].atom);
      }

    }
    return true;
  }

  bool checkValid(TAtomRef t,
                  const forward_list<TAtomRef>& sources) {
    if (t->getFactors()->cmp(sources.front()->getFactors()) == 0) {
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
  void BBDeducer::minWtMain(bblist& deds) {
    int ns = M->getStrict().getNumCols();
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
      DMatrix blt = mkMatrix(lt);
      blt.doElim();
      if (BBSolver::findRow(blt) != -1)  {
        continue;
      }

      /* STEP 7 */
      eq.insert(eq.end(), lt.begin(), lt.end()); 
      DMatrix beq = mkMatrix(eq);
      beq.toMod2();

      /* STEP 8 */
      beq.doElim();
      /* STEP 9 */

      vector<char> vc(*(w.vc));
      for (char& c : vc) {
        c %= 2;
      }
      /* STEP 10 */
      AtomRow wp(vc, w.atom);
      vector<char> wpReduced;
      forward_list<TAtomRef> sources;

      if (reduceRow(wp, wpReduced, sources, beq, eq)) {
        /* STEP 11 */
        vector<char> resRow(*(w.vc));
        resRow[0] = 1;
        for (int i = 1; i < ns; i++) {
          resRow[i] = 0;
        }
        for (size_t i = ns; i < resRow.size(); i++) {
          if (resRow[i] == 1) resRow[i] = 2;
        }
        AtomRow res(resRow, w.atom);
        mkMinWtDed(res, sources, deds);

        /* STEP 12 */
        int numPopped = 0;
        for (int i = B.size()-1; i >= 0; i--) {
          if (weight(B[i].vc, ns) < weight(w.vc, ns)) break;
          if (support(B[i].vc, w.vc, ns) == 1) {
            size_t backIdx = B.size() - (1 + numPopped);
            if (i != backIdx) {
              AtomRow tmp = B[i];
              B[i] = B[backIdx];
              B[backIdx] = tmp;
            }
            numPopped++;
            i++;
          }
        }
        B.erase(B.end()-numPopped, B.end()); 
        continue;
      }

      /* STEP 13-14 */
      //We combine the null step and the non null case here
      //Because we want to deduce the null row and say that
      //Some atom can be deduced by other atoms
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
    proof.push_front(orig);
    for (size_t i = 0; i < sources.size(); i++) {
      //NOTE: This line may be dangerous
      //Because the guarantee that each atom has been deduced
      //Is because each atom is a pivot row in a succesful
      //Deduction...
      proof.push_front(M->getMeaning(sources[i]));
    }
    if (!equals(t, proof.front()))
      deds.emplace_back(t, Deduction::BBCOM, proof);
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
      for (FactObj::factorIterator itr = f->factorBegin();
           itr != f->factorEnd(); ++itr) {
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
      for (std::vector<char>::iterator itr = row.begin()+1; itr != row.end(); ++itr) {
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
        forward_list<TAtomRef> expNu = M->explainMeaning(source[i]);
        deds.emplace_back(nu, Deduction::BBSTR, expNu);
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
