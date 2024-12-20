#include "readSMTRetFormula.h"
#include "smtlib.h"
#include "../formula/formula.h"
#include "../formula/formmanip.h"
#include "../algparse/algparse.h"
#include "../tarskisysdep.h"
#include <stack>
#include <map>
#include <ctype.h>

using namespace SMTLib;
using namespace std;

namespace tarski {

  bool isValidTarskiAlgebraicVariable(const string s) {
    auto itr = s.begin();
    if (itr == s.end()) return false;
    if (!isalpha(*itr)) return false;
    while(++itr != s.end() && (isalpha(*itr) || isdigit(*itr) || *itr == '_'));
    return itr == s.end();
  }
  
  const string& VarTranslationMapObj::fwdMap(const string& var, const string& msg) {
    if (_passThroughFlag) { return var; }
    auto itr = FwdMap.find(var);
    if (itr == FwdMap.end()) {
      throw TarskiException(msg == "" ? ("No var translation for '" + var + "' ") : msg);
    }
    return itr->second;
  }
  const string& VarTranslationMapObj::revMap(const string& var) {
    if (_passThroughFlag) { return var; }
    auto itr = RevMap.find(var);
    if (itr == RevMap.end()) { throw TarskiException("No var reverse translation for '" + var + "' "); }
    return itr->second;
  }
  const string& VarTranslationMapObj::fetchOrAdd(const string& var) {
    if (_passThroughFlag) {
      if (isValidTarskiAlgebraicVariable(var))
	return var;
      throw TarskiException("Variable '" + var + "' is valid for SMT-Lib, but not for Tarski. See documentation for details on how to enable variable mapping.");
    }
    auto itr = FwdMap.find(var);
    if (itr != FwdMap.end())
      return itr->second;
    else {
      const string& mapvar =  FwdMap[var] = "x" + std::to_string(mcount++);
      RevMap[mapvar] = var;
      return mapvar;
    }
  }

  
  bool isnum(const string & s, int & countBefore, int & countAfter)
  {
    int cbefore = 0, cafter = 0, cdots = 0; 
    for(unsigned int i = 0; i < s.length(); i++)
      if ('0' <= s[i] && s[i] <= '9')
	(cdots > 0 ? cafter : cbefore)++;
      else if (s[i] == '.')
	cdots++;
      else
	return false;
    if (cdots == 0) { countBefore = cbefore; countAfter = 0; return true; }
    if (cdots == 1) { countBefore = cbefore; countAfter = cafter; return true; }
    return false;
  }


  /*****************************************************************
   * LetDictionary - this class provides means to store the variable
   * bindings resulting from evaluating let expressions.
   *****************************************************************/
  class LetDictionary
  {
  private:
    map< string,stack<ExpRef> > D;

  public:
    void push(const string& name, ExpRef value)
    {
      D[name].push(value);
    }
    ExpRef get(const string& name)
    {
      stack<ExpRef>& S = D[name];
      if (S.empty())
	return NULL;
      else
	return D[name].top();
    }
    void pop(const string& name)
    {
      D[name].pop();
    }
  };


  /*****************************************************************
   * write(e,out) - Write expression e in Tarski syntax to stream out.
   * Note: this may include divisions, which Tarski parses, but then
   * refuses to interpret as a Tarski formula.
   *****************************************************************/
  void write(ExpRef e, ostream& out, VarTranslationMapRef varTranslationMap, LetDictionary& D);

  void write(ExpRef e, ostream& out, VarTranslationMapRef varTranslationMap)
  {
    LetDictionary D;
    write(e,out,varTranslationMap,D);
  }

