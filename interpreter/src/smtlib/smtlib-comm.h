#ifndef _SMTLIB_COMM_H_
#define _SMTLIB_COMM_H_

#include "../shell/einterpreter.h"
#include "readSMTRetFormula.h"

namespace tarski {
  
class SMTLibLoad : public EICommand
{
public:
  SMTLibLoad(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, std::vector<SRef> &args)
  {
    // must have arguments
    int N = args.size();
    if (N < 1) { return new ErrObj("smt-load requires an argument (the file to load)."); }

    // last argument must be file to load
    StrRef path = args[N-1]->str();
    if (path.is_null()) { return new ErrObj("smt-load requires its last argument to be of type std::string."); }
    ifstream fin(path->val.c_str());
    if (!fin) { return new ErrObj("file '" + path->val + "' not found in smt-load."); }            
    
    // process other arguments
    bool clearDenominators = false;
    for(int i = 0; i < N-1; i++)
    {
      SymRef opt = args[i]->sym();
      if (!opt.is_null() && opt->val == "clear")
	clearDenominators = true;
      else
	return new ErrObj("smt-load optional argument '" + args[i]->toStr() +"' not understood.");
    }
    
    try 
    {
      std::ostringstream sout;
      readSMTRetTarskiString(fin,sout);
      fin.close();
      /* std::cerr << "TEST" << std::endl; */
      /* std::cerr << sout.str(); */
      /* std::cerr << "TEST" << std::endl; */
      TFormRef F;
      if (!clearDenominators)
	F = processExpFormula(sout.str(),getPolyManagerPtr());
      else
	F = processExpFormulaClearDenominators(sout.str(),getPolyManagerPtr());
      return new TarObj(F);
    } 
    catch(TarskiException &e) 
    {
      return new ErrObj(e.what());
    }
  }
  std::string testArgs(std::vector<SRef> &args) { return ""; /* require(args,_str); */ }
  std::string doc() { return "\
(smt-load [switch] <std::string>) where <std::string> is a filename, reads file <std::string> of smtlib input and attempts to interpret it as a Tarski formula.  When smtlib's (checksat) expression is encountered, the current formula is returned.  Note that at present very little syntax checking on the input is done.  By default, smtlib-load will not process inputs with non-constant denominators.  However, with the optional switch 'clear, smtlib-load will allow them, and clear them in the usual way but, and this is important, under the assumption that the formula \"guards\" denominators, i.e. that any branch of the formula with a denominator includes already implies the non-vanishing of that denominator."; }
  std::string usage() { return "(smtlib-load [switch] <std::string>)"; }
  std::string name() { return "smtlib-load"; }
};


class SMTLibStore : public EICommand
{
public:
  SMTLibStore(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, std::vector<SRef> &args)
  {
    TarRef F = args[0]->tar();
    StrRef path = args[1]->str();
    ofstream fout(path->val.c_str());
    if (!fout) { return new ErrObj("file '" + path->val + "' not found in smt-store."); }            
    writeSMTLIB(F->val, fout);
    return new SObj();
  }
  std::string testArgs(std::vector<SRef> &args) { return require(args,_tar,_str); }
  std::string doc() { return "\
(smt-store <tarski formula> <std::string>), where <std::string> is a filename, creates a new file named\
<std::string> and writes into in in SMT-LIB to satisfiability problem for the given formula."; }
  std::string usage() { return "(smtlib-store <tarski formula> <std::string>)"; }
  std::string name() { return "smtlib-store"; }
};
}//end namespace tarski
#endif
