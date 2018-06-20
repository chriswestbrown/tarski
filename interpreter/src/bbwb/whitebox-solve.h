#ifndef WHITEBOX_SOLVE_MAN_H
#define WHITEBOX_SOLVE_MAN_H

#include "deduction.h"
#include "solver-manager.h"
#include "../poly/poly.h"

#include <set>
#include <utility>

namespace tarski {

  class WBSolver : public QuickSolver {
  private:
    PolyManager* PM;
    VarSet allVars;
    std::set<IntPolyRef> singleVars;
    std::set<IntPolyRef> multiVars;
    /* These three sets store all the calculations we need to do */
    std::set<pair<IntPolyRef, IntPolyRef>> singleVarsDed;
    std::set<pair<IntPolyRef, IntPolyRef>> multiVarsDed;
    std::set<IntPolyRef> polySigns;

    /* Vars to all the inequalities they appear in */
    VarKeyedMap<forward_list<IntPolyRef>> varToIneq;

    void loadVars(TAndRef taf);
    void varsToPoly();
    void populateSingleVars();
    void populateMultiVars();

    /*
      For all the variables in poly, insert the pair of that variable and poly into singleVarsDed (to perform deduce sign on it)
    */
    void saveAllVarsDed(IntPolyRef poly);
    /*
      For all the variables in poly, insert all polynomials which have a variable in common with poly into a pair with poly, and insert it into multiVarsDed to perform deduce sign on them
    */
    void saveAllPolysDed(IntPolyRef poly);
    void saveAllPolySigns(IntPolyRef poly);

    //Turns somehting thats being processed by whitebox into a deduction
    WBDed * toDed(VarKeyedMap<int> signs, VarSet v, IntPolyRef pMain, short sgn);
    WBDed * toDed(VarKeyedMap<int> signs, VarSet v, IntPolyRef pMain, IntPolyRef p2, short lsgn, short sgn2);


    IntPolyRef lastUsed;
    Deduction * doSingleDeduce();
    Deduction * doPolySigns();
    Deduction * doMultiDeduce();
    
  public:
  
    //Fills in the sets singleVars, multiVars, polysigns, and varToIneq
    WBSolver(TAndRef tf);
    Deduction * deduce(TAndRef t);
    void notify();
    void update(std::vector<Deduction *>::const_iterator begin, std::vector<Deduction *>::const_iterator end);

  
  };

  
}//end namespace tarski

#endif
