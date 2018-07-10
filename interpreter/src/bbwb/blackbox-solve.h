#ifndef BLACKBOX_SOLVE_MAN_H
#define BLACKBOX_SOLVE_MAN_H

#include "deduction.h"
#include "solver-manager.h"
#include "shell/einterpreter.h"
#include "../formula/formula.h"
#include "../formula/monomialinequality.h"
#include "../poly/factor.h"
#include <vector>
#include <set>
#include <queue>
#include <map>
namespace tarski{


  class MonoIneqRep;
  class BBDed;
  //NOTE: Deprecated. Practical experience has shwon that there is frequently very little choice in strengthening factors.
  std::vector<TAtomRef> scoringFunction(const std::map<IntPolyRef, std::vector<TAtomRef> > &candidates, const std::map<IntPolyRef, bool> &weakFactors);
  Matrix genMatrix(MonoIneqRep, short&, std::vector<bool>&);
  Matrix genMatrix(MonoIneqRep MIR, short& maxStrict, std::vector<bool>& known, std::vector<bool>& has2);


  /*
    A class for doing blackbox deductions
    Since blackbox is implemented as returning a vector 
    of all the deductions it can make at once, this class
    simulates making deductions one at a time by storing
    that vector and giving them back one at a time with 
    every call to deduce.
    When the vector size is 0 and deduce is called,
    it returns NULL only once. The next time we call 
    deduce, a new conjunction t is assumed, and BBSolver 
    performs BlackBox on t.
  */
  class BBSolver : public QuickSolver {
  private:
    PolyManager* PM;
    std::map<IntPolyRef, TAtomRef> polyToSIneq;
    std::vector<Deduction *> deductions;
    bool once;
    MonoIneqRep * MIR;


    void populatePoly(TAndRef t);
    void populatePoly(TAtomRef A);

  public:
    inline BBSolver(TAndRef tf)  : deductions(0), once(true) {
      this->PM = tf->getPolyManagerPtr();
      populatePoly(tf);
    }
    std::vector<Deduction *> bbsat(TAndRef t);
    Deduction * deduce(TAndRef t);
    inline void notify() {}
    void update(std::vector<Deduction *>::const_iterator begin, std::vector<Deduction *>::const_iterator end) {
      while (begin != end) {
        populatePoly((*begin)->getDed());
        ++begin;
      }
    }
  };


   
  class BBChecker {
  private:
    MonoIneqRep * MIR;
    PolyManager * PM;
    std::map<IntPolyRef, TAtomRef> * polyToSIneq;
    std::vector<int> reasons;
    std::vector<int> traceRow;
    std::set<IntPolyRef> findWeak(std::vector<TAtomRef>& conflict);
    std::set<TAtomRef> strengthenWeak(const std::set<IntPolyRef>& weakFacts);
    std::vector<TAtomRef> getConflict();
    void printMIR();
    void printBeforeGauss(Matrix& M);
    void printAfterGauss(Matrix& M, vector<DBV>& george);
  public:
    bool checkSat();
    vector<Deduction *> explainUnsat();
    inline BBChecker(MonoIneqRep * m, std::map<IntPolyRef, TAtomRef> * pToS, PolyManager * pm) {
      MIR = m;
      polyToSIneq = pToS;
      PM = pm;
    }
  };

  class BBDeducer {
  private:
    MonoIneqRep * MIR;
    std::map<IntPolyRef, TAtomRef> * polyToSIneq;
    PolyManager * PM;
    void strictDeds(Matrix& M, std::vector<DBV>& george, vector<int>& traceRow, std::vector<Deduction *>& deds);
  public:
    vector<Deduction *> getDeductions();
    inline BBDeducer(MonoIneqRep * m, std::map<IntPolyRef, TAtomRef> * pToS, PolyManager * pm) {
      MIR = m;
      polyToSIneq = pToS;
      PM = pm;

    }
  };
  /* FUTURE
     class MinWtExp;
  */

} //end namespace




#endif
