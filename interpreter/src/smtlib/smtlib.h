#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "../GC_System/GC_System.h"


namespace SMTLib
{
  enum TokType { ERR=0, LP, RP, SYM, LITSTR, END, NUM };
  const std::string tt2str[] = {"ERR", "LP", "RP", "SYM", "LITSTR", "END", "NUM"};
  inline const std::string& toString(TokType t) { return tt2str[t]; }
  inline std::string LPval() { return "("; }
  inline std::string RPval() { return ")"; }

  class Token
  {
    TokType _mytype;
    std::string _tokval;
    int _lineNum, _colPos; //-- gives lineNum & colPos of initial character of token
  public:
    Token() : _mytype(ERR), _tokval() { _lineNum = -1; _colPos = -1; }
    Token(TokType mytype, const std::string& tokval, int lineNum = -1, int colPos = -1) : 
      _mytype(mytype), _tokval(tokval) { _lineNum = lineNum; _colPos = colPos;  }
    TokType getType() { return _mytype; }
    const std::string & getValue() { return _tokval; }
    std::string getPrintValue() { return _mytype == LITSTR ? "|" + getValue() + "|" : getValue(); }
    int getLineNum() { return _lineNum; }
    int getColPos() { return _colPos; }
    bool matches(const Token &t) const { return _mytype == t._mytype && _tokval == t._tokval; }
    std::string posString()
    {
      std::ostringstream os;
      os << "line " << getLineNum() << ", column " << getColPos();
      return os.str();
    }
  };

  class Lexer
  {
    static const char none = EOF; 
    char lookahead;
    std::istream* pin;
    int lineNum, colPos;
  public:
    Lexer(std::istream &in) { pin = &in; lookahead = none; lineNum = 1; colPos = 1; }
    ~Lexer() { cleanup(); }
    void cleanup() { if (lookahead != none) { (*pin).putback(lookahead); lookahead = none; } } 
    Token next()
    {
      int state = 0;
      std::string collect = "";
      while(1)
      {
	// get next char
	char nc = lookahead; if (lookahead != none) lookahead = none; else nc = pin->get(); 
	colPos++;

	// deal with the comment character
	if (state != 1 && nc == ';')
	  while(nc != '\n' && nc != EOF)
	  { nc = pin->get(); colPos++; }
	
	switch(state)
	{
	case 0: 
	  switch(nc) {
	  case '(': return Token(LP,"(",lineNum,colPos);
	  case ')': return Token(RP,")",lineNum,colPos);
	  case ' ': case '\t': break;
	  case '\n': lineNum++; colPos = 1; break;
	  case '|': state = 1; break;
	  case EOF: return Token(END,"");
	  case '0': case '1': case '2': case '3': case '4': case '5': 
	  case '6': case '7': case '8': case '9': case '.':
	    state = 3; collect += nc; break;
	  default: state = 2; collect += nc; break; }	  
	  break;
	case 1:
	  // NOTE: the commented out line was the old way.  It seems that literal strings are allowed
	  //       as SYMs, so ... I made the change.
	  //	   if (nc == '|') return Token(LITSTR,collect,lineNum,colPos); else collect += nc;
	  if (nc == '|') return Token(SYM,collect,lineNum,colPos); else collect += nc;
	  break;
	case 2: 
	  switch(nc) {
	  case '(': 
	  case ')': 
	  case ' ': case '\t':
	  case '\n':
	  case '|': 
	  case EOF: lookahead = nc; return Token(SYM,collect,lineNum,colPos); break;
	  default: collect += nc; break; }	  
	  break;
	case 3:
	  switch(nc) {
	  case '0': case '1': case '2': case '3': case '4': case '5': 
	  case '6': case '7': case '8': case '9':
	    collect += nc; break;
	  case '.': state = 4; collect += nc; break; 
	  default: lookahead = nc; return Token(NUM,collect,lineNum,colPos); break; }
	  break;
	case 4:
	  switch(nc) {
	  case '0': case '1': case '2': case '3': case '4': case '5': 
	  case '6': case '7': case '8': case '9':
	    collect += nc; break;
	  case '.': collect += nc; return Token(ERR,collect,lineNum,colPos); break;
	  default: lookahead = nc; return Token(NUM,collect,lineNum,colPos); break; }
	  break;
	}
      }
    }
  };

