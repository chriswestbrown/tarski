#include "../formula/monomialinequality.h"
#include "bbded.h"
#include "deduction.h"
#include "minwtbasis.h"
#include "../poly/factor.h"
#include <set>
#include <ctime>
#include <string>

namespace tarski {
  void classifyFactors(const std::vector<int>& reasons, const std::vector<int>& traceRow, MonoIneqRep& MIR,   std::map<IntPolyRef, bool>& weakFacts, std::map<IntPolyRef, bool>& strongFacts, std::vector<TAtomRef>& conflict) {
    for (int i = 0; i < reasons.size(); i++) {
      int j = traceRow.at(reasons.at(i));
      TAtomRef A = MIR.rowIndexToAtom(j);
      conflict.push_back(A);

      for(std::map<IntPolyRef,int>::iterator fitr = A->factorsBegin(); fitr != A->factorsEnd(); ++fitr)
        {
          if (A->relop == 1 || A->relop == 4 || A->relop == 5) {
            strongFacts[fitr->first] = true;
          }
          else {
            weakFacts[fitr->first] = true;
          }
        }
    }

    std::vector<std::map<IntPolyRef, bool>::iterator> toRemove;
    std::map<IntPolyRef, bool>::iterator ater = weakFacts.begin();
    std::map<IntPolyRef, bool>::iterator ater2 = strongFacts.begin();
    while (ater != weakFacts.end() && ater2 != strongFacts.end()) {
      {
        if (ater->first < ater2->first) {
          ++ater;
        }
        else if (ater2->first < ater->first){
          ++ater2;
        }
        else {
          toRemove.push_back(ater);
          ++ater;
        }
      }
    }
    for (int i = 0; i < toRemove.size(); i++)
      weakFacts.erase(toRemove[i]);
  }

  BBDed * genDed(std::vector<TAtomRef>& additions, std::vector<TAtomRef>& conflict) {

    additions.insert(additions.end(), conflict.begin(), conflict.end());
    

    return new BBDed(additions);
  }

  std::vector<Deduction *> getExplain(MonoIneqRep& MIR, const std::vector<int>& reasons, const std::vector<int>& traceRow) {
    std::map<IntPolyRef, bool> weakFacts;
    std::map<IntPolyRef, bool> strongFacts;
    std::vector<TAtomRef> conflict;
    classifyFactors(reasons, traceRow, MIR, weakFacts, strongFacts, conflict);
    std::vector<TAtomRef> additions = scoringFunction(MIR.getCandidates(), weakFacts);
    std::vector<Deduction *> toRet;
    toRet.push_back(genDed(additions, conflict));
    return toRet;
  }


  void printMIR(MonoIneqRep& MIR, PolyManager& PM) {
    std::cout << "Vector Rep:" << std::endl;
    std::cout << "sigma";
    int r0 = MIR.numStrict();
    int m0 = MIR.numVars();
    for(int i = 1; i <= m0; i++)
      {
        if (i == r0 + 1) { std::cout << " :"; }
        std::cout << " (";
        MIR.reverseMap[i]->first->write(PM);
        std::cout << ")";
      }
    std::cout << std::endl;

    for(unsigned int i = 0; i < MIR.B.size(); ++i)
      MIR.B[i].write();
  }

  void printBeforeGauss(Matrix& M) {
    std::cout << "Before gauss:" << std::endl;
    write(M);
  }

  void printAfterGauss(Matrix& M, vector<DBV>& george) {
    std::cout << "After gauss:" << std::endl;
    std::cout << "M: " << std::endl; write(M);
    std::cout << "George: " << std::endl; write(george);
  }


