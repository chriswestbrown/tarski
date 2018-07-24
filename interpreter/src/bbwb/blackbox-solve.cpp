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

  Deduction * BBSolver::deduce(TAndRef t) {
    if (deductions.size() == 0) {
      if (!once)  { once = true; return NULL; }
      else {
        deductions = bbsat(t);
        if (deductions.size() == 0) return NULL;
        else once = false;
      }
    }
    Deduction * d = deductions.back();
    deductions.pop_back();
    return d;
  }

  void BBSolver::update(std::vector<Deduction *>::const_iterator begin, std::vector<Deduction *>::const_iterator end) {
    while (begin != end) {
      M.addAtom((*begin)->getDed());
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
  std::vector<Deduction *> BBSolver::bbsat(TAndRef t) {
    // Process the formula
    BBChecker bc(&M, t->getPolyManagerPtr());
    if (!bc.checkSat()) {
      return bc.explainUnsat();
    }

    else {
      BBDeducer bd(&M, t->getPolyManagerPtr());
      return bd.getDeductions();
    }
  }





  bool BBChecker::checkSat() {
    M->strictElim();
    const DMatrix& d = M->getStrict();
    const std::vector<std::vector<char> >& matrix = d.getMatrix();
    for (int i = 0; i < matrix.size(); i++ ) {
      const std::vector<char>& row = matrix.at(i);
      if (row.at(0) == false) continue;
      bool isSat = false;
      for (int j = 1; j < row.size(); j++) 
        if (row.at(j) == true ) { isSat = true; break; }
      if (!isSat) { unsatRow = i; return isSat; }
    }
    return true;
  }

  /*
    If bbsat finds UNSAT, this returns the explanation in the form of a vector of BBDeds with a single BBDed which is the explanation
  */
  std::vector<Deduction *> BBChecker::explainUnsat() {
    std::vector<TAtomRef> conflict = getConflict();
    std::set<IntPolyRef> weakFacts = findWeak(conflict);
    std::set<TAtomRef> additions = strengthenWeak(weakFacts);
    conflict.insert(conflict.end(), additions.begin(), additions.end());
    return { new BBDed(conflict) };
  }

  /*
    Gets all the atoms involved in the conflict by using reasons (the indices in the matrix)
    and traceRow (which maps the new rows to original rows) to find the corresponding atom
    in the MonoIneqRep
  */
  std::vector<TAtomRef> BBChecker::getConflict() {
    std::vector<TAtomRef> conflict;
    const std::vector<bool>& exp = M->getStrict().getComp().at(unsatRow);
    for (int i = 0; i < exp.size(); i++) {
      if (exp.at(i) == true) {
        conflict.push_back(M->getAtom(i, true));
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
  std::set<IntPolyRef> BBChecker::findWeak(std::vector<TAtomRef>& conflict ) {
    std::set<IntPolyRef> strongFacts;
    std::set<IntPolyRef> weakFacts; 
    for (std::vector<TAtomRef>::const_iterator itr = conflict.begin(), end = conflict.end();
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
  void BBDeducer::strictDeds(std::vector<Deduction *>& deds) {
    //Here, we iterate through everything that we learned by doing gaussian elimination (AKA, all rows which were of the form X, 0, 0, ..., 1, 0, ..., 0 which were not of that form before gaussian elimination)
    const std::vector<std::vector<char> >& matrix = M->getStrict().getMatrix();
    const std::vector<std::vector<bool> >& comp = M->getStrict().getComp();
    for (int i = 0; i < matrix.size(); i++) {
      short idx = getNonZero(matrix.at(i));
      if (idx == -1) continue;
      std::vector<TAtomRef> atoms;

      IntPolyRef dedP = M->getPoly(idx);
      short dedSign = (matrix.at(i).at(0) == 0 ? GTOP : LTOP);
      FactRef F = new FactObj(PM);
      F->addFactor(dedP, 1);
      TAtomRef dedAtom = new TAtomObj(F, dedSign);

      const std::vector<bool>& deps = comp.at(i);
      bool strengthen = false;
      //Iterating through the dependencies of this deduction, which are identified by a "1" in deps
      for (int a = 0; a < deps.size(); a++) {
        if (deps.at(a) == 0) continue; //Indicates the row is unused for this deduction
        TAtomRef A = M->getAtom(a, true);
        atoms.push_back(A);
        //In this block, we see if one of the already existing dependencies of the deduction strengthens it
        if (!strengthen && deps.at(a) != 0) { //If it contains the polynomial which we are trying to strengthen
          if (A->relop == LEOP || A->relop == GEOP || A->relop == EQOP) continue;
          FactRef F = A->F;
          for (std::map<IntPolyRef, int>::iterator itr = F->factorBegin(), end = F->factorEnd(); itr != end; ++itr) {
            if (dedP->equal(itr->first)) {
              strengthen = true;
              break;
            }
          }
        }



      }
      if (strengthen == false)  {
        atoms.push_back(M->strengthenPoly(dedP));
      }

      deds.push_back(new BBDed(dedAtom, atoms));
    }
  }


  /*
    If bbsat finds BBSAT, this returns all deductions which can be made in the form of a vector of BBDeds
    Note that MinWtBasis is not yet implemented, so no deductions on the nonstrict part of the function
    NOTE: Assumes that theh matrix manager has already reduced the strict matrix
  */
  std::vector<Deduction *> BBDeducer::getDeductions() {

    std::vector<Deduction  *> deds;
    //Size check - a formula of size one won't let BB Deduce anything
    short m = M->getStrict().getMatrix().size();
    if (m < 2) {
      return deds;
    }
    strictDeds(deds);
    //TODO: get MinWtBasis working properly
    //vector<Deduction * > deds2 = minWtBasisDeds(MIR, pureStrict);
    //deds.insert(deds.end(), deds2.begin(), deds2.end());
    return deds;
  }


  void BBDeducer::minWtExplain(std::vector<Deduction *>& deds) {
  
  }

  bool BBDeducer::isStrictRow(int cutOff, const std::vector<char>& vc) {
    for (int i = cutoff; i < vc.size(); i++) {
      if (vc[i] != 0) return false;
    }
    return true;
  }

  int BBDeducer::weight(const std::vector<char>& vc) {
    int wt = 0;
    for (std::vector<char>::const_iterator itr = vc.begin()+1; itr != vc.end(); ++ itr) {
      if (*itr != 0) ++wt;
    }
    return wt;
  }

  //Checks if v1 is in the space of v2
  int BBDeducer::space(const std::vector<char>& v1, const std::vector<char>& v2, int cutoff) {
    bool a = false, b = false;
    for (unsigned long i = cutoff; i < v1.size() && !b; i++) {
      a |= (v1[i] == 0 && v2[i] != 0);
      b |= (v1[i] != 0 && v2[i] == 0);
    }
    if (b) return -1;
    if (!a) return 0;
    return 1;
  }

  void BBDeducer::minWtMain() {
    const DMatrix& nonStrict = M->getAll();
    int ns = M->getStrict().getNumCols();
    int nn = nonStrict.getNumCols()-ns;

    map<vector<char>, set<TAtomRef> > Bi;
    for (unsigned int i = 0; i < nonStrict.getNumRows(); i++) {
      const std::vector<char>& vc = nonStrict.getRow(i);
      if (isStrictRow(ns, vc)) continue;
      set<TAtomRef> source;
      //Reduce the strict part of getRow[i] by the strict Matrix
      //Source is the set of TAtomRef needed to reduce this row
      Bi[vc] = source;
    }

    while (Bi.size() > 0) {
      //Choose w
      int maxWeight = -1; set <vector<char> >::iterator mitr;
      for (set<vector<char> >::iterator itr = Bi.begin();
           itr != Bi.end(); ++itr) {
        int tmpWeight = weight(*itr);
        if (tmpWeight > maxWeight) { maxWeight = tmpWeight; mitr = itr; }
      }
      vector<char> w = *mitr;
      Bi.erase(w);
      //Construct Blt and Beq
      vector<map<vector<char>, set<TAtomRef> >::iterator> Blt, Beq;
      for (map<vector<char>, set<TAtomRef> >::iterator itr = Bi.begin();
             itr != Bi.end(); ++itr) {
        int sp = space(itr->first, w);
        if (sp == 1) { Blt.push_back(itr); }
        else if (sp == 0) {Beq.push_back(itr); }
      }
    }
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
