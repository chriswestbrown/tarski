#include "Solver.h"
#include <memory>
#include <forward_list>
#include <list>
namespace tarski {
  typedef Minisat::vec<Minisat::Lit> mVec;
  typedef std::list<mVec> listVec;
}         
