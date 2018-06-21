#include "readSMTRetFormula.h"
#include "smtlib.h"
#include "../formula/formula.h"
#include "../formula/formmanip.h"
#include "../algparse/algparse.h"
#include "../tarskisysdep.h"
#include <stack>
#include <map>

using namespace SMTLib;
using namespace std;

namespace tarski {
  
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
void write(ExpRef e, ostream& out, LetDictionary& D);

void write(ExpRef e, ostream& out) { LetDictionary D; write(e,out,D); }

void write(ExpRef e, ostream& out, LetDictionary& D)
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
    if (letValue.is_null())
      out << lv; 
    else
      write(letValue,out,D);
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
    write(valueExp,out,D);
    for(int i = names.size()-1; i >= 0; i--)
      D.pop(names[i]->getLeadTok().getValue());
  }
  else if (v == "not") { out << "~("; write(e->get(1),out,D); out << ")"; }
  else if (v == "-" && N == 2) 
  {
    // NOTE: I'm operating under the assumption that in smtlib (- 3) should be -3.
    out << "(-("; write(e->get(1),out,D); out << "))"; 
  } 
  else if (v == "/" && N == 2) 
  {
    // NOTE: I'm operating under the assumption that in smtlib (/ 3) should be 1/3.
    out << "(1/("; write(e->get(1),out,D); out << "))"; 
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
    { out << (i > 1 ? op : string("")) << "("; write(e->get(i),out,D); out << ")"; }
  }
  else if (v == "=>")
  {
    out << "~("; write(e->get(1),out,D); out << ")\\/("; write(e->get(2),out,D); out << ")";
  }
  else if (v == "exists" || v == "forall")
  {
    out << "[" << (v == "exists" ? "ex" : "all") << " ";
    ExpRef varlist = e->get(1);
    int N = varlist->size();    
    for(int i = 0; i < N; i++)
    {
      if (i > 0 ) out << ", ";
      write(varlist->get(i)->get(0),out,D);
    }
    out << "[ ";
    write(e->get(2),out,D);
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


void readSMTRetTarskiString(std::istream& in, ostream& out)
{
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
	write(formula,out);
	out << "]";
	previousAsserts = true;
      }
      else if (e->size() == 1 && e->get(0)->matches(SMTLib::Token(SMTLib::SYM,"check-sat")))
      {
	done = true;
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
public:
  SMTWriter(ostream& _out) : out(_out) { }
  void action(TConstObj* p) 
  { 
    out << (p->constValue() == TRUE ? "true" : "false"); 
  }
  void action(TAtomObj* p) 
  { 
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
	{ out << ' '; itr->first->writeSMTLIB(*(p->getPolyManagerPtr()),out); }
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
  void action(TQBObj* p) { throw TarskiException("TFPolyFun: action unimplemented for TQBObj!"); }
};




void writeSMTLIB(TFormRef F, ostream& out)
{
  // write header
  out << "(set-logic QF_NRA)" << endl;
  out << "(set-info :source | Produced by tarski version " << tarskiVersion << "  |)" << endl;
  out << "(set-info :smt-lib-version 2.0)" << endl;
  
  // declare variables
  PolyManager* ptrPM = F->getPolyManagerPtr();
  VarSet V = F->getVars();
  for(VarSet::iterator itr = V.begin(); itr != V.end(); ++itr)
    out << "(declare-fun " << ptrPM->getName(*itr) << " () Real)" << endl;

  // write assert
  out << "(assert ";
  SMTWriter W(out);
  W(F);
  out << ")" << endl;

  // finish
  out << "(check-sat)" << endl;
}

}//end namespace tarski
