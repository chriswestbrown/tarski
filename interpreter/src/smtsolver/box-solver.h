#ifndef BOXSOLVER_H
#define BOXSOLVER_H

#include "../../../minisat/core/CallBack.h"
#include "../../../minisat/core/SolverTypes.h"
#include "../../../minisat/mtl/Vec.h"
#include "../formula/formula.h"
#include "../bbwb/solver-manager.h"
#include "formula-maker.h"
#include "fernutils.h"
#include "mhs-gen.h"
#include <algorithm>
#include <climits>
#include <cmath>
#include <stack>
#include <vector>

namespace tarski {

  class FormulaMaker;
  class BoxSolver;
  class IdxManager;


  /*
    This class implements the TSolver interface
    It calls our first iteration of the BlackBox/WhiteBox
    theory solver, which runs in polynomial time but is
    non-incremental
   */
  class BoxSolver : public Minisat::TSolver {

  public:
    //Intended to be called by miniSAT
    //Generates the conflict clause by applying non-incremental BB/WB
    void getClause(Minisat::vec<Minisat::Lit>& lits, bool& conf);
    void getFinalClause(Minisat::vec<Minisat::Lit>& lits, bool& conf);

    //Intended to be called by miniSAT
    //Generates all other learned clauses which are not conflicts
    //Learned by applying BB/WB
    void getAddition(Minisat::vec<Minisat::Lit>& lits, bool& conf);

    BoxSolver(tarski::TFormRef formula);
    
    virtual ~BoxSolver();

    bool solve(string&);
    short solve(); 
    //Prints the mapping of all the atoms in formula to their indices
    //Intended for debugging usage
    void printMapping() {IM->printMapping(); /*IM.printRevMapping();*/}

    
  protected:
    bool unsat;
    bool ranOnce;
    SolverManager * SM;
    Minisat::Solver * S;
    IdxManager * IM;
    MHSGenerator * M;
    bool isPureConj;
    int numAtoms;
    int limit;
    int count;
    int numGetFinalClauseCalls;
    double numDNFDisjuncts; // the number of disjuncts in naive DNF on the "formula to solve" (since we may split orig!)

    TFormRef formula;
    //----------------------- right now this is set super high to essentially be full lazy
    //--------------------------------------------------------------------------------vvvv    
    // BoxSolver() : unsat(false), ranOnce(false), isPureConj(true), numAtoms(-1), limit(5000), count(0),
    // 		  numGetFinalClauseCalls(0) {};
    
    //Returns the CNF translation of a formula
    listVec makeFormula(tarski::TFormRef formula);

    int classifyConj(TAndRef T);



    std::vector<Minisat::Lit> lastVec;
    bool compareVecs(Minisat::vec<Minisat::Lit>&);

    tarski::PolyManager * pm;
    //If [idx] maps to an atom with sign GTOP, [idx+1] maps to the opposite LEOP. The >= sign always goes before < and != signs
    stack<stack< Minisat::Lit > > learned; //to be returned with getAddition
    //this is all the necessary implication clauses as well as whats learned by BB/WB that's not the conflict
    void writeLearnedClause(Minisat::vec<Minisat::Lit>& lits);

    bool doSplit(TFormRef T, int& s);
    vector<TAtomRef> splitAtom(TAtomRef t);
    TAndRef genMHS();
    //Processes a formula and assigns indices to all the atoms
    //the first version is non clause splitting
    //the second is
    void processAtoms(tarski::TFormRef formula);
    void processAtoms(tarski::TFormRef formula, tarski::TFormRef out);
    //Called when a conflict is discovered to translate the reason,
    //which is in tarski formula objects,
    //back into the respective minisat literals
    void constructClauses(Minisat::vec<Minisat::Lit>&, Result&);
    void getClauseMain(Minisat::vec<Minisat::Lit>& lits, bool& conf);
    //Makes a clause from a single Result object taken from BB/WB Boxer object
    stack<Minisat::Lit> mkClause(tarski::Result r);
    stack<Minisat::Lit> mkClause(tarski::Result r, int idx);
    //Adds a clause of x literals to the learned clauses
    void addToLearned(Minisat::Lit a, Minisat::Lit b);
    void addToLearned(Minisat::Lit a, Minisat::Lit b, Minisat::Lit c);
    bool atomFromLit(Minisat::Lit p, TAtomRef& t);
    virtual bool directSolve();


    void writeSimpToFile(TAndRef orig, TAndRef simp);

    //Generates a lit from an integer index
    inline Minisat::Lit litFromInt(int var, bool val);


    //given a maximum index in trail, takes all the assignments
    //in trail from the minisat solver and turns it into a
    //TAndRef by maping back to atoms
    tarski::TAndRef genTAnd(int maxIdx);

    //Given a vector of integers indices presumably taken from a QepcadConnection
    //And a TFormRef tand also presumably given to that QepcadConnection
    //Generate the Unsat Core as minisat Minisat::Lits and put it in lits
    void getQEPUnsatCore(Minisat::vec<Minisat::Lit>& lits, vector<int> indices, tarski::TAndRef tand);
  };




} //end namespace




#endif

