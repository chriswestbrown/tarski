#ifndef IDX_MAN_H
#define IDX_MAN_H
#include <vector>
#include "../formula/formula.h"
#include "../minisat/core/SolverTypes.h"
namespace Minisat {
//NOTE: Currently not dynamic in the sense that after you process the conjunction initially, you can add extra definitions
  class IdxManager {
  private:
    std::vector< tarski::TAtomRef> mapping;
    std::map<tarski::TAtomRef, int, tarski::TAtomObj::OrderComp> revMapping;
    int s;
  public:
    inline IdxManager() : s(0) {}
    inline int getIdx(tarski::TAtomRef t) const {
      map<tarski::TAtomRef, int, tarski::TAtomObj::OrderComp>::const_iterator itr = revMapping.find(t);
      if (itr == revMapping.end()) {
        return -1;
      }
      else return itr->second;
    }
    inline void mkIdx(tarski::TAtomRef t) {

      mapping.push_back(t);
      revMapping[t] = s;
      s++;
    }
    inline bool getAtom(int idx, tarski::TAtomRef& t) const {
      if (idx < 0) return false;
      if (idx >= mapping.size()) return false;
      t = mapping[idx];
      return true;
    }
    inline bool getAtom(Lit l, tarski::TAtomRef& t) const {  return getAtom(var(l), t); }
    inline int size() const {return s;}
    inline void incSize() {s++;} 
    void printMapping() const  {
      std::vector<tarski::TAtomRef>::const_iterator itr = mapping.begin(), end = mapping.end();
      int i = 0;
      while (itr != end) {
        cout << i << " maps to "; (*itr)->write(); cout << endl;
        ++itr;
        i++;
      }
      std::map<tarski::TAtomRef, int, tarski::TAtomObj::OrderComp>::const_iterator itr2 = revMapping.begin(), end2 = revMapping.end();
      while (itr2 != end2) {
        (itr2->first)->write(); cout << " maps to " << itr2->second << endl;
        ++itr2;
      }
    }
  };
}
#endif
