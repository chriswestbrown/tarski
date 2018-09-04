#include "Solver.h"
#include <memory>
#include <forward_list>
#include <list>
namespace tarski {
  typedef std::unique_ptr<Minisat::vec<Minisat::Lit>> vecPtr;
  typedef std::list<vecPtr> listVec;
}         
