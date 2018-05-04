#ifndef BOXSOLVER_H
#define BOXSOLVER_H

#include "../minisat/core/CallBack.h"
#include "../minisat/core/SolverTypes.h"
#include "../minisat/mtl/Vec.h"
#include "../formula/formula.h"
#include "mhs-gen.h"
#include "formula-maker.h"
#include "boxer.h"
#include <algorithm>
#include <climits>
#include <cmath>
#include <stack>
#include <vector>


//TODO: Add a minisat solver as a class member of this class in the constructor
//TODO: Eliminate the need to call solve from Minisat, instead call it from here
//TODO: Add the ability to detect pure conjuncts
//TODO: Add the MHS Code!

namespace Minisat {


  const short ATOM = 0;
  const short NEG = 1;
  const short AND = 2;
  const short OR  = 3;

  class FormulaMaker;
  class BoxSolver;
  class IdxManager;


  /*
    This class implements the TSolver interface
    It calls our first iteration of the BlackBox/WhiteBox
    theory solver, which runs in polynomial time but is
    non-incremental
   */
  class BoxSolver : public TSolver {

  public:
    //Intended to be called by miniSAT
    //Generates the conflict clause by applying non-incremental BB/WB
    void getClause(vec<Lit>& lits, bool& conf);
    void getFinalClause(vec<Lit>& lits, bool& conf);

    //Intended to be called by miniSAT
    //Generates all other learned clauses which are not conflicts
    //Learned by applying BB/WB
    void getAddition(vec<Lit>& lits, bool& conf);

    BoxSolver(tarski::TFormRef formula);
    virtual ~BoxSolver();

    bool solve(string&);
    short solve(); 
    //Prints the mapping of all the atoms in formula to their indices
    //Intended for debugging usage
    void printMapping() {IM->printMapping(); /*IM.printRevMapping();*/}



    //Creates and prints the CNF translation of a formula
    //Intended for debugging usage
    //Call this only after calling mkFormula;
    inline void printCNF(tarski::TFormRef formula) {
      
      for (int i = 0; i < form.size(); i++) {
        for (int j = 0; j < form[i].size(); j++) {
          write(form[i][j]); std::cout << " ";
        }
        std::cout << std::endl;
      }
    }
    
  protected:
    Solver * S;
    MHSGenerator * M;
    IdxManager * IM;
    int numAtoms;
    tarski::TAndRef genMHS();
    tarski::TFormRef formula;
    //Returns the CNF translation of a formula
    std::vector<std::vector<Lit> > makeFormula(tarski::TFormRef formula);
    bool isPureConj;
    int limit;
    int count;
    vector<vector<Lit> > form;
    tarski::PolyManager * pm;
    //If [idx] maps to an atom with sign GTOP, [idx+1] maps to the opposite LEOP. The >= sign always goes before < and != signs
    stack<stack< Lit > > learned; //to be returned with getAddition
    //this is all the necessary implication clauses as well as whats learned by BB/WB that's not the conflict
    void writeLearnedClause(vec<Lit>& lits);
    //Processes a formula and assigns indices to all the atoms
    void processAtoms(tarski::TFormRef formula);
    //Called when a conflict is discovered to translate the reason,
    //which is in tarski formula objects,
    //back into the respective minisat literals
    void constructClauses(vec<Lit>&, const tarski::Boxer&, int numDeds);
    void getClauseMain(vec<Lit>& lits, bool& conf);
    //Makes a clause from a single Result object taken from BB/WB Boxer object
    stack<Lit> mkClause(tarski::Result r);
    stack<Lit> mkClause(tarski::Result r, int idx);
    //Adds a clause of x literals to the learned clauses
    void addToLearned(Lit a, Lit b);
    void addToLearned(Lit a, Lit b, Lit c);

    bool directSolve();




    //Generates a lit from an integer index
    inline Lit litFromInt(int var, bool val);


    //given a maximum index in trail, takes all the assignments
    //in trail from the minisat solver and turns it into a
    //TAndRef by maping back to atoms
    inline tarski::TAndRef genTAnd(int maxIdx);

    //Given a vector of integers indices presumably taken from a QepcadConnection
    //And a TFormRef tand also presumably given to that QepcadConnection
    //Generate the Unsat Core as minisat Lits and put it in lits
    void getQEPUnsatCore(vec<Lit>& lits, vector<int> indices, tarski::TAndRef tand);
  };




} //end namespace




#endif

