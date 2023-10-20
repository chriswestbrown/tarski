#ifndef _SMTLIB_COMM_H_
#define _SMTLIB_COMM_H_

#include "../shell/einterpreter.h"
#include "readSMTRetFormula.h"
#include <map>

namespace tarski {

  class VarMapTranslateTypeObj; typedef GC_Hand<VarMapTranslateTypeObj> VarMapTranslateTypeRef;
  class VarMapTranslateTypeObj : public TypeExtensionObj
  {
  public:
    VarMapTranslateTypeObj(VarTranslationMapRef vtm) { this->vtm = vtm; }
    std::string name() { return "VarMapTranslate"; }    
    std::string shortDescrip() { return "translator from Tarski var names to non-Tarski var names."; }    
    std::string display() { return vtm->toStr(); }
    SRef fwd(std::vector<SRef>& args);
    SRef rev(std::vector<SRef>& args);
    SRef add(std::vector<SRef>& args);
    VarTranslationMapRef getMap() { return vtm; }
  // BEGIN: BOILERPLATE
  static AMsgManager<VarMapTranslateTypeObj> _manager;
  const MsgManager& getMsgManager() { return _manager; } 
  static TypeExtensionObj::LFH<VarMapTranslateTypeObj> A[];
  //   END: BOILERPLATE      
  private:
    VarTranslationMapRef vtm;
  };
  
  class SMTLibLoad : public EICommand
  {
  public:
    SMTLibLoad(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, std::vector<SRef> &args)
    {
      // must have arguments
      int N = args.size();
      if (N < 1) { return new ErrObj("smt-load requires an argument (the file to load)."); }

      // last argument must be file to load or smtlib string
      StrRef path = args[N-1]->str();
      if (path.is_null()) { return new ErrObj("smt-load requires its last argument to be of type std::string."); }
      ifstream fin;
      istringstream sin(path->val);
      istream *smtin; // this will point to the actual smtlib input stream
      if (path->val[0] != '(') {
	fin.open(path->val.c_str());
	if (!fin) { return new ErrObj("file '" + path->val + "' not found in smt-load."); }
	smtin = &fin;
      }
      else {
	smtin = &sin;
      }

      // process other arguments
      bool clearDenominators = false;
      bool translate = false;
      for(int i = 0; i < N-1; i++)
      {
	SymRef opt = args[i]->sym();
	if (!opt.is_null() && opt->val == "clear")
	  clearDenominators = true;
	else if (!opt.is_null() && opt->val == "translate")
	  translate = true;
	else
	  return new ErrObj("smt-load optional argument '" + args[i]->toStr() +"' not understood.");
      }
    
      try 
      {
	std::ostringstream sout;
	VarTranslationMapRef vtm = new VarTranslationMapObj(!translate);
	readSMTRetTarskiString(*smtin,sout,vtm);
	//fin.close();
	/* std::cerr << "TEST" << std::endl; */
	/* std::cerr << sout.str(); */
	/* std::cerr << "TEST" << std::endl; */
	TFormRef F;
	if (!clearDenominators)
	  F = processExpFormula(sout.str(),getPolyManagerPtr());
	else
	  F = processExpFormulaClearDenominators(sout.str(),getPolyManagerPtr());	
	if (translate) {
	  VarMapTranslateTypeRef vtmr = new VarMapTranslateTypeObj(vtm);
	  return new LisObj(new TarObj(F), new ExtObj(vtmr));
	}
	else
	  return new TarObj(F);
      } 
      catch(TarskiException &e) 
      {
	return new ErrObj(e.what());
      }
    }
    std::string testArgs(std::vector<SRef> &args) { return ""; /* require(args,_str); */ }
    std::string doc() { return "(smtlib-load [switch] <std::string>) where <std::string> is a filename or contains smtlib code and has a left parenthesis as first character, reads the file/string of smtlib input and attempts to interpret it as a Tarski formula.  When smtlib's (checksat) expression is encountered, the current formula is returned.  Note that at present very little syntax checking on the input is done.  By default, smtlib-load will not process inputs with non-constant denominators.  However, with the optional switch 'clear, smtlib-load will allow them, and clear them in the usual way but, and this is important, under the assumption that the formula \"guards\" denominators, i.e. that any branch of the formula with a denominator includes already implies the non-vanishing of that denominator.  SMT-LIB syntax has very few restrictions on the names of algebraic variables.  As a result, valid SMT-LIB variables in the input file may not be valid algebraic variables in Tarski.  By default, if such a variable occurs, an error will be returned.  To deal with this, you can give the 'translate flag, in which case smtlib-load returns a pair (F T) where F is the formula written with variables x0,x,... and T is a VarMapTranslate object.  Example:\n> (smtlib-load \"smt007.smt2\")\n\"Variable 'x!7' is valid for SMT-Lib, but not for Tarski. See documentation for details on how to enable variable mapping.\":err\n> (smtlib-load 'translate \"smt007.smt2\")\n( ex x2[2 x2^2 + 2 x1^2 + 2 x0^2 - 1 < 0 /\\ 3 x2 + 3 x1 + 3 x0 - 1 < 0]:tar (('x0,\"x!7\") ('x1,\"y?z\") ('x2,\"z\")):VarMapTranslate )"; 
    }
      std::string usage() { return "(smtlib-load [switch*] <std::string>)"; }
    std::string name() { return "smtlib-load"; }
  };


