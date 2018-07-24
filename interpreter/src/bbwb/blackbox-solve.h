#ifndef BLACKBOX_SOLVE_MAN_H
#define BLACKBOX_SOLVE_MAN_H

#include "deduction.h"
#include "solver-manager.h"
#include "shell/einterpreter.h"
#include "../formula/formula.h"
#include "../formula/monomialinequality.h"
#include "matrix-manager.h"
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
    std::vector<Deduction *> deductions;
    bool once;
    MatrixManager M;

  public:
    inline BBSolver(TAndRef tf)  : deductions(0), once(true), M(tf) {
      this->PM = tf->getPolyManagerPtr();
    }
    std::vector<Deduction *> bbsat(TAndRef t);
    Deduction * deduce(TAndRef t);
    inline void notify() {}
    void update(std::vector<Deduction *>::const_iterator begin, std::vector<Deduction *>::const_iterator end);
  };


   
  class BBChecker {
  private:
    MatrixManager * M;
    PolyManager * PM;
    int unsatRow;
    std::set<IntPolyRef> findWeak(std::vector<TAtomRef>& conflict);
    std::set<TAtomRef> strengthenWeak(const std::set<IntPolyRef>& weakFacts);
    std::vector<TAtomRef> getConflict();
    void printBeforeGauss();
    void printAfterGauss();
  public:
    bool checkSat();
    vector<Deduction *> explainUnsat();
    inline BBChecker(MatrixManager * m, PolyManager * pm) {
      M = m;
      PM = pm;
    }
  };

  class BBDeducer {
  private:
    MatrixManager * M;
    PolyManager * PM;
    //Pure Strict
    void strictDeds(std::vector<Deduction *>& deds);
    int getNonZero(const std::vector<char>&);

    //NonStrict Methods
    void minWtExplain(std::vector<Deduction *>& deds);

  public:
    vector<Deduction *> getDeductions();
    inline BBDeducer(MatrixManager * m, PolyManager * pm) {
      M = m;
      PM = pm;

    }
  };

} //end namespace




#endif