  void write(ExpRef e, ostream& out, VarTranslationMapRef varTranslationMap, LetDictionary& D)
  {
    Token &lt = e->getLeadTok();
    const string& lv = lt.getValue();
  
    //-- Deal with numbers (possibly floating point!)
    if (lt.getType() == NUM)
    {
      int cb, ca;
      if (isnum(lv,cb,ca)) 
      {   
	for(int i = 0; i < cb; i++)
	  out << lv[i];
	if (ca > 0)
	{
	  for(int i = 0; i < ca; i++) out << lv[cb + 1 + i];
	  out << "/1";
	  for(int i = 0; i < ca; i++) out << '0';
	}
	return; 
      }
    }
  
    //-- Deal with the only other non-list element: variables
    if (lt.getType() == SYM) 
    { 
      ExpRef letValue = D.get(lv);
      if (letValue.is_null()) {
	// auto itr = varTranslationMap.find(lv);
	// if (itr == varTranslationMap.end()) { throw TarskiException("Undeclared variable '" + lv + "' in SMTLIB input."); }
	// out << itr->second;
	out << varTranslationMap->fwdMap(lv);
      }
      else
	write(letValue,out,varTranslationMap,D);
      return; 
    }

    //-- Deal with lists
    const string& v = e->get(0)->getLeadTok().getValue();
    int N = e->size();

    if (v == "let")
    {
      // (let <nameList> <valueExp>)
      ExpRef nameList = e->get(1), valueExp = e->get(2);
      if (nameList->getLeadTok().getType() != LP)
      { throw TarskiException("SMTLib Error! arg1 of let-expression is not a list!"); }
    
      // nameList is ( (name_0 value_0) ... (name_n value_n) )
      vector<ExpRef> names, values;
      for(int i = 0; i < nameList->size(); i++)
      {
	ExpRef P = nameList->get(i);
	if (P->getLeadTok().getType() != LP)
	{ throw TarskiException("SMTLib Error! arg1 of let-expression is not a list of name-value pairs!"); }
      
	ExpRef nexp = P->get(0), vexp = P->get(1);
	if (nexp->getLeadTok().getType() != SYM) 
	{ 
	  throw TarskiException("Error! Argument 1 of a let expression non-name '" + 
				toString(nexp->getLeadTok().getType()) + "'."); 
	}
      
	names.push_back(nexp);
	values.push_back(vexp);      
      }
    
      // push the new variable bindings to D, write valueExp, pop the new variable bindings
      for(unsigned int i = 0; i < names.size(); i++)
	D.push(names[i]->getLeadTok().getValue(),values[i]);
      write(valueExp,out,varTranslationMap,D);
      for(int i = names.size()-1; i >= 0; i--)
	D.pop(names[i]->getLeadTok().getValue());
    }
    else if (v == "not") { out << "~("; write(e->get(1),out,varTranslationMap,D); out << ")"; }
    else if (v == "-" && N == 2) 
    {
      // NOTE: I'm operating under the assumption that in smtlib (- 3) should be -3.
      out << "(-("; write(e->get(1),out,varTranslationMap,D); out << "))"; 
    } 
    else if (v == "/" && N == 2) 
    {
      // NOTE: I'm operating under the assumption that in smtlib (/ 3) should be 1/3.
      out << "(1/("; write(e->get(1),out,varTranslationMap,D); out << "))"; 
    }
    else if (v == "+" || 
	     v == "-" || 
	     v == "*" || 
	     v == "/" || 
	     v == "and" || 
	     v == "or" ||
	     v == "=" || 
	     v == "!=" || 
	     v == "<" || 
	     v == ">" || 
	     v == "<=" || 
	     v == ">=")
    {
      string op = v;
      if (v == "*") { op = " "; }
      else if (v == "!=") { op = "/="; }
      else if (v == "and") { op = "/\\"; }
      else if (v == "or") { op = "\\/"; }
      for(int i = 1; i < N; i++)
      { out << (i > 1 ? op : string("")) << "("; write(e->get(i),out,varTranslationMap,D); out << ")"; }
    }
    else if (v == "=>")
    {
      out << "~("; write(e->get(1),out,varTranslationMap,D); out << ")\\/("; write(e->get(2),out,varTranslationMap,D); out << ")";
    }
    else if (v == "exists" || v == "forall")
    {
      out << "[" << (v == "exists" ? "ex" : "all") << " ";
      ExpRef varlist = e->get(1);
      int N = varlist->size();
      for(int i = 0; i < N; i++)
      {
	if (i > 0 ) out << ", ";
	ExpRef nvpair = varlist->get(i); // should be a name-value pair
	if (nvpair->size() != 2
	    || nvpair->get(0)->getLeadTok().getType() != SMTLib::SYM
	    || nvpair->get(1)->getLeadTok().getType() != SMTLib::SYM)
	{ throw TarskiException("SMT-Lib exists/forall must be followed by a list of name-value pairs!");  }
	const string& varName = nvpair->get(0)->getLeadTok().getValue();
	out << varTranslationMap->fetchOrAdd(varName);
	// string& mapvar = varTranslationMap[varName];
	// if (mapvar == "")
	//   mapvar = "x" + std::to_string(mcount++);
	// out << mapvar;
      }
      out << "[ ";
      write(e->get(2),out,varTranslationMap,D);
      out << "]]";
    }
    else
    {
      ostringstream sout;
      sout << "SMTLib: ";
      sout << "unknown operator '" << v << "' ";
      sout << "near line " << e->get(0)->getLeadTok().getLineNum() << ", column " 
	   << e->get(0)->getLeadTok().getColPos();
      throw TarskiException(sout.str());
    }
  }

