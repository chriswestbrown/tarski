#ifndef SOLVER_MANAGER_H
#define SOLVER_MANAGER_H


#include "deduction.h"
#include "../formula/formula.h"
#include "../shell/einterpreter.h"
#include <vector>
#include <forward_list>

namespace tarski {
  //A class which defines a solver procedure via the deduce method
  class QuickSolver
  {
  protected:
    //TODO: Whitebox requires this, so I need to get rid of this from whitebox
    DedManager * dedM;
    
  public:

    virtual ~QuickSolver() {};

    // A quicksolver is idempotent if the set of deduced facts after two consecutive calls
    // is the same as for one call.  The point of this is that each kind of solver must
    // declare itself as idempotent or not.
    virtual bool isIdempotent() = 0; 
    
    //use this method to have the solve rmake a single deduction
    //res set to false when no deduction can be made
    virtual DedExp deduce(TAndRef t, bool& res) = 0;

    //use to notify the solver if the last deduction taught something new
    //notify is called by solvermanager when the deduction manager
    //processes the Deduction * returned by deduce, and returns true
    //indicating that the deduction is used. However, it is not
    //called when that deduction causes deduction manager to deduce false,
    //as the solving process stops as soon as false is deduced
    virtual void notify() = 0;
    inline void setDedM(DedManager * d)
    {
      this->dedM = d;
    }
    virtual void update(std::vector<Deduction>::const_iterator
			begin, std::vector<Deduction>::const_iterator end) = 0;
    virtual list<DedExp> deduceTarget(std::vector<Deduction>::const_iterator begin, std::vector<Deduction>::const_iterator end) {
      std::list<DedExp> v;
      return v;
    }
    virtual std::string name() const = 0;
  };


  /*
    This class uses "quickSolver" objects to deduce some interesting information about a formula
      It solves using all the procedures given by the deduce methods of quickSolver objects, and 
    continuously loops until nothing new can be discovered
      It saves the index of the last quickSolver object to deduce something new in lastChange
    When it does a complete loop and returns to the index lastChange, it terminates, as a full
    loop with no new deductions indicates nothign new may be deduced.
      It also terminates as soon as it deduces false, AKA the formula is unsatisfiable
      Whenever a new solver is called in deduceAll, update() is first called which contains 
    iterators beginning with the deduction immediately after the last deduction which 
    was made to the end of the vector containing all deduced facts, obtained from the
    deduction manger. The solver manager stores the indices in the vector lastDeds, where
    the ith element of solvers has the last index it deduced in the ith element of lastDeds.
   */
  class SolverManager : public Orderer
  {
  private:
    std::vector<QuickSolver *> solvers; //All the solving procedures
    std::vector<short> lastDeds; //The index of the last deduction made by each solver
    TAndRef t; //The conjunction to solve, which WILL BE MODIFIED
    bool hasRan; //Indicates whether or not deduceAll has already been called
    bool hasSimplified;
    TAndRef simp;
    Result finResult; //The final result of the program
    DedManager * dedM; //The deduction manager which stores the results of each deduce()

  public:
    SolverManager(int codes, TAndRef tand);
    virtual ~SolverManager();

    static const int BB = 1;
    static const int WB = 2;
    static const int SS = 4;

    //Adds a quicksolver
    inline void addNew(QuickSolver * q) { solvers.push_back(q); solvers.back()->setDedM(dedM); }

    //Checks if the solvers have determined UNSAT 
    inline bool isUnsat() { return dedM->isUnsat(); }

    //The "main" method which loops through all the QuickSolver objects
    Result deduceAll();

    //This is its "Orderer" function
    std::vector<size_t> proxy_sorted_indices(std::vector<Deduction> &deds);
    
    TAndRef simplify(); 
    inline Result explainAtom(TAtomRef t) { return dedM->explainAtom(t);}
    inline Result explainSimp(size_t i) { return dedM->explainSimp(i); }

    //nice, human readable format with a proof and a list of all deductions
    void prettyPrintResult();
    void prettyPrintSimplify(TAndRef t);

    void debugWriteSorted()
    {
      if (solvers.size() > 0) dedM->debugWriteSorted(*this);
      else cout << "No deductions were mades!" << endl;
    }

    //returns a tarksi object containing the results of deduceAll
    LisRef genLisResult();

    void update();
    void updateSolver(const std::vector<TAtomRef>&);

    // HACK!
    TAndRef filterOut(TAndRef t);

  private:
    //return 0 if nothing has changed
    //return 1 if somehting hew has been deduced
    //return 2 if UNSAT deduced
    short deduceLoop(int i);

    //Updates a solver by getting iterators from the deduction manager
    void updateSolver(int i);


    //Assumed to be called after the SolverManager returns SAT
    //deduceOrig is used to alert each solver that they should try to deduce
    //the given atoms for purposes of simplification
    void deduceOrig();

  };
  
  
} //end namespace tarski


#endif
