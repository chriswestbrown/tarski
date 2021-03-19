#include "checks.h"

using namespace tarski;

bool prop::check(prop::Tag p, IntPolyRef f, SamplePointManagerRef SM, SamplePointId sid)
{
  
  switch(p)
  {
  case prop::nz_alpha:
    return SM->polynomialSignAt(sid,f) != 0;
    break;
  case prop::nnull_alpha:
    return !SM->isNullifiedAt(sid,f);
    break;
  case prop::null_alpha:
    return SM->isNullifiedAt(sid,f);
    break;
  case prop::constant:
    return f->isConstant();
    break;
  case prop::level1:
    return SM->getVarOrder()->level(f) == 1;
    break;
  default:
    throw TarskiException("Unknown property in check!");
    break;
  }
  return false;
}

bool prop::check(prop::Tag p, SectionRef s, SamplePointManagerRef SM, SamplePointId sid)
{
  
  switch(p)
  {
  case prop::above_alpha:
    return SM->compareToAlpha(sid,s) > 0;
    break;
  case prop::on_alpha:
    return SM->compareToAlpha(sid,s) == 0;
    break;
  case prop::below_alpha:
    return SM->compareToAlpha(sid,s) < 0;
    break;
  default:
    throw TarskiException("Unknown property in check!");
    break;
  }
  return false;
}

bool prop::check(prop::Tag p, SectionRef s0, SectionRef s1, SamplePointManagerRef SM, SamplePointId sid)
{
  
  switch(p)
  {
  case prop::leq_alpha:
    return SM->compareAtAlpha(sid,s0,s1) <= 0;
    break;
  case prop::lt_alpha:
    return SM->compareAtAlpha(sid,s0,s1) < 0;
    break;
  default:
    throw TarskiException("Unknown property in check!");
    break;
  }
  return false;
}


bool prop::check(TermRef T, SamplePointManagerRef SM, SamplePointId sid)
{
  const Property * p = dynamic_cast<const Property*>(&(*T));
  prop::Tag t = T->getProperty();
  if (prop::isPolynomialCheck(t))
  {
    return check(t, p->getChild(0)->getPoly(), SM, sid);
  }
  else if (prop::isSingleSectionCheck(t))
  {
    SectionRef s = p->getChild(0)->getSection();
    return check(t, s, SM, sid);
  }
  else if (prop::isDoubleSectionCheck(t))
  {
    SectionRef s0 = p->getChild(0)->getSection();
    SectionRef s1 = p->getChild(1)->getSection();
    return check(t, s0, s1, SM, sid);
  }
  throw TarskiException("unknown check!");
}

