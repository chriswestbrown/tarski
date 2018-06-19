#ifndef WBSAT_H
#define WBSAT_H


#include "deduction.h"
#include "../poly/poly.h"
#include "../shell/einterpreter.h" 
#include <set>
#include <utility>


namespace tarski {

  class WBManager {
  protected:

    Result finResult;
    bool hasRan;

    
    bool unsat;
    PolyManager* PM;
    VarSet allVars;
    //The std::set of all single variable intpolyrefs
    std::set<IntPolyRef> singleVars;
    //The std::set of all multi variable intpolyrefs
    std::set<IntPolyRef> multiVars;

    /* These three sets store all the calculations we need to do */
    std::set<pair<IntPolyRef, IntPolyRef>> singleVarsDed;
    std::set<pair<IntPolyRef, IntPolyRef>> multiVarsDed;
    std::set<IntPolyRef> polySigns;

    /* Vars to all the inequalities they appear in */
    VarKeyedMap<forward_list<IntPolyRef>> varToIneq;

    DedManager * dedM;
    
    /* Do one round of deductions*/
    virtual Deduction * doWBRound();

    /* Checks if there are no more deductions to make */
    bool notDone();

    /* Adds the new deductions that whitebox needs to make*/
    void saveDeductions();

    /*
      For all the variables in poly, insert the pair of that variable and poly into singleVarsDed (to perform deduce sign on it)
    */
    void saveAllVarsDed(IntPolyRef poly);
    /*
      For all the variables in poly, insert all polynomials which have a variable in common with poly into a pair with poly, and insert it into multiVarsDed to perform deduce sign on them
    */
    void saveAllPolysDed(IntPolyRef poly);


    void saveAllPolySigns(IntPolyRef poly);



    
    IntPolyRef lastUsed;
    WBDed * toDed(VarKeyedMap<int> signs, VarSet v, IntPolyRef pMain, short sgn) {
      vector<TAtomRef> deps;
      for (VarSet::iterator itr = v.begin(), end = v.end(); itr != end; ++itr){
        if (signs[*itr] == ALOP) continue;
        IntPolyRef p = new IntPolyObj(*itr);
        FactRef F = new FactObj(PM);
        F->addFactor(p, 1);
        TAtomRef t = new TAtomObj(F, signs[*itr]);
        deps.push_back(t);
      }
      FactRef F = new FactObj(PM);
      F->addFactor(pMain, 1);
      TAtomRef t = new TAtomObj(F, sgn);
      return new WBDed(t, deps, PSGN);
    }
    WBDed * toDed(VarKeyedMap<int> signs, VarSet v, IntPolyRef pMain, IntPolyRef p2, short lsgn, short sgn2) {
      vector<TAtomRef> deps;
      for (VarSet::iterator itr = v.begin(), end = v.end(); itr != end; ++itr){
        if (signs[*itr] == ALOP) continue;
        IntPolyRef p = new IntPolyObj(*itr);
        FactRef F = new FactObj(PM);
        F->addFactor(p, 1);
        TAtomRef t = new TAtomObj(F, signs[*itr]);
        deps.push_back(t);
      }
      FactRef F = new FactObj(PM);
      F->addFactor(pMain, 1);
      TAtomRef t = new TAtomObj(F, lsgn);
      if (sgn2 != ALOP) {
        FactRef F2 = new FactObj(PM);
        F2->addFactor(p2, 1);
        TAtomRef t2 = new TAtomObj(F2, sgn2);
        deps.push_back(t2);
      }
      return new WBDed(t, deps, DEDSGN);
    }


    
    
    WBManager();

  public:

    /* Constructor. Fills in allInequalities. Fills in the sets singleVars, multiVars, and polySigns.
     */
    WBManager(TAndRef &tf);
    virtual ~WBManager() {delete dedM;}
    /* Returns true if the formula is discovered to be unsat*/
    inline bool isUnsat() {
      return dedM->isUnsat();
    }

    /* Performs the whitebox w/ explanation algorithm 
       Returns the sign information learned by applying whitebox.
       If the formula is discovered to be UNSAT, the VarSet contains the variables whose relops implied that the formula is Unsat
    */
    virtual Result deduceAll();
    Result getLearnedSigns();
    void writeProof() { dedM->writeProof(); }
    void writeAll() {dedM->writeAll(); }

    void prettyPrintResult() {
      if (!hasRan) throw TarskiException("DeduceAll not yet called, but prettyPrintResult called!");
      std::cout << "##################################################" << std::endl;
      Result res = finResult;
      if (isUnsat()) {
	std::cout << "UNSAT\n";
	std::cout << "Explanation: "; res.write(); std::cout << std::endl << std::endl;
	std::cout << "Proof: \n";
	writeProof();
	std::cout << std::endl;
	std::cout << "All Deductions: \n";
	writeAll();
      }
      else {
	std::cout << "BBWBSAT\n";
	std::cout << std::endl;
	std::cout << "All Deductions: \n";
	writeAll();
      }
      std::cout << "##################################################" << std::endl;

    
}
    
    LisRef genLisResult() {
      Result r = deduceAll();
      LisRef l = new LisObj();
      if (isUnsat()) l->push_back(new SymObj("UNSAT"));
      else l->push_back(new SymObj("SAT"));
      vector<TAtomRef>& vec = r.atoms;
      TAndRef t = new TAndObj();
      for (vector<TAtomRef>::iterator itr = vec.begin(); itr != vec.end(); ++itr) {
	t->AND(*itr);
      }
      l->push_back(new TarObj(t));
      return l;
    }

  };

}
#endif
