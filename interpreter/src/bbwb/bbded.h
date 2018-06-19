#ifndef _BBDED_H_
#define _BBDED_H_

#include "deduction.h"
#include "../shell/einterpreter.h"
#include "../formula/formula.h"
#include "../formula/monomialinequality.h"
#include "../poly/factor.h"
#include <vector>
#include <set>
#include <queue>
#include <map>



namespace tarski {

  class MonoIneqRep;
  class BBDed; 
  std::vector<Deduction *> bbsat(const TAndRef &F);

  /*
    If bbsat finds BBSAT, this returns all deductions which can be made in the form of a std::vector of BBDeds
  */
  std::vector<Deduction *> getDeductions(MonoIneqRep&, const TAndRef&);

  //getDeductions helper funcitons______________________________________________________________________________
  /*
    Find all the atoms which are strict, and also sets the PolyManager
  */
  std::vector<short> findPureStrict(MonoIneqRep &MIR, PolyManager* PM);


  void strictDeds(Matrix& M, std::vector<DBV>& george, MonoIneqRep& MIR, std::vector<Deduction *>& deds, std::vector<short>& pureStrict);


  //end getDeductions helper functions__________________________________________________________________________


  void printBeforeGuass(Matrix& M);
  void printAfterGauss(Matrix& M, vector<DBV>& george);





  /*
    If bbsat finds UNSAT, this returns the explanation in the form of a std::vector of BBDeds with a single BBDed which is the explanation
  */
  std::vector<Deduction *> getExplain(MonoIneqRep &MIR, const std::vector<int> &reasons, const std::vector<int> &traceRow);

  //getExplain helper functions__________________________________________________________________________________
  /*
    Determines whether each factor in MIR is weak or strong and classifies them appropriately
    Adds all the factors involved in teh conflict to a std::vector called conflict
  */
  void classifyFactors(const std::vector<int>& reasons, const std::vector<int>& traceRow, MonoIneqRep& MIR,   std::map<IntPolyRef, bool>& weakFacts, std::map<IntPolyRef, bool>& strongFacts, std::vector<TAtomRef>& conflict);
  BBDed * genDed(std::vector<TAtomRef>& additions,  std::vector<TAtomRef>& conflict);

  //end getExplain helper functions_______________________________________________________________________________


  Matrix genMatrix(MonoIneqRep, short&, std::vector<bool>&);
  Matrix genMatrix(MonoIneqRep MIR, short& maxStrict, std::vector<bool>& known, std::vector<bool>& has2);
  std::vector<TAtomRef> scoringFunction(const std::map<IntPolyRef, std::vector<TAtomRef> > &candidates, const std::map<IntPolyRef, bool> &weakFactors);

  TAtomRef scoreFun2(MonoIneqRep &MIR, IntPolyRef p, std::vector<short> pureStrict);

}//end namespace

#endif

