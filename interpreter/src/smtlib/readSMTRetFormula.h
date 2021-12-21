#ifndef _READ_SMT_RET_FORMULA_
#define _READ_SMT_RET_FORMULA_
#include "../formula/formula.h"


namespace tarski {

  class VarTranslationMapObj;
  typedef GC_Hand<VarTranslationMapObj> VarTranslationMapRef;

  class  VarTranslationMapObj : public GC_Obj, public WSIRevmapper {
  private:
    std::map<std::string,std::string> FwdMap;
    std::map<std::string,std::string> RevMap;
    int mcount;
    bool _passThroughFlag;
  public:
    VarTranslationMapObj(bool passThroughFlag) : mcount(0), _passThroughFlag(passThroughFlag) { }
    const string& fwdMap(const string& var, const string& msg = "");
    const string& revMap(const string& var);
    const string& fetchOrAdd(const string& var);
    void addExplicit(const string& var0, const string& var1)
    {
      auto itr = FwdMap.find(var0);
      if (itr == FwdMap.end()) {
	FwdMap[var0] = var1;
	RevMap[var1] = var0;
      }
      else if (itr->second != var1) {
	throw TarskiException("Name '" + var0 + "' already mapped!");
      }
    }
    string toStr() const {
      if (_passThroughFlag) { return "<passthrough>"; }
      ostringstream sout;
      sout << "(";
      for(auto itr = RevMap.begin(); itr != RevMap.end(); ++itr)
	sout << (itr != RevMap.begin() ? " " : "") << "('" << itr->first << ",\"" << itr->second << "\")";
      sout << ")";
      return sout.str();
    }
  };

  
  // Read SMTLIB from input stream
  // and at the first "assert" spit out the formula
  // as a TFormRef that we're supposed to check the 
  // satisfiability of. 
  TFormRef readSMTRetFormula(std::istream& in, PolyManager* PM);

  // varTranslationMap : if this is null the original names will be used and an exception
  //                     thrown if variables don't comply with Tarski algebraic variable rules.
  //                     if non-null, variables will be translated to x0,x1,...
  void readSMTRetTarskiString(std::istream& in, std::ostream& out, VarTranslationMapRef varTranslationMap);

  TFormRef processExpFormula(const std::string& instr, PolyManager* PM);

  TFormRef processExpFormulaClearDenominators(const std::string& instr, PolyManager* PM);

  //void writeSMTLIB(TFormRef F, ostream& out);
  void writeSMTLIB(TFormRef F, ostream& out, std::map<string,string> &fields, VarTranslationMapRef vtm);

}//end namespace tarski
#endif

