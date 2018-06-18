#ifndef BLACKBOX_SOLVE_MAN_H
#define BLACKBOX_SOLVE_MAN_H

#include "deduction.h"
#include "solver-manager.h"
#include <vector>
namespace tarski{


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
    
    std::vector<Deduction *> deductions;
    bool once;
  public:
    inline BBSolver(TAndRef tf) : deductions(0), once(true) {};
    Deduction * deduce(TAndRef t);
    inline void notify() {}
    void update(std::vector<Deduction *>::const_iterator begin, std::vector<Deduction *>::const_iterator end) {}
  };
} //end namespace


#endif
