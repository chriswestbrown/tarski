#ifndef CALLBACK_H
#define CALLBACK_H

#include "SolverTypes.h"
#include "Solver.h"
#include "../mtl/Vec.h"
#include <iostream>
#include <vector>
#include <climits>



namespace Minisat {
  

  /*
    Interface for all TSolvers

    - getClause : gets called after each decision
    - getAddition : gets called after each decision
                    set conf to true if there is a clause
		    to add, and false otherwise
    - getFinalClause : gets called when all variables have a value
                       on the stack
    - stackCancelNotification : gets called after "cancelUntil"
                  This is essentially notifying TSolver of retractions.
  */
  class TSolver {
  public:
    //If there is a conflict, this assumes the conflict will be in lits
    virtual void getClause(vec<Lit>& lits, bool& conf) = 0;

    //Assuems tehre will be no conflict here
    virtual void getAddition(vec<Lit>& lits, bool& conf) = 0;

    //Final Check (IE, for handing off to a complete theory solver if necessary)
    virtual void getFinalClause(vec<Lit>& lits, bool& conf) {getClause(lits, conf);}

    // callee can check qhead to see stack size after cancel
    virtual void stackCancelNotification(int newStackSize) { }

    // Pass-through functios to add vars & clauses to minisat
    virtual Var newMinisatVar() { return mini->newVar(); }
    virtual bool addMinisatClause (const vec<Lit>& ps)
    {
      return mini->addClause(ps);
    }
    // Chris added ... may regret!
    virtual void addMinisatLearntClause(vec<Lit>& ps)
    {
      mini->addClauseAddition(ps);
    }
    
    
    void printStack();

    virtual ~TSolver() {}

    TSolver() {}

    void setSolver(Solver * S) {mini = S; mini->remove_satisfied = false; }

  protected:
    Solver * mini;
    Solver* getMinisatSolver() { return mini; }
    void printData() {
      mini->printData();
    }
    const vec<Lit>& getTrail() {
      return mini->trail;
    }
    const vec<int>& getTrailLim() {
      return mini->trail_lim;
    }
    const vec<lbool>& getAssigns() {
      return mini->assigns;
    }
    inline int getQhead() {
      return mini->qhead;
    }
    inline int getNVars() {
      return mini->nVars();
    }
    //Returns the last index in trail which satisfies the current clause set
    inline int getLastSatisfying() {
      return mini->getLastSatisfying();
    }
    //given a lit in trail, returns the appropriate integer value for array indexing
    //returns INT_MAX if value is undefined
    const int getVal(Lit q) {
      while (var(q) > mini->nVars()) {
        mini->newVar();
      }
      Lit l = mkLit(var(q)); //a lit in the stack can be - or +, but what matters is its value in assigns... so i need to ensure they are all of one sign for toInt to work properly
      const vec<lbool>& assigns = getAssigns();
      if (assigns[var(l)] == l_False) {
        return (toInt(l));
      }
      else if (assigns[var(l)] == l_True) {
        return toInt(l)+1;
      }
      else return INT_MAX;
    }
    void assignWriteLit(Lit l) {
      const vec<lbool>& assigns = getAssigns();
      if (assigns[var(l)] == l_False) std::cout << "-"; else if (assigns[var(l)] == l_True) std::cout << "+";
      std::cout <<  var(l)+1;
    }
  };
  
  class UserSolver : public TSolver {
  public:
    void getClause(vec<Lit>& lits, bool& conf);
    void getAddition(vec<Lit>& lits, bool& conf);
    UserSolver() {}
    ~UserSolver() {}
  };
  
}
#endif
