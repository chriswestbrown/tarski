#ifndef _UNINTERP_FORM_
#define _UNINTERP_FORM_

#include "../shell/schemish.h"
#include "bexpr.h"

namespace tarski {

class UifObj : public SObj
{
 private:
  BExpRef E;
  PolyManager* pM;
  
 public:
  UifObj(TarskiRef T, PolyManager* pM);
  virtual int type() { return _uif; }
  std::string typeName() { return "uif"; }
  UifRef uif() { return this; }
  std::string toStr() { return E->str() + ":uif"; }
  void display(std::ostream& out) { out << E->str(); }
  BooRef equal(SRef x) { return new BooObj(false); }
  BExpRef getValue() { return E; }

  // clear denominators!
  // clearProcess: 0 <- from my paper, 1 <- no guarding, 2 <- naive way (den /= 0)
  SRef clear(int clearProcess);

};
}
#endif
    