  class SMTLibStore : public EICommand
  {
  public:
    SMTLibStore(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, std::vector<SRef> &args)
    {
      std::map<string,string> fields =
	{
	 {"by", ""},
	 {"on", ""},
	 {"tool",""},
	 {"app",""},
	 {"solver",""},
	 {"pubs",""},
	 {"license","\"https://creativecommons.org/licenses/by/4.0/\""},
	 {"cat","\"crafted\""},
	 {"status","unknown"}
	};
      TarRef F = args[0]->tar();
      StrRef path = args[1]->str();
      ofstream fout(path->val.c_str());
      if (!fout) { return new ErrObj("file '" + path->val + "' not found in smt-store."); }            

      VarTranslationMapRef vmt = new VarTranslationMapObj(true); // pass through as default
      for(int k = 2; k < args.size(); k++) {
	if (args[k]->type() == _lis) {
	  LisRef L = args[k]->lis();
	  for(int i = 0; i < L->length(); i++) {
	    LisRef nvp = L->get(i)->lis();
	    if (nvp.is_null() || nvp->length() != 2) {
	      return new ErrObj("smtlib-store third argument must be list of lists (name-value pairs).");
	    }
	    SymRef name = nvp->get(0);
	    StrRef value = nvp->get(1);
	    if (name.is_null() || value.is_null()) {
	      return new ErrObj("smtlib-store third argument must be list of lists (sym-string pairs).");
	    }
	    auto itr = fields.find(name->getVal());
	    if (itr == fields.end()) { return new ErrObj("smtlib-store: unkown name '" + name->getVal() + "'"); }
	    itr->second = value->getVal();
	  }
	}
	else if (args[k]->type() == _ext && args[k]->ext()->typeName() == "VarMapTranslate") {
	  vmt = args[k]->ext()->getExternObjPtr<VarMapTranslateTypeObj>()->getMap();
	}
	else {
	  throw TarskiException("Unsupported argument type for smtlib-store!");
	}
      }
    
      writeSMTLIB(F->val, fout, fields, vmt);
      return new SObj();
    }
    std::string testArgs(std::vector<SRef> &args) {
      // string s = require(args,_tar,_str);
      // return s == "" ? s : require(args,_tar,_str,_lis);
      return "";
    }
    std::string doc() { return "\
(smt-store <tarski formula> <std::string> [<opts>+]), where <std::string> is a filename, creates a new file named\
<std::string> and writes into in in SMT-LIB to satisfiability problem for the given formula.\n\
\n\
There are two optional arguments that can be used individually or in combination.  You can \
pass a VarMapTranslate object, which smt-store will use to translate the Tarski variable \
names when writing SMT-LIB.  Another optional argument is a list of name-value pairs \
defining some or all of the following names:\n\
'by - Generated by: the name(s) of those who generated the benchmark;\n\
'on - Generated on: generation date with format YYYY-MM-DD;\n\
'tool - Generator: tool which generated the benchmark (if any);\n\
'app - Application: the general goal;\n\
'solver - Target solver: the solvers that were initially used to check the benchmarks;\n\
'pubs - Publications: references to related publications. This can be followed by any other useful information in free text.\n\
'license - defaults to https://creativecommons.org/licenses/by/4.0/\n\
'cat - <category> is either \"crafted\", indicating that it was hand-made, \"random\", indicating that it was generated randomly, or \"industrial\" (everything else). Note that the category should be in quotes.  Default is \"crafted\"\n\
'status - <status> is either sat or unsat according to the status of the benchmark, or unknown if not known.  Default is unkown.\n\
If you want to submit to the SMT-LIB, all of these should be filled.  Example:\n\
(smtlib-store [ x^2 + y^2 < 0 ] \"ex.stm2\" '((by \"C. Brown\") (on \"2021-10-20\") (tool \"N/A\") (app \"Geometry Solving\") (pubs \"N/A\") (cat \"\\\"crafted\\\"\") (status \"unsat\")))"; }
    std::string usage() { return "(smtlib-store <tarski formula> <std::string>)"; }
    std::string name() { return "smtlib-store"; }
  };

  class MakeVarMapTranslate : public EICommand
  {
    
  public:
    MakeVarMapTranslate(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, std::vector<SRef> &args);
    std::string usage() { return "(make-varmap) or (make-varmap ([(<str> <sym>)]+))"; }
    std::string name() { return "make-varmap"; }
    std::string testArgs(std::vector<SRef> &args) { return ""; }
    std::string doc() { return "Returns VarMapTranslate object, optionally initialized with list of (<str> <sym>) pairs."; }  
  };
}//end namespace tarski
#endif
