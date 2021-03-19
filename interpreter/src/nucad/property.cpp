#include "property.h"
#include "goal.h"

using namespace tarski;

bool prop::isPolynomialCheck(Tag p)
{
  return nz_alpha <= p && p <= level1;
}


string prop::name(prop::Tag p)
{
  switch(p)
  {
  case prop::si: return "si";
  case prop::oi: return "oi";
  case prop::ni: return "ni";
  case prop::ana_delin: return "ana-delin";
  case prop::sector: return "sector";
  case prop::section: return "section";
  case prop::leq: return "leq";
  case prop::lt: return "lt";
  case prop::narrowed: return "narrowed";
    
  case prop::nz_alpha: return "nz-alpha";
  case prop::nnull_alpha: return "nnull-alpha";
  case prop::null_alpha: return "null-alpha";
  case prop::constant: return "constant";
  case prop::level1: return "level1";

  case prop::above_alpha: return "above-alpha";
  case prop::on_alpha: return "on-alpha";
  case prop::below_alpha: return "below-alpha";

  case prop::leq_alpha: return "leq-alpha";
  case prop::lt_alpha: return "lt-alpha";
    
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

