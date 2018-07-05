#ifndef MHS_GEN_H
#define MHS_GEN_H
#include <vector>
#include <unordered_set>
#include "../../../minisat/core/SolverTypes.h"
#include "../../../minisat/mtl/Vec.h"

namespace tarski {
  class MHSGenerator {
  private:
    int maxFormula; //Indicates variables larger than this are purely propositional

    //Clauses is formatted for use of Dr. Brown's HitProb class (IE, DIMACS)
    std::vector<std::vector<int > > clauses;
    std::unordered_set<int> required; //These must always be true (IE, part of a pure conjunct in the problem)

    int absDec(int i);
    Minisat::Lit toLit(int i);
    void fastErase(std::vector<int>& A, int i);
    void fastErase(std::vector<std::vector<int> >& A, int i);
    int intFromLit(Minisat::Lit l);
    std::unordered_set<int> genAllTrail(const Minisat::vec<Minisat::Lit>& trail);
    std::vector<std::vector <int> > preProcess (std::unordered_set<int>& allTrail, std::vector<int>& forceLearn);
  public:

    std::vector<Minisat::Lit> genMHS(const Minisat::vec<Minisat::Lit>& trail);
    MHSGenerator(std::vector<std::vector <Minisat::Lit> >& form, int maxForm);

    

    

  };

}
#endif
