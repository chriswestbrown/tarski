#ifndef _NUCAD_CELL_TYPE_
#define _NUCAD_CELL_TYPE_

#include "nucadcell.h"
#include "collectivetermbase.h"
#include "../poly/factor.h"
#include "../shell/schemish.h"

namespace tarski {
  
class NuCADCellTypeObj; typedef GC_Hand<NuCADCellTypeObj> NuCADCellTypeRef;

class NuCADCellTypeObj : public TypeExtensionObj
{  
public:
  NuCADCellTypeObj(NuCADCellRef cell) { this->cell = cell; }

  std::string name() { return "NuCADCell"; }

  std::string shortDescrip() { return "A NuCAD Cell."; }

  std::string display() { return cell->cellDescrip(); }

  SRef refine(std::vector<SRef>& args);

  SRef refineToChild(std::vector<SRef>& args);

  SRef tst(std::vector<SRef>& args);

  SRef signAtSample(std::vector<SRef>& args);

  SRef checkBase(std::vector<SRef>& args);

  SRef dump(std::vector<SRef>& args);
  
  // BEGIN: BOILERPLATE
  static AMsgManager<NuCADCellTypeObj> _manager;
  const MsgManager& getMsgManager() { return _manager; } 
  static TypeExtensionObj::LFH<NuCADCellTypeObj> A[];
  //   END: BOILERPLATE  

 private:
  NuCADCellRef cell;
};
}//end namespace tarski
#endif
