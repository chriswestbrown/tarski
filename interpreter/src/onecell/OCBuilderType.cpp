
#include "OCBuilderType.h"
namespace tarski {

  TypeExtensionObj::LFH<OCBuilderObj> OCBuilderObj::A[] = {
  {"plot2d",&OCBuilderObj::plot2d,"TO APPEAR"},
  {"pfset" ,&OCBuilderObj::pfset, "returns a list of the projection factors for this cell.  The projection factors are polynomials that are known to be sign-invariant in the cell."},
  {0,0,"function does not exist"}
 };
TypeExtensionObj::AMsgManager<OCBuilderObj> OCBuilderObj::_manager = TypeExtensionObj::AMsgManager<OCBuilderObj>();

}//end namespace tarski
