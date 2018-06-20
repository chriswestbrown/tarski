#include "property.h"
#include "goal.h"

using namespace tarski;

bool prop::check(prop::Tag p, GoalContextRef GC, IntPolyRef f)
{
  SamplePointManagerRef SM = GC->getSamplePointManager();
  SamplePointId sid = GC->getSamplePointId();
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
    return GC->getVarOrder()->level(f) == 1;
    break;
  default:
    throw TarskiException("Unknown property in check!");
    break;
  }
  return false;
}

string prop::name(prop::Tag p)
{
  switch(p)
  {
  case prop::si: return "si";
  case prop::oi: return "oi";
  case prop::ni: return "ni";
  case prop::ana_delin: return "ana-delin";
  case prop::narrowed: return "narrowed";
    
  case prop::nz_alpha: return "nz-alpha";
  case prop::nnull_alpha: return "nnull-alpha";
  case prop::null_alpha: return "null-alpha";
  case prop::constant: return "constant";
  case prop::level1: return "level1";

  case prop::si_s: return "si*";
  case prop::oi_s: return "oi*";
  case prop::ni_s: return "ni*";

  default:
    throw TarskiException("Unknown property in name!");
    break;
  }  
}

prop::Tag prop::nameToProp(const string& str)
{
  int i = 0;
  while(i < prop::err && name((prop::Tag)i) != str)
    i++;
  return (prop::Tag)i;
}

