#include "testsamplepointmanager.h"

#include "term.h"
#include "property.h"
#include "checks.h"

using namespace tarski;

int SamplePointManagerObj::check(SamplePointId sid, TermRef T)
{
  if (T->kind() != Term::property) return UNDET;
  prop::Tag t = T->getProperty();
  if (!isCheck(t)) return UNDET;
  return prop::check(T,this,sid);
}

int SamplePointManagerObj::factorSignAt(SamplePointId s, FactRef f)
{
  int t = f->signOfContent();
  for(auto itr = f->factorBegin(); itr != f->factorEnd(); ++itr)
  {
    int sp = polynomialSignAt(s,itr->first);
    if (sp == 0) { t = 0; break; }
    t = (itr->second % 2 == 0 ? t : t*sp);
  }
  return t;
}


