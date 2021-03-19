#ifndef _NUCAD_TERM_
#define _NUCAD_TERM_
#include "property.h"
#include "section.h"
#include <unordered_map>
#include <sstream>

namespace tarski {

/******************************************************
 * Term is the general class.  Subclasses are:
 * - Property
 * - SectionTerm
 * - PolyTerm : note must be a canonical poly!
 ******************************************************/
class TermContextObj;
typedef GC_Hand<TermContextObj> TermContextRef;

class Term;
typedef GC_Hand<Term> TermRef;

  class Term : public GC_Obj
  { 
  public:
    static const int property = 1, section = 2, poly = 3, factor = 4;
    virtual ~Term() { }
    virtual string toString(PolyManager* pPM) const { return "dummy"; }
    virtual int kind() const = 0;
    virtual int level(const VarOrderRef &VO) const = 0;
    virtual prop::Tag getProperty() const { return prop::err; }
    virtual SectionRef getSection() const { return NULL; }
    virtual IntPolyRef getPoly() const { return NULL; }
    virtual FactRef getFactor() const { return NULL; }
    virtual int comp(const Term &t) const = 0;
    virtual bool hasSectionVariable() const { return false; }
    virtual bool hasFactor() const { return false; }
  };
  
  class Property; typedef GC_Hand<Property> PropertyRef;
  class Property : public Term
  {
  private:
    prop::Tag p;
    std::vector<TermRef> child;
  public:
    Property(prop::Tag pr) : p(pr), child()  { }
    Property(prop::Tag pr, TermRef t1) : p(pr), child({t1})  { }
    Property(prop::Tag pr, TermRef t1, TermRef t2) : p(pr), child({t1,t2})  { }
    int kind() const { return Term::property; }
    int level(const VarOrderRef &VO) const
    {
      int res = 0;
      for(auto itr = child.begin(); itr != child.end(); ++itr)
	res = std::max(res,(*itr)->level(VO));
      return res;
    }
    unsigned int numChildren() const { return child.size(); }
    TermRef getChild(unsigned int i) const { return child[i]; }
    int comp(const Term &t) const
    {
      int k1 = kind(), k2 = t.kind();
      if (k1 < k2) return -1; else if (k1 > k2) return 1;
      const Property * other = dynamic_cast<const Property*>(&t);
      int p1 = this->getProperty(), p2 = other->getProperty();
      if (p1 < p2) return -1; else if (p1 > p2) return 1;
      int n1 = this->child.size(), n2 = other->child.size();
      if (n1 < n2) return -1; else if (n1 > n2) return 1;
      for(unsigned int i = 0; i < n1; i++)
      {
	int tst = this->child[i]->comp(*(other->child[i]));
	if (tst != 0) return tst;
      }
      return 0;
    }
   
    string toString(PolyManager* pPM) const
    {
      std::ostringstream sout;
      sout << "(" << prop::name(p);
      for(unsigned int i = 0; i < child.size(); i++)
	sout << " " << child[i]->toString(pPM);
      sout << ")";
      return sout.str();
    }
    prop::Tag getProperty() const { return p; }
    bool hasSectionVariable() const
    {
      bool has = false;
      for(auto itr = child.begin(); itr != child.end(); ++itr)
	has = has || (*itr)->hasSectionVariable();
      return has;
    }
    bool hasFactor() const
    {
      bool has = false;
      for(auto itr = child.begin(); itr != child.end(); ++itr)
	has = has || (*itr)->hasFactor();
      return has;
    }
  };

  class PolyTerm; typedef GC_Hand<PolyTerm> PolyTermRef;
  class PolyTerm : public Term
  {
  private:
    IntPolyRef p;
  public:
    PolyTerm(IntPolyRef pr) : p(pr) { }
    int kind() const { return Term::poly; }
    int level(const VarOrderRef &VO) const
    {
      return VO->level(p);
    }
    string toString(PolyManager* pPM) const
    {
      return "[ " + pPM->polyToStr(p) + " ]";
    }
    IntPolyRef getPoly() const { return p; }    

    int comp(const Term &b) const
    {
      int k1 = kind(), k2 = b.kind();
      if (k1 < k2) return -1; else if (k1 > k2) return 1;
      int t = this->getPoly()->compare(b.getPoly());
      return t < 0 ? -1 : (t > 0 ? 1 : 0);
    }
  };

  class FactorTerm; typedef GC_Hand<FactorTerm> FactorTermRef;
  class FactorTerm : public Term
  {
  private:
    FactRef f;
  public:
    FactorTerm(FactRef factor) : f(factor) { }
    int kind() const { return Term::factor; }
    int level(const VarOrderRef &VO) const
    {
      int res = 0;
      for(auto itr = f->factorBegin(); itr != f->factorEnd(); ++itr)
	res = std::max(res,VO->level(itr->first));
      return res;
    }
    string toString(PolyManager* pPM) const
    {
      return "[ " + f->toString() + " ]";
    }
    FactRef getFactor() const { return f; }    

    int comp(const Term &b) const
    {
      int k1 = kind(), k2 = b.kind();
      if (k1 < k2) return -1; else if (k1 > k2) return 1;
      int t = this->getFactor()->cmp(b.getFactor());
      return t < 0 ? -1 : (t > 0 ? 1 : 0);
    }
    bool hasFactor() const { return true; }
  };

  class SectionTerm; typedef GC_Hand<SectionTerm> SectionTermRef;  
  class SectionTerm : public Term
  {
  private:
    SectionRef s;
  public:
    SectionTerm(SectionRef sr) : s(sr) { }
    int kind() const { return Term::section; }
    int level(const VarOrderRef &VO) const
    {
      return s->level(VO);
    }
    string toString(PolyManager* pPM) const
    {
      return s->toStr(pPM);
    }
    SectionRef getSection() const { return s; }    
    int comp(const Term &b) const
    {
      int k1 = kind(), k2 = b.kind();
      if (k1 < k2) return -1; else if (k1 > k2) return 1;
      int t = this->getSection()->compare(b.getSection());
      return t < 0 ? -1 : (t > 0 ? 1 : 0);
    }
  };

  class SectionVariable : public SectionTerm
  {
  private:
    int identifier;
    int slevel;
  public:
    SectionVariable(int identifier, int level) : SectionTerm(NULL), identifier(identifier), slevel(level) { }
    string toString(PolyManager* pPM) const
    {
      std::ostringstream sout; sout << "(s sec_" << identifier << ")";
      return sout.str();
    }
    bool hasSectionVariable() const { return true; }    
    int level(const VarOrderRef &VO) const
    {
      return slevel;
    }
    int comp(const Term &b) const
    {
      int k1 = kind(), k2 = b.kind();
      if (k1 < k2) return -1; else if (k1 > k2) return 1;
      const Term *bp0 = &b;
      const SectionVariable * bp = dynamic_cast<const SectionVariable*>(bp0);
      if (bp == NULL)
	return 1;
      if (slevel != bp->slevel) return slevel < bp->slevel ? -1 : +1;
      return identifier < bp->identifier ? -1 : (identifier == bp->identifier ? 0 : +1);
    }
  };


}
#endif
