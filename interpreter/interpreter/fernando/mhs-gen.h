#ifndef MHS_GEN_H
#define MHS_GEN_H
#include <vector>
#include <unordered_set>
#include "../minisat/core/SolverTypes.h"
#include "../minisat/mtl/Vec.h"

namespace Minisat {
  class MHSGenerator {
  private:
    int maxFormula; //Indicates variables larger than this are purely propositional

    //Clauses is formatted for use of Dr. Brown's HitProb class (IE, DIMACS)
    std::vector<std::vector<int > > clauses;
    std::unordered_set<int> required; //These must always be true (IE, part of a pure conjunct in the problem)

    int absDec(int i);
    Lit toLit(int i);
    void fastErase(std::vector<int>& A, int i);
    void fastErase(std::vector<std::vector<int> >& A, int i);
    int intFromLit(Lit l);
    std::unordered_set<int> genAllTrail(const vec<Lit>& trail);
    std::vector<std::vector <int> > preProcess (std::unordered_set<int>& allTrail, std::vector<int>& forceLearn);
  public:

    std::vector<Lit> genMHS(const vec<Lit>& trail);
    MHSGenerator(std::vector<std::vector <Lit> >& form, int maxForm);

    

    

  };

}
#endif
