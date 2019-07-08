#ifndef IDX_MAN_H
#define IDX_MAN_H
#include <vector>
#include <unordered_map>
#include "../formula/formula.h"
#include "../../../minisat/core/SolverTypes.h"

namespace tarski {
//NOTE: Currently not dynamic in the sense that after you process the conjunction initially, you can add extra definitions
  class IdxManager {
  private:
    int s;
    std::vector< tarski::TAtomRef> mapping;
    //DRBROWN: added the "ConjunctOrder" back in!  What was he thinking!
    //         we need this so that distinct Objects in memory that represent
    //         the same atom don't result in duplicate entries.
    std::map<tarski::TAtomRef, int,ConjunctOrder> revMapping;

  public:
    inline IdxManager() : s(0), mapping(0) {}
    inline int getIdx(tarski::TAtomRef t) const {
      map<tarski::TAtomRef, int>::const_iterator itr = revMapping.find(t);
      if (itr == revMapping.end()) {
        return -1;
      }
      else return itr->second;
    }
    inline void mkIdx(tarski::TAtomRef t) {
      //<<<DRBROWN ADDED: This way we don't add what's already there.  I.e. if
      //                  t is an atom that is already represented in the IdxManager
      //                  we need to do nothing, not add a duplicate!
      if (revMapping.find(t) != revMapping.end()) return;
      //>>>DRBROWN ADDED
      mapping.push_back(t);
      revMapping[t] = s;
      s++;
    }
    inline bool getAtom(unsigned int idx, tarski::TAtomRef& t) const {
      if (idx < 0) return false;
      if (idx >= mapping.size()) return false;
      t = mapping[idx];
      return true;
    }
    inline bool getAtom(Minisat::Lit l, tarski::TAtomRef& t) const {  return getAtom(var(l), t); }
    inline int size() const {return s;}
    inline void incSize() {s++;} 
    void printMapping() const  {
      auto itr = mapping.begin(), end = mapping.end();
      int i = 0;
      while (itr != end) {
        cout << i << " maps to "; (*itr)->write(); cout << endl;
        ++itr;
        i++;
      }
      std::map<tarski::TAtomRef, int>::const_iterator itr2 = revMapping.begin(), end2 = revMapping.end();
      while (itr2 != end2) {
        (itr2->first)->write(); cout << " maps to " << itr2->second << endl;
        ++itr2;
      }
    }
  };
}
#endif
