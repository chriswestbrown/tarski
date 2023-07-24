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
#include <list>
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
  typedef std::list<DedExp> bblist;
  class BBSolver : public QuickSolver
  {
  private:
    PolyManager* PM;
    bblist deductions;
    bool once;
    MatrixManager M;
  public:

    inline BBSolver(TAndRef tf)  : deductions(0), once(true), M(tf) {
      this->PM = tf->getPolyManagerPtr();
      // cerr << "BBSolver created: "; M.write(); cerr << endl;
    }

    bool isIdempotent() { return true; }

    bblist bbsat(TAndRef t);
    DedExp deduce(TAndRef t, bool& res);
    inline void notify() {}
    void update(std::vector<Deduction>::const_iterator begin, std::vector<Deduction>::const_iterator end);
    list<DedExp> deduceTarget(std::vector<Deduction>::const_iterator begin, std::vector<Deduction>::const_iterator end);
    std::string name() const { return "BBSolver"; }
    //Searches for a row of the form 1 0 0 0 0 0 ... 0
    static int findRow(const DMatrix& d);
  };


   
  class BBChecker {
  private:
    MatrixManager * M;
    PolyManager * PM;
    int unsatRow;
    std::set<IntPolyRef> findWeak(std::forward_list<TAtomRef>& conflict);
    std::set<TAtomRef> strengthenWeak(const std::set<IntPolyRef>& weakFacts);
    std::forward_list <TAtomRef> getConflict();
    void printBeforeGauss();
    void printAfterGauss();
  public:
    bool checkSat();
    bblist explainUnsat();

    inline BBChecker(MatrixManager * m, PolyManager * pm) {
      M = m;
      PM = pm;
    }
  };

  class BBDeducer {
  private:
    MatrixManager * M;
    PolyManager * PM;
    /*===========STRICT=========*/
    void strictDeds(bblist& deds);
    int getNonZero(const std::vector<char>&);

    


    /*==========NONSTRICT=======*/
    //Nonstrict Types
    struct AtomRow {
      const vector<char> * vc;
      TAtomRef atom;
      inline AtomRow(const vector<char>& v, TAtomRef a) : vc(&v), atom(a) { }
      inline AtomRow(const AtomRow& a) : vc(a.vc), atom(a.atom) { }
    };

    struct weightCompare {
      weightCompare(int i) : cutoff(i) {}
      int cutoff;
      inline bool operator() (const AtomRow& l, const AtomRow& r) const {
        return BBDeducer::weight(l.vc, cutoff) < BBDeducer::weight(r.vc, cutoff);
      }
    };
    

    //NonStrict Methods
    vector<AtomRow> mkB(); 
    static bool isStrictRow(int cutOff, const std::vector<char>& vc);
    static int weight(const std::vector<char>* vc, int);
    static int support(const std::vector<char>*,const std::vector<char>*,int);
    static void fillMatrices(vector<AtomRow>&, vector<AtomRow>&,
                             const vector<AtomRow>&,
                             const AtomRow&, int);
    static DMatrix mkMatrix(const vector<AtomRow>&);
    static int reduceRow(AtomRow&, vector<char>&,
                          forward_list<TAtomRef>&, const DMatrix&,
                          const vector<AtomRow>&);
    void mkMinWtDed(AtomRow&,const forward_list<TAtomRef>&,
                    bblist&);
    inline void mkMinWtDed(AtomRow& a, bblist& deds)
    { mkMinWtDed(a, {a.atom}, deds); }
    void minWtMain(bblist&);
    
    MatrixManager* getMatrixManager() { return M; }
    PolyManager * getPolyManager()  { return PM; }
    
    
    //Joint Methods
    void jointDeds(bblist& deds);
    void mkJointDed(std::vector<char>&, std::vector<int>&,
                    TAtomRef, bblist&);

    //Debugging only
    void writeChar(const std::vector<char>& vc, int cutOff); 

  public:
    bblist getDeductions();
    inline BBDeducer(MatrixManager * m, PolyManager * pm) {
      M = m;
      PM = pm;
    }
    list<DedExp> attemptDeduce(std::vector<Deduction>::const_iterator begin, std::vector<Deduction>::const_iterator end);
  };

} //end namespace




#endif
