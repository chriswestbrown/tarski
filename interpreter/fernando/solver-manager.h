
#include "deduction.h"
#include "../formula/formula.h"
#include <vector>

namespace tarski {
  //A class which defines a solver procedure via the deduce method
  class QuickSolver {
  protected:
    DedManager * dedM;
    
  public:
    //use this method to have the solve rmake a single deduction
    //returns NULL when no deduction can be made!
    virtual Deduction * deduce(TAndRef t) = 0;
    
    //use to notify the solver if the last deduction taught something new
    //notify is called by solvermanager when the deduction manager
    //processes the Deduction * returned by deduce, and returns true
    //indicating that the deduction is used. However, it is not
    //called when that deduction causes deduction manager to deduce false,
    //as the solving process stops as soon as false is deduced
    virtual void notify() = 0;

    void setDedM(DedManager * d) { dedM = d; }
  };


  /*
    This class uses "quickSolver" objects to deduce some interesting information about a formula
    It solves using all the procedures given by the deduce methods of quickSolver objects, and 
    continuously loops until nothing new can be discovered
    It saves the index of the last quickSolver object to deduce something new in lastChange
    When it does a complete loop and returns to the index lastChange, it terminates, as a full
    loop with no new deductions indicates nothign new may be deduced.
    It also terminates as soon as it deduces false, AKA the formula is unsatisfiable
   */
  class SolverManager {
  private:
    std::vector<QuickSolver *> solvers; //All the solving procedures
    int lastChange; //The index of the last solver which deduced something new
    TAndRef t; //The conjunction to solve, which WILL BE MODIFIED
    bool hasRan;
    Result finResult;
    DedManager dedM;
  public:
    SolverManager(vector<QuickSolver *>& v, TAndRef tand);

    //Adds a quicksolver
    inline void addNew(QuickSolver * q) { solvers.push_back(q); }
    //Checks if the solvers have determined UNSAT 
    inline bool isUnsat() { return dedM.isUnsat(); }
  
  //The "main" method which loops through all the QuickSolver objects
  Result deduceAll();
  
  //return 0 if nothing has changed
  //return 1 if somehting hew has been deduced
  //return 2 if UNSAT deduced
  short deduceLoop(QuickSolver * q);

  
  void prettyPrintResult();
  
  };
  
  
} //end namespace tarski