  TFormRef processExpFormula(const string& instr, PolyManager* PM)
  {
    // Parse
    istringstream sin(instr);
    TarskiRef T;
    LexContext LC(sin);
    algparse(&LC,T);
    if (T.is_null() || asa<AlgebraicObj>(T)) 
    { 
      throw TarskiException("Could not parse formula!");
    }

    // Interpret as Tarski formul and return TFormRef
    try 
    { 
      MapToTForm MF(*PM);
      T->apply(MF);
      return MF.res;
    }
    catch(TarskiException &e) { 
      throw TarskiException(string("Could not interpret as Tarski Formula! ") + e.what());
    }  
  }


  TFormRef processExpFormulaClearDenominators(const string& instr, PolyManager* PM)
  {
    // Parse
    istringstream sin(instr);
    TarskiRef T;
    LexContext LC(sin);
    algparse(&LC,T);
    if (T.is_null() || asa<AlgebraicObj>(T)) 
      throw TarskiException("Could not parse formula!");

    TarskiRef Ts = clearDenominators(T);
    if (Ts.is_null())
      throw TarskiException("Could not parse formula!");

    // Interpret as Tarski formul and return TFormRef
    try 
    { 
      MapToTForm MF(*PM);
      Ts->apply(MF);
      return MF.res;
    }
    catch(TarskiException &e) { 
      throw TarskiException(string("Could not interpret as Tarski Formula! ") + e.what());
    }  
  }


  void readSMTRetTarskiString(std::istream& in, ostream& out, VarTranslationMapRef varTranslationMap)
  {
    //    map<string,string> varTranslationMap; // map original var name to saclib-safe var name
    //int mcount = 0;
  
    using namespace std;
    SMTLib::Parser P(in);
    bool previousAsserts = false, done = false;
    out << "[ ";
    while(!done)
    {
      SMTLib::ExpRef e = P.next();
      switch(e->getLeadTok().getType())
      {
      case SMTLib::END: 
	done = true;
	break;

      case SMTLib::ERR: 
	{
	  ostringstream sout;
	  sout << "SMTLib: ";
	  sout << e->getLeadTok().getValue() << " ";
	  sout << "near line " << e->getLeadTok().getLineNum() << ", column " 
	       << e->getLeadTok().getColPos();
	  P.cleanup();
	  throw TarskiException(sout.str());
	}
	break;

      case SMTLib::LP:
	if (e->size() >= 1 && e->get(0)->matches(SMTLib::Token(SMTLib::SYM,"exit")))
	{
	  done = true;
	  break;
	}
	else if (e->size() > 1 && e->get(0)->matches(SMTLib::Token(SMTLib::SYM,"assert")))
	{
	  if (previousAsserts) { out << " /\\ "; }
	  SMTLib::ExpRef formula = e->get(1);
	  out << "[";
	  write(formula,out,varTranslationMap);
	  out << "]";
	  previousAsserts = true;
	}
	else if (e->size() == 1 && e->get(0)->matches(SMTLib::Token(SMTLib::SYM,"check-sat")))
	{
	  done = true;
	}
	else if (e->size() > 1 && (
				   e->get(0)->matches(SMTLib::Token(SMTLib::SYM,"declare-fun")) ||
				   e->get(0)->matches(SMTLib::Token(SMTLib::SYM,"declare-const"))		
				   ))
	{
	  //	  string& mapvar = varTranslationMap[e->get(1)->getLeadTok().getValue()];
	  varTranslationMap->fetchOrAdd(e->get(1)->getLeadTok().getValue());
	  // if (mapvar == "")
	  //   mapvar = "x" + std::to_string(mcount++);
	}
	break;
      default:
	{
	  ostringstream sout;
	  sout << "SMTLib: expression '";
	  e->write(sout);
	  sout << "' not understood!";
	  P.cleanup();
	  throw TarskiException(sout.str());
	}
	break;
      }
    }  
    out << " ]";
    P.cleanup();
  }

