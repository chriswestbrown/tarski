#ifndef BOXER_H
#define BOXER_H

#include "deduce-sign.h"
#include "poly-explain.h"
#include "mono-explain.h"
#include "fern-poly-iter.h"
#include "deduction.h"
#include "../poly/poly.h"
#include "bbded.h"
#include "wbsat.h"
#include <set>
#include <utility>
#include <forward_list>


namespace tarski {
  class Boxer : public WBManager {
  private:
    TAndRef tand;

    //In this attempt at making BB/WB, I'm generating a new matrix each time rather than manipulating the matrix directly
    //So I need to generate a TAtomRef for everything and stick it in the TAndRef
    //Additionaly, everytime I learn a new sign I have to update the signs in the TAndRef (this is almost more complicated than would be worth it)
    std::map<IntPolyRef, TAtomRef> refToAtom;
    void updateAtom(IntPolyRef, short);

    std::vector<std::vector<TAtomRef> >  getAllExplanations{};


  public:
    Result deduceAll();
    Boxer(TAndRef &tf);
    inline int size() const {return dedM->size();}
    inline Result traceBack(int i) const { return dedM->traceBack(i);  }
    inline Result traceBack() const {return dedM->traceBack(); }


  };

}//end namespace
#endif
