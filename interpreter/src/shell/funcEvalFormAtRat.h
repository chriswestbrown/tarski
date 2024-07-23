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
    else if (args[0]->type() == _sym && args[1]->type() == _num) {
      std::vector<SRef> modargs(args);
      modargs[0] = new LisObj(args[0]);
      modargs[1] = new LisObj(args[1]);
      return executeList(input,modargs);
    }
    else {
      return new ErrObj("Invalid arguments to function " + name() + ".");
    }
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

    SRef res;
    try { res = new TarObj(evalFormulaAtRationalPoint(value,F->val)); }
    catch(TarskiException e) { res = new ErrObj(e.what()); }    
    return res;
  }

  std::string testArgs(std::vector<SRef> &args) 
  { 
    std::string t1 = require(args,_sym,_num,_tar);
    return t1 == "" ? t1 : require(args,_lis,_lis,_tar);
  }
  std::string doc() { return "Evaluate a formula with rational value for a variable, or a list of rational values for a list of variables.\
For example:\n\
> (evalf '(y) '(3/2) [x^2 + x y - y^2 < z])\n\
[-1(4 z - 4 x^2 - 6 x + 9) < 0]:tar\n\
> (evalf '(y z) '(3/2 -2) [x^2 + x y - y^2 < z])\n\
[4 x^2 + 6 x - 1 < 0]:tar\n\
> (evalf '(y z x) '(3/2 -2 -1) [x^2 + x y - y^2 < z])]\n\
true:tar\n\
Note: partial evaluation of _root_ constraints is allowed as long as you do not give a value for the LHS variable without giving values for all the remaining RHS variables.\
"; }
  std::string usage() { return "(evalf varname rat-num formula ) or (evalf list-varname list-rat-num formula )"; }
  std::string name() { return "evalf"; }
};
}//end namespace tarski
#endif