  /***************************************************************
   * write SMTLIB(F,out)
   ***************************************************************/
  class SMTWriter : public TFPolyFun
  {
    ostream& out;
    VarTranslationMapRef vtm;
  public:
    SMTWriter(ostream& _out, VarTranslationMapRef _vtm) : out(_out), vtm(_vtm) { }
    void action(TConstObj* p) 
    { 
      out << (p->constValue() == TRUE ? "true" : "false"); 
    }
    void action(TAtomObj* p) 
    { 

      // SMTLIB has no neop ... must be (not (= ....))
      if (p->getRelop() == NEOP)
      {
	out << "(not ";
	TAtomObj* eqp = new TAtomObj(p->getFactors(),EQOP);
	action(eqp);
	delete eqp;
	out << ")";
	return;
      }
    
      out << '(' << relopString(p->getRelop()) << ' ';

      FactRef F = p->F;
      if (F->signOfContent() == 0)
	out << 0;
      else
      {
	map<IntPolyRef,int>::iterator itr = p->factorsBegin();
	Word c = F->getContent();
	bool contentOne = (ICOMP(c,1) == 0);
	int n = F->numFactors();
	bool noProduct = (n == 0 || (contentOne && n == 1 && itr->second == 1));

	if (!noProduct) 
	  out << "(*";

	if (!contentOne) {
	  PushOutputContext(out);
	  if (ISIGNF(c) < 0) { out << " (- "; IWRITE(INEG(c)); out << ")"; } else { IWRITE(c); } 
	  PopOutputContext();
	}      

	for(; itr != p->factorsEnd(); ++itr)
	{
	  for(int i = 0; i < itr->second; ++i)
	  { out << ' '; itr->first->writeSMTLIB(*(p->getPolyManagerPtr()),out,*vtm); }
	}
	if (!noProduct) 
	  out << ")";
      }

      out << " 0)";
    }
    void action(TExtAtomObj* p) 
    { 
      throw TarskiException("TFPolyFun: action unimplemented for TExtAtomObj!"); 
    }
    void action(TAndObj* p)
    {
      out << "(and";
      for(TAndObj::conjunct_iterator itr = p->begin(); itr != p->end(); ++itr)
      {
	out << ' ';
	actOn(*itr);
      }
      out << ")";
    }
    void action(TOrObj* p)
    {
      out << "(or";
      for(TOrObj::disjunct_iterator itr = p->begin(); itr != p->end(); ++itr)
      {
	out << ' ';
	actOn(*itr);
      }
      out << ")";
    }
    void action(TQBObj* p)
    {
      PolyManager* ptrPM = p->getPolyManagerPtr();
      for(int i = p->numBlocks()-1; i >= 0 ; i--) {
	out << "(" << (p->blockType(i) == EXIST ? "exists" : "forall") << " (";
	VarSet V = p->blockVars(i);
	for(auto itr = V.begin(); itr != V.end(); ++itr) {
	  if (itr != V.begin()) out << " ";
	  out << "(" << vtm->revMap(ptrPM->getName(*itr)) << " Real)";
	}
	out << ") ";
      }
      actOn(p->getFormulaPart());
      for(int i = 0; i < p->numBlocks(); i++)
	out << ")";
    }
  };




  // void writeSMTLIB(TFormRef F, ostream& out)
  // {
  //   std::map<string,string> fields =
  //     {
  //      {"by", ""},
  //      {"on", ""},
  //      {"tool",""},
  //      {"app",""},
  //      {"solver",""},
  //      {"pubs",""},
  //      {"license","\"https://creativecommons.org/licenses/by/4.0/\""},
  //      {"cat","\"crafted\""},
  //      {"status","unknown"}
  //     };
  //   writeSMTLIB(F,out,fields);
  // }

  void writeSMTLIB(TFormRef F, ostream& out, std::map<string,string> &fields, VarTranslationMapRef vtm)
  {
    // write header
    out << "(set-info :smt-lib-version 2.0)" << endl;
    out << "(set-logic " << (isQuantifierFree(F) ? "QF_NRA" : "NRA") << ")" << endl;

    std::string description = "";
    int ct = 0;
    if (fields["by"] != "") { description += "\nGenerated by: " + fields["by"]; ++ct; }
    if (fields["on"] != "") { description += "\nGenerated on: " + fields["on"]; ++ct; }
    if (fields["tool"] != "") { description += "\nGenerator: " + fields["tool"]; ++ct; }
    if (fields["app"] != "") { description += "\nApplication: " + fields["app"]; ++ct; }
    if (fields["solver"] != "") { description += "\nTarget solver: " + fields["solver"]; ++ct; }
    if (fields["pubs"] != "") { description += "\nPublications: " + fields["pubs"]; ++ct; }  
    out << "(set-info :source |"
	<< (ct == 0 ? string("Produced by tarski version ") + tarskiVersion : (description + "\n"))
	<< "|)" << endl;

    out << "(set-info :license " << fields["license"] << ")" << endl;
    out << "(set-info :category " << fields["cat"] << ")" << endl;
    out << "(set-info :status " << fields["status"] << ")" << endl;
  
    // declare variables
    PolyManager* ptrPM = F->getPolyManagerPtr();
    VarSet V = getFreeVars(F); //F->getVars();
    for(VarSet::iterator itr = V.begin(); itr != V.end(); ++itr)
      out << "(declare-fun " << vtm->revMap(ptrPM->getName(*itr)) << " () Real)" << endl;
  
    // write assert
    out << "(assert ";
    SMTWriter W(out,vtm);
    W(F);
    out << ")" << endl;

    // finish
    out << "(check-sat)" << endl;
  }

}//end namespace tarski
