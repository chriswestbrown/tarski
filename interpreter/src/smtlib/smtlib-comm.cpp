#include "smtlib-comm.h"
namespace tarski {
  
  TypeExtensionObj::AMsgManager<VarMapTranslateTypeObj> VarMapTranslateTypeObj::_manager = TypeExtensionObj::AMsgManager<VarMapTranslateTypeObj>();

  TypeExtensionObj::LFH<VarMapTranslateTypeObj> VarMapTranslateTypeObj::A[] = {
  {"fwd",&VarMapTranslateTypeObj::fwd,"(msg <obj> 'fwd <str>) returns sym that variable <str> was mapped to."},
  {"rev",&VarMapTranslateTypeObj::rev,"(msg <obj> 'rev <sym>) returns string that was mapped to <sym>."},
  {"add",&VarMapTranslateTypeObj::add,"(msg <obj> 'add <str> <sym>) adds <str> -> <sym> to the map."},
  {0,0,"function does not exist"}
  };


  SRef VarMapTranslateTypeObj::fwd(std::vector<SRef>& args)
  {
    if (args.size() != 1 || args[0]->type() != _str) {
      return new ErrObj("Message fwd takes a single argument of type str.");
    }
    try { 
      const string s = args[0]->str()->getVal();
      return new SymObj(vtm->fwdMap(s));
    } catch(TarskiException &e) {
      return new ErrObj(e.what());
    }
  }

  SRef VarMapTranslateTypeObj::rev(std::vector<SRef>& args)
  {
    if (args.size() != 1 || args[0]->type() != _sym) {
      return new ErrObj("Message rev takes a single argument of type sym.");
    }
    try { 
      const string s = args[0]->sym()->getVal();
      return new StrObj(vtm->revMap(s));
    } catch(TarskiException &e) {
      return new ErrObj(e.what());
    }
  }

  SRef VarMapTranslateTypeObj::add(std::vector<SRef>& args)
  {
    if (args.size() != 2 || args[0]->type() != _str || args[1]->type() != _sym) {
      return new ErrObj("Message add requires str argument followed by sym argument.");
    }
    try { 
      const string s0 = args[0]->str()->getVal();
      const string s1 = args[1]->sym()->getVal();
      getMap()->addExplicit(s0,s1);
      return new SObj;
    } catch(TarskiException &e) {
      return new ErrObj(e.what());
    }
  }

  SRef MakeVarMapTranslate::execute(SRef input, std::vector<SRef> &args) {
    const std::string msg = "make-varmap requires no argument or list of (<str> <sym>) pairs.";
    VarMapTranslateTypeRef vtm = new VarMapTranslateTypeObj(new VarTranslationMapObj(false));
    if (args.size() == 0)
      ;
    else if (args.size() != 1 || args[0]->type() != _lis) { throw TarskiException(msg); }
    else {
      LisRef L = args[0]->lis();
      for(int i = 0; i < L->length(); i++) {
	LisRef A = L->get(i)->lis();
	if (A.is_null() || A->length() != 2 || A->get(0)->type() != _str || A->get(1)->type() != _sym)
	  throw TarskiException(msg);
	vtm->getMap()->addExplicit(A->get(0)->str()->getVal(),A->get(1)->sym()->getVal());
      }
    }
    return new ExtObj(vtm);
  }

}//end namespace tarski
