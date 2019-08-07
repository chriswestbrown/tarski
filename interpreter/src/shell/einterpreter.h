#ifndef _CWBEINTERP_
#define _CWBEINTERP_
#include "schemish.h"

namespace tarski {
  
class NewEInterpreter;

class EICommand
{
protected:
    NewEInterpreter* interp;
public:
  EICommand(NewEInterpreter* ptr) : interp(ptr) { }
  virtual ~EICommand() { }
  PolyManager* getPolyManagerPtr();
  virtual SRef eval(SRef input, std::vector<SRef> &args)
  {
    std::string err = testArgs(args);
    if (err != "") 
    { 
      return new ErrObj(name() + ": " + err + " usage: " + usage()); 
    }
    else { 
      SRef res = execute(input,args); 
      return res;
    }
  }
  virtual SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    return new ErrObj("For " + name() + " execute undefined!"); 
  }
  virtual std::string testArgs(std::vector<SRef> &args) = 0;
  virtual std::string doc() = 0;
  virtual std::string usage() = 0;
  virtual std::string name() = 0;
  std::string require(std::vector<SRef> &args, int typeReq)
  {
    if (args.size() != 1) { return "Wrong number of arguments."; }
    if (args[0]->type() != typeReq) { return "Type mismatch in argument."; }
    return "";
  }
  std::string require(std::vector<SRef> &args, int typeReq1, int typeReq2)
  {
    if (args.size() != 2) { return "Wrong number of arguments."; }
    if (args[0]->type() != typeReq1) { return "Type mismatch in argument 1."; }
    if (args[1]->type() != typeReq2) { return "Type mismatch in argument 2."; }
    return "";
  }
  std::string require(std::vector<SRef> &args, int typeReq1, int typeReq2, int typeReq3)
  {
    if (args.size() != 3) { return "Wrong number of arguments."; }
    if (args[0]->type() != typeReq1) { return "Type mismatch in argument 1."; }
    if (args[1]->type() != typeReq2) { return "Type mismatch in argument 2."; }
    if (args[2]->type() != typeReq3) { return "Type mismatch in argument 3."; }
    return "";
  }
  std::string requirepre(std::vector<SRef> &args, int typeReq)
  {
    if (args.size() < 1) { return "Wrong number of arguments."; }
    if (args[0]->type() != typeReq) { return "Type mismatch in argument."; }
    return "";
  }
  std::string requirepre(std::vector<SRef> &args, int typeReq1, int typeReq2)
  {
    if (args.size() < 2) { return "Wrong number of arguments."; }
    if (args[0]->type() != typeReq1) { return "Type mismatch in argument 1."; }
    if (args[1]->type() != typeReq2) { return "Type mismatch in argument 2."; }
    return "";
  }
  std::string requirepre(std::vector<SRef> &args, int typeReq1, int typeReq2, int typeReq3)
  {
    if (args.size() < 3) { return "Wrong number of arguments."; }
    if (args[0]->type() != typeReq1) { return "Type mismatch in argument 1."; }
    if (args[1]->type() != typeReq2) { return "Type mismatch in argument 2."; }
    if (args[2]->type() != typeReq3) { return "Type mismatch in argument 3."; }
    return "";
  }

};


class NewEInterpreter : public Interpreter
{
public:
  static const int OFFSET = 10;
  std::vector<EICommand*> X;
  std::vector< GC_Hand<TypeExtensionObj> > typeCache;
  void add(EICommand* t)
  { 
    (*rootFrame)[t->name()] = new FunObj(-X.size()); 
    X.push_back(t);
  }
  void addType(GC_Hand<TypeExtensionObj> typeRef)
  {
    typeCache.push_back(typeRef);
  }
public:
  NewEInterpreter(PolyManager *ptrPM) : Interpreter(ptrPM), X(OFFSET) { }
  virtual ~NewEInterpreter() { for(unsigned int i = 0; i < X.size(); ++i) delete X[i]; }
  void init();
  SRef exFunEval(SRef input, int funcode,std::vector<SRef> &args) 
  { 
    return X[-funcode]->eval(input,args); 
  }
private:

};

inline PolyManager* EICommand::getPolyManagerPtr() { return interp->getPolyManagerPtr(); }
}//end namespace tarski
#endif