  std::vector<Deduction *> bbsat(const TAndRef &C) {
    // Process the formula
    std::vector<Deduction *> discovered;
    PolyManager &PM = *(C->getPolyManagerPtr());
    MonoIneqRep MIR;
    MIR.processConjunction(C);

    
    // Pull out the strict part
    vector<int> traceRow;
    Matrix M = MIR.genStrictMatrix(traceRow);

    if (verbose) { printMIR(MIR, PM); printBeforeGauss(M);}

    // Gaussian Elimination!
    std::vector<int> piv;
    std::vector<int> piv2;
    std::vector<DBV>  george = idMatrix(M);
    reducedGaussExplain(M,george,piv,piv2,true);

    //CHANGES
    if (verbose) printAfterGauss(M, george);

    bool unsat = false;
    std::vector<int> reasons;
    unsat = detectUnsat(M, george, reasons, false);

    if (unsat) return getExplain(MIR, reasons, traceRow);
    else return getDeductions(MIR, C);

  }


  std::vector<short> findPureStrict(MonoIneqRep &MIR) {
    std::vector<short> pureStrict;
    for (int i = 0; i < MIR.B.size(); i++) {
      TAtomRef A = MIR.rowIndexToAtom(i);
      //If the atom is known Strict...
      if (A->relop == LTOP || A->relop == GTOP || A->relop == NEOP)
        pureStrict.push_back(i);
    }
    return pureStrict;
  }


  void strictDeds(Matrix& M, std::vector<DBV>& george, vector<int>& traceRow, MonoIneqRep& MIR, std::vector<Deduction *>& deds, vector<short> pureStrict) {
    //Here, we iterate through everything that we learned by doing gaussian elimination (AKA, all rows which were of the form X, 0, 0, ..., 1, 0, ..., 0 which were not of that form before gaussian elimination)
    for (int i = 0; i < M.getNumRows(); i++) {
      short idx = M[i].getNonZero(true);
      if (idx == -1) continue;
      std::vector<TAtomRef> atoms;

      IntPolyRef dedP = MIR.reverseMap[idx]->first;
      short dedSign = (M[i].get(0) == 0 ? GTOP : LTOP);
      //std::cout << "on row " << i << ", deduced " << numToRelop(dedSign) << std::endl;

      DBV deps = george[i];
      bool strengthen = false;
      //Iterating through the dependencies of this deduction, which are identified by a "1" in deps
      for (int a = 0; a < deps.size(); a++) {
        if (deps.get(a) == 0) continue; //Indicates the row is unused for this deduction
        int j = traceRow[a];
        TAtomRef A = MIR.rowIndexToAtom(j);

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

        atoms.push_back(A);

      }
      if (strengthen == false) {
        TAtomRef A = scoreFun2(MIR, dedP, pureStrict);
        atoms.push_back(A);
      }
      FactRef F = new FactObj(atoms[0]->getPolyManagerPtr());
      F->addFactor(dedP, 1);
      TAtomRef dedAtom = new TAtomObj(F, dedSign);
      deds.push_back(new BBDed(dedAtom, atoms));
    }
  }


  /*
    Makes all deductions possible by BBSAT on the conjunction
  */
  std::vector<Deduction *> getDeductions(MonoIneqRep &MIR, const TAndRef &C) {

    std::vector<Deduction  *> deds;
    short m = MIR.B.size();
    if (m < 2) {
      return deds;
    }

    PolyManager* PM = C->getPolyManagerPtr();
    std::vector<short> pureStrict = findPureStrict(MIR);

    vector<int> traceRow;
    Matrix M = MIR.genStrictMatrix(traceRow);

    std::vector<int> piv;
    std::vector<int> piv2;
    std::vector<DBV> george = idMatrix(M);
    reducedGaussExplain(M,george,piv,piv2,true);

    strictDeds(M, george, traceRow, MIR, deds, pureStrict);
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
  */
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



  /* The dreaded scoringFunction(std::map<IntPolyRef, std::vector <TAtomRef> > candidates, std::map<IntPolyRef, bool> weakFactors)
   * Input:
   * candidates, a mapping from each factor in the formula to all the atoms which can strengthen them
   * weakFactors, all the factors in need of strengthening
   *
   * Output:
   * strength, the std::vector<IntPolyRef> which contains all the factors necessary to strengthen all the members of weakFactors
   */
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
    //Step 4: return strength.

    return toReturn;
  }




}//end namespace tarski














