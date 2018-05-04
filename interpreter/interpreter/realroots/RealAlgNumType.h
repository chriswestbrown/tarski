#include "../shell/schemish.h"
#include "../shell/einterpreter.h"
#include "realroots.h"
#include <limits>
#ifndef _OC_REAL_ALG_NUM_TYPE_
#define _OC_REAL_ALG_NUM_TYPE_

namespace tarski {
  
class RealAlgNumTypeObj; typedef GC_Hand<RealAlgNumTypeObj> RealAlgNumTypeRef;

class RealAlgNumTypeObj : public TypeExtensionObj
{  
public:
  RealAlgNumTypeObj(RealAlgNumRef ar) { num = ar; }

  std::string name() { return "RealAlgNum"; }

  std::string shortDescrip() { return "a real algebraic number, or +/- infinity."; }

  std::string display() { return num->toStr(); }

  /* virtual SRef evalMsg(SymRef msg, std::vector<SRef>& args)  */
  /* {  */
  /*   //    Messages::call(f,name.c_str(),x); */
  /*   return TypeExtensionObj::call(*this,msg->getVal().c_str(),args); */
  /*   return new ErrObj("Message " + msg->val + " unsupported by " + name() + " objects");  */
  /* } */

  SRef approximate(std::vector<SRef>& args) 
  { 
    if (args.size() == 0) { return new ErrObj("missing argument!"); }
    NumRef r = args[0]->num();
    if (r.is_null()) { return new ErrObj("must have integer argument!"); }
    num->refineTo(RNNUM(r->getVal()));
    std::ostringstream sout; 
    sout.precision(std::numeric_limits< double >::max_digits10);
    sout << num->approximate(+1); return new StrObj(sout.str()); 
  }

  SRef chris(std::vector<SRef>& args)
  {
    IntPolyRef p = args[0]->alg()->getVal();
    RealRootIUPRef r = asa<RealRootIUPObj>(getVal());
    std::vector<RealRoot1ExtRef> V = RealRootIsolate(p, 1, r);
    LisRef L = new LisObj();
    for(int i = 0; i < V.size(); i++)
      L->push_back(new ExtObj(new RealAlgNumTypeObj(V[i])));
    return L;
  }
  
  RealAlgNumRef getVal() { return num; } 
  
  // BEGIN: BOILERPLATE
  static AMsgManager<RealAlgNumTypeObj> _manager;
  const MsgManager& getMsgManager() { return _manager; } 
  static TypeExtensionObj::LFH<RealAlgNumTypeObj> A[];
  //   END: BOILERPLATE  

 private:
  RealAlgNumRef num;
};

class RootsComm : public EICommand
{
public:
  RootsComm(NewEInterpreter* ptr) : EICommand(ptr) { }
  std::string testArgs(std::vector<SRef> &args) { return ""; }
  std::string doc() { return "TO APPEAR"; }
  std::string usage() { return "(roots <poly>)"; }
  std::string name() { return "roots"; }
  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    // TODO: need to do error catching here!
    AlgRef ar = args[0]->alg(); if (ar.is_null()) {
      return new ErrObj("roots requires a polynomial as an argument."); }
    IntPolyRef p = ar->getVal();
    std::vector<RealRootIUPRef> V;
    try { V = RealRootIsolateRobust(p); }
    catch(TarskiException e)
    {
      return new ErrObj(e.what());
    }
    LisRef L = new LisObj();
    for(unsigned int i = 0; i < V.size(); i++)
      L->push_back(new ExtObj(new RealAlgNumTypeObj(V[i])));
    return L;
  }
};

class RanCompComm : public EICommand
{
public:
  RanCompComm(NewEInterpreter* ptr) : EICommand(ptr) { }
  std::string testArgs(std::vector<SRef> &args) { return ""; }
  std::string doc() { return "TO APPEAR"; }
  std::string usage() { return "(ran-comp <RealAlgNum1> <RealAlgNum2>)"; }
  std::string name() { return "ran-comp"; }
  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    // TODO:  Need to do error catching here!
    ExtRef er0 = args[0]->ext();
    ExtRef er1 = args[1]->ext();
    RealAlgNumTypeRef t0 = er0->getExternObjPtr<RealAlgNumTypeObj>();
    RealAlgNumTypeRef t1 = er1->getExternObjPtr<RealAlgNumTypeObj>();
    RealAlgNumRef r0 = t0->getVal();
    RealAlgNumRef r1 = t1->getVal();
    int res = r0->compareTo(r1);
    return new NumObj(res);
  }
};


class NumToRanComm : public EICommand
{
public:
  NumToRanComm(NewEInterpreter* ptr) : EICommand(ptr) { }
  std::string testArgs(std::vector<SRef> &args) { return ""; }
  std::string doc() { return "TO APPEAR"; }
  std::string usage() { return "(num-to-ran <num>)"; }
  std::string name() { return "num-to-ran"; }
  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    // TODO:  Need to do error catching here!
    NumRef r = args[0]->num();
    RealAlgNumRef x = rationalToRealAlgNum(r->getVal());
    return new ExtObj(new RealAlgNumTypeObj(x));
  }
};

class IndexedRootToRanComm : public EICommand
{
public:
  IndexedRootToRanComm(NewEInterpreter* ptr) : EICommand(ptr) { }
  std::string testArgs(std::vector<SRef> &args) { return ""; }
  std::string doc() { return "TO APPEAR"; }
  std::string usage() { return "(indexedroot-to-ran <index> <poly>)"; }
  std::string name() { return "indexedroot-to-ran"; }
  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    // TODO:  Need to do error catching here!

    // get index
    NumRef id = args[0]->num();
    if (id.is_null() || id->denominator() != 1) { return new ErrObj("Index must be an integer."); }
    int i = id->numerator();

    // get polynomial & isolate roots
    AlgRef ar = args[1]->alg(); if (ar.is_null()) {
      return new ErrObj("roots requires a polynomial as an argument."); }
    IntPolyRef p = ar->getVal();
    std::vector<RealRootIUPRef> V;
    try { V = RealRootIsolateRobust(p); }
    catch(TarskiException e)
    {
      return new ErrObj(e.what());
    }
    if (i == 0) { return new ErrObj("zero not a valid root index."); }
    if (i > 0 && i <= (int)V.size()) { return new ExtObj(new RealAlgNumTypeObj(V[i-1])); }
    if (i < 0 && -i <= (int)V.size()) { return new ExtObj(new RealAlgNumTypeObj(V[V.size() + i])); }
    return new ErrObj("Invalid root index for polynomial");
  }
};
}//end namespace tarski
#endif
