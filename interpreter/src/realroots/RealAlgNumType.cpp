#include "RealAlgNumType.h"
namespace tarski {

  TypeExtensionObj::AMsgManager<RealAlgNumTypeObj> RealAlgNumTypeObj::_manager = TypeExtensionObj::AMsgManager<RealAlgNumTypeObj>();

TypeExtensionObj::LFH<RealAlgNumTypeObj> RealAlgNumTypeObj::A[] = {
  {"approx",&RealAlgNumTypeObj::approximate,"(msg <obj> 'approx <num>) returns a string with decimal approximation of the real algebraic number <obj> refined to an interval of width 2^(<num>)."},
  {"chris",&RealAlgNumTypeObj::chris,"TEMP!"},
  {0,0,"function does not exist"}
};

}//end namespace tarski
