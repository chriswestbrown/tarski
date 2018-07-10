#include "deduction.h"
#include "minwtbasis.h"
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

  /*
    BBSat performs the bbsat algorithm
    If the function determines UNSAT, then BBSAT returns the reason
    in the form of a vector of deductions
    Otherwise, the function attempts to make deductions on the formula
    with the Blackbox algorithm
  */
  std::vector<Deduction *> BBSolver::bbsat(TAndRef t) {
    // Process the formula
    MonoIneqRep m;
    m.processConjunction(t);
    BBChecker bc(&m, &polyToSIneq, t->getPolyManagerPtr());
    if (bc.checkSat()) {
      return bc.explainUnsat();
    }

    else {
      BBDeducer bd(&m, &polyToSIneq, t->getPolyManagerPtr());
      return bd.getDeductions();
    }
  }





  bool BBChecker::checkSat() {
    // Pull out the strict part
    Matrix M = MIR->genStrictMatrix(traceRow);
    //printMIR(); printBeforeGauss(M);
    // Gaussian Elimination!
    std::vector<int> piv;
    std::vector<int> piv2;
    std::vector<DBV>  george = idMatrix(M);
    reducedGaussExplain(M,george,piv,piv2,true);
    //printAfterGauss(M, george);
    return detectUnsat(M, george, reasons, false);

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
    for (std::vector<int>::iterator itr = reasons.begin(), end = reasons.end();
         itr != end; ++itr) {
      int j = traceRow[(*itr)];
      TAtomRef A = MIR->rowIndexToAtom(j);
      conflict.push_back(A);
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
      for (std::set<IntPolyRef>::iterator itr = weakFacts.begin(), end = weakFacts.end(); itr != end; ++itr) {
        TAtomRef t = (*polyToSIneq)[*itr];
        sAtoms.insert(t);
      }
      return sAtoms;
    }

  void BBChecker::printMIR() {
    std::cout << "Vector Rep:" << std::endl;
    std::cout << "sigma";
    int r0 = MIR->numStrict();
    int m0 = MIR->numVars();
    for(int i = 1; i <= m0; i++)
      {
        if (i == r0 + 1) { std::cout << " :"; }
        std::cout << " (";
        MIR->reverseMap[i]->first->write(*PM);
        std::cout << ")";
      }
    std::cout << std::endl;

    for(unsigned int i = 0; i < MIR->B.size(); ++i)
      MIR->B[i].write();
  }

  void BBChecker::printBeforeGauss(Matrix& M) {
    std::cout << "Before gauss:" << std::endl;
    write(M);
  }

  void BBChecker::printAfterGauss(Matrix& M, vector<DBV>& george) {
    std::cout << "After gauss:" << std::endl;
    std::cout << "M: " << std::endl; write(M);
    std::cout << "George: " << std::endl; write(george);
  }


  

  /*
    This function actually makes the deductions returned by getDeductions for the strict part of the matrix
  */
  void BBDeducer::strictDeds(Matrix& M, std::vector<DBV>& george, vector<int>& traceRow, std::vector<Deduction *>& deds) {
    //Here, we iterate through everything that we learned by doing gaussian elimination (AKA, all rows which were of the form X, 0, 0, ..., 1, 0, ..., 0 which were not of that form before gaussian elimination)
    for (int i = 0; i < M.getNumRows(); i++) {
      short idx = M[i].getNonZero(true);
      if (idx == -1) continue;
      std::vector<TAtomRef> atoms;

      IntPolyRef dedP = MIR->reverseMap[idx]->first;
      short dedSign = (M[i].get(0) == 0 ? GTOP : LTOP);
      FactRef F = new FactObj(PM);
      F->addFactor(dedP, 1);
      TAtomRef dedAtom = new TAtomObj(F, dedSign);

      DBV deps = george[i];
      bool strengthen = false;
      //Iterating through the dependencies of this deduction, which are identified by a "1" in deps
      for (int a = 0; a < deps.size(); a++) {
        if (deps.get(a) == 0) continue; //Indicates the row is unused for this deduction
        int j = traceRow[a];
        TAtomRef A = MIR->rowIndexToAtom(j);
        atoms.push_back(A);
        //In this block, we see if one of the already existing dependencies of the deduction strengthens it
        if (!strengthen && deps.get(a) != 0) { //If it contains the polynomial which we are trying to strengthen
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
        atoms.push_back((*polyToSIneq)[dedP]);
      }

      deds.push_back(new BBDed(dedAtom, atoms));
    }
  }


  /*
    If bbsat finds BBSAT, this returns all deductions which can be made in the form of a vector of BBDeds
    Note that MinWtBasis is not yet implemented, so no deductions on the nonstrict part of the function
  */
  std::vector<Deduction *> BBDeducer::getDeductions() {

    std::vector<Deduction  *> deds;
    //Size check - a formula of size one won't let BB Deduce anything
    short m = MIR->B.size();
    if (m < 2) {
      return deds;
    }


    vector<int> traceRow;
    Matrix M = MIR->genStrictMatrix(traceRow);

    std::vector<int> piv;
    std::vector<int> piv2;
    std::vector<DBV> george = idMatrix(M);
    reducedGaussExplain(M,george,piv,piv2,true);

    strictDeds(M, george, traceRow, deds);

    //TODO: get MinWtBasis working properly
    //vector<Deduction * > deds2 = minWtBasisDeds(MIR, pureStrict);
    //deds.insert(deds.end(), deds2.begin(), deds2.end());

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
  {

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
  void BBSolver::populatePoly(TAndRef t) {
    std::set<TFormRef,ConjunctOrder>::iterator itr, end;
    for (itr = t->begin(),  end = t->end(); itr != end; ++itr) {
      TAtomRef A = asa<TAtomObj>(*itr);
      populatePoly(A);
    }
  }
  void BBSolver::populatePoly(TAtomRef A) {
    if (A->getRelop() != NEOP && A->getRelop() != GTOP && A->getRelop() != LTOP) return;
    FactRef F = A->F;
    std::map<IntPolyRef,int>::iterator itr, end;
    for (itr = F->factorBegin(), end = F->factorEnd(); itr != end; ++itr) {
      polyToSIneq[itr->first] = A;
    }
  }
  

} //end namespace 
