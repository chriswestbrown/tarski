#ifndef _NUCAD_SECTION_
#define _NUCAD_SECTION_

#include "../poly/poly.h"


namespace tarski {
  
  class Section;
  typedef GC_Hand<Section> SectionRef;
  
  class Section : public GC_Obj
  {
  private:
    int kind;
    IntPolyRef p;
    int index;
    Section(int i, IntPolyRef q, int idx)
    {
      kind = i; p = q; index = idx;
    }
  public:
    static const int negInfty = 0, posInfty = 1, indexedRoot = 2;

    // NOTE: this requires that IntPolyRefs are all cannonical
    int compare(const SectionRef &S2) const
    {
      if (getKind() < S2->getKind()) return -1;
      if (S2->getKind() < getKind()) return +1;
      if ((void*)this < S2.vpval()) return -1;
      if (S2.vpval() < (void*)this) return +1;
      if (getIndex() < S2->getIndex()) return -1;
      if (S2->getIndex() < getIndex()) return +1;
      return 0;
    }
    bool operator<(const SectionRef &S2)
    {
      return compare(S2) < 0;
    }

    int getKind() const { return kind; }
    std::string toStr() const
    {
      std::string s = "(s ";
      switch(getKind())
      {
      case negInfty: s += "neg-infty"; break;
      case posInfty: s += "pos-infty"; break;
      case indexedRoot: s += "<poly> " + std::to_string(index); break;
      default: throw TarskiException("Error in Section: Unknown kind");
      }
      return s + ")";
    }
    std::string toStr(PolyManager* pPM) const
    {
      std::string s = "(s ";
      switch(getKind())
      {
      case negInfty: s += "neg-infty"; break;
      case posInfty: s += "pos-infty"; break;
      case indexedRoot: s += "[ " + pPM->polyToStr(p) + " ] " + std::to_string(index); break;
      default: throw TarskiException("Error in Section: Unknown kind");
      }
      return s + ")";
    }
    IntPolyRef getPoly() const { return p; }
    int getIndex() const { return index; }
    static SectionRef mkNegInfty() { return new Section(negInfty,NULL,0); }
    static SectionRef mkPosInfty() { return new Section(posInfty,NULL,0); }
    static SectionRef mkIndexedRoot(IntPolyRef p, int index)
    {
      return new Section(indexedRoot,p,index);
    }
    
  };

}

#endif

