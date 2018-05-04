#ifndef _FUNC_EVAL_FORM_AT_RAT_
#define _FUNC_EVAL_FORM_AT_RAT_

#include "einterpreter.h"

namespace tarski {
  
class EvalFormAtRat : public EICommand
{
public:
  EvalFormAtRat(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, std::vector<SRef> &args)
  {
    if (args.size() < 3) { return new ErrObj("Function " + name() + " requires three arguments."); }
    if (args[0]->type() == _lis && args[1]->type() == _lis) { return executeList(input,args); }
    else { return executeSingle(input,args); }
  }
  SRef executeList(SRef input, std::vector<SRef> &args)
  {
    // pull out arguments
    LisRef L1 = args[0]->lis();
    LisRef L2 = args[1]->lis();
    TarRef F = args[2]->tar();
    if (L1->length() != L2->length()) 
    { return new ErrObj("Function " + name() + " requires first and second arguments to have the same length."); }

    // build std::map of variables to values
    PolyManager* PM = getPolyManagerPtr();
    VarKeyedMap<GCWord> value(NIL);
    for(int i = 0, n = L1->length(); i < n; i++)
    {
      SymRef x = L1->get(i)->sym(); 
      NumRef r = L2->get(i)->num(); 
      VarSet X = PM->getVar(x->val);
      value[X] = r->getVal();
    }

    return new TarObj(evalFormulaAtRationalPoint(value,F->val));
  }

  SRef executeSingle(SRef input, std::vector<SRef> &args)
  {
    SymRef x = args[0]->sym(); 
    NumRef r = args[1]->num(); 
    TarRef F = args[2]->tar();
    const std::string emsg = "Function " + name() + " expects object of type ";
    if (x.is_null()) { return new ErrObj(emsg + "sym for argument 1"); }
    if (r.is_null()) { return new ErrObj(emsg + "num for argument 2"); }
    if (F.is_null()) { return new ErrObj(emsg + "tar for argument 3"); }
    PolyManager* PM = getPolyManagerPtr();
    VarSet X = PM->getVar(x->val);
    Word R = r->val;
    /* EvalFormulaAtRational E(X,R); */
    /* E(F->val); */
    /* return new TarObj(E.res); */
    return  new TarObj(evalFormulaAtRational(X,R,F->val));
  }
  std::string testArgs(std::vector<SRef> &args) 
  { 
    std::string t1 = require(args,_sym,_num,_tar);
    return t1 == "" ? t1 : require(args,_lis,_lis,_tar);
  }
  std::string doc() { return "Evaluate a formula with rational value for a variable, or a list of rational values for a list of variables."; }
  std::string usage() { return "(evalf varname rat-num formula ) or (evalf list-varname list-rat-num formula )"; }
  std::string name() { return "evalf"; }
};
}//end namespace tarski
#endif