  /*********************************************************
   * Exp: an "error" Exp has leadTok std::set to ERR, and its 
   * line number and column number should indicate the position
   * of the error.
   *********************************************************/
  class ExpObj; typedef GC_Hand<ExpObj> ExpRef;

  class ExpObj : public GC_Obj
  {
    Token leadTok;
    std::vector<ExpRef> elements;
  public:
    ExpObj(const Token &t) { leadTok = t; }
    void push(ExpRef e) { elements.push_back(e); }
    int size() { return elements.size(); }
    ExpRef get(int i) 
    {
      if (i >= (int)elements.size())
      {
	std::ostringstream os;
	os << "SMTlib-Exp-get: invalid index '" << i << "' in expression starting at "<< leadTok.posString();
	throw tarski::TarskiException(os.str());
      }
	return elements[i]; 
    }
    bool matches(TokType tt) { return leadTok.getType() == tt; }
    bool matches(Token t) { return leadTok.matches(t); }
    Token& getLeadTok() { return leadTok; }
    void setLeadTok(const Token &t) { leadTok = t; } // use with care!
    void write(std::ostream& out)
    {
      //      std::cerr << "Writing Exp: token type = " << leadTok.getType() << " num elements = " << elements.size() << std::endl;      
     if (leadTok.getType() != LP)
       out << leadTok.getPrintValue();
     else
     {
       out << LPval();
       for(std::vector<ExpRef>::iterator i = elements.begin(); i != elements.end(); i++)
       {
	 if (i != elements.begin()) out << ' ';
	 (*i)->write(out);
       }
       out << RPval();
     }
    }
  };
  
  class Parser
  {
  private:
    Lexer lexer;
    Token lookahead;
    bool hasLookahead;
    Token getNextToken()
    {
      if (hasLookahead) { hasLookahead = false; return lookahead; }
      //      return lexer.next();
      Token tmp = lexer.next();
      /* std::cerr << toString(tmp.getType()) << ":" << tmp.getValue() << " "  */
      /* 	   << tmp.getLineNum() << ":" << tmp.getColPos() << std::endl; */
      return tmp;
    }
    Token peekNextToken()
    {
      if (!hasLookahead) { lookahead = lexer.next(); hasLookahead = true; }
      return lookahead;
    }
    void putbackToken(Token t)
    {
      if (hasLookahead) { std::cerr << "Error in SMTLIB Parser!" << std::endl; exit(1); }
      else { lookahead = t; hasLookahead = true; }
    }
  public:
    Parser(std::istream& in) : lexer(in) { hasLookahead = false; }
    ExpRef next()
    {
      Token nextTok = getNextToken();
      switch(nextTok.getType())
      {
      case LP: {
	ExpRef ep = new ExpObj(nextTok);
	while(true)
	{
	  nextTok = getNextToken();
	  if (nextTok.getType() == RP) return ep;
	  else if (nextTok.getType() == END) 
	  { 
	    ep->setLeadTok(Token(ERR,"Unexpected end of file while reading list!",
				 nextTok.getLineNum(),nextTok.getColPos()));
	    return ep; 
	  }
	  else if (nextTok.getType() == ERR)
	  {
	    ep->setLeadTok(nextTok);
	    return ep;
	  }
	  else
	  {
	    putbackToken(nextTok);
	    ExpRef nextep = next();
	    if (nextep->getLeadTok().getType() == ERR) { return ep; }
	    if (nextep->getLeadTok().getType() == END) 
	    {
	      ep->push(nextep);
	      ep->setLeadTok(Token(ERR,"Unexpected end of file while reading list!",
				   nextep->getLeadTok().getLineNum(),nextep->getLeadTok().getColPos()));
	      return ep; 
	    }
	    ep->push(nextep);
	  }	  
	}
      } break;
      case RP: return new ExpObj(Token(ERR,"Unexpected ')'!",nextTok.getLineNum(),
				       nextTok.getColPos())); break;
      default: return new ExpObj(nextTok); break;
      }
      throw tarski::TarskiException("Error! Unexpected condition in SMTLIB parser!");
    }
    void cleanup() { lexer.cleanup(); }
    ~Parser() { cleanup(); }
  };
}

inline std::ostream& operator<<(std::ostream& out, SMTLib::TokType t)
{
  return out << SMTLib::toString(t);
}

