#include "alglex.h"
#include "grammar.tab.hpp"

namespace tarski {
void LexContext::getNext(char &ic, int &iline, int &icolumn)
{
  // Get next character, line & column
  iline = line[bufIndex(PosNext)];
  icolumn = column[bufIndex(PosNext)];
  if (pbCount == 0) { ic = symbol[bufIndex(PosNext)] = tsFilter(InPtr->get()); }    
  else { ic = symbol[bufIndex(PosNext)]; --pbCount; }
  
  // Update "Next" info
  ++PosNext;
  if (ic == '\n') { line[bufIndex(PosNext)] = iline + 1; column[bufIndex(PosNext)] = 1; }
  else { line[bufIndex(PosNext)] = iline; column[bufIndex(PosNext)] = icolumn + 1; }
}

// Returns symbol,line,column info for the symbol before the one being putback
bool LexContext::putBack(char &ic, int &iline, int &icolumn)
{
  if (pbCount >= BUFFSIZE - 2 || PosNext == 1) { return false; }    
  ++pbCount;
  --PosNext;
  ic = symbol[bufIndex(PosNext-1)];
  iline = line[bufIndex(PosNext-1)];
  icolumn = column[bufIndex(PosNext-1)];
  return true;
}

int alglex(union algValue *pT, LexContext *LC)
{
  int tok = -1;
  int retval = 0; // 0 = OK, 1 = EOF, 2 = Unexpected Symbol
  int state = 0;
  int sline, scolumn;
  char c;
  int line, column;
  std::string value = "";

  while(1)
  {
    LC->getNext(c,line,column);
    switch(state)
    {
    case 0:
      value = c;
      sline = line; scolumn = column;
      if (isdigit(c)) state = 1;
      else if (isalpha(c)) state = 2;
      else if (c == '/') state = 3;
      else if (c == '\\') state = 4;
      else if (c == '<') state = 5;
      else if (c == '>') state = 6;
      else if (c == '=') state = 7;
      else if (isspace(c)) state = 0;
      else if (c == '~') { tok = NEG; goto Return;}
      else if (c == '+') { tok = PLUS; goto Return;}
      else if (c == '-') { tok = MINUS; goto Return;}
      else if (c == '*') { tok = MULT; goto Return;}
      else if (c == '^') { tok = EXP; goto Return;}
      else if (c == '(') { tok = LP; goto Return;}
      else if (c == ')') { tok = RP; goto Return;}
      else if (c == '[') { tok = LB; goto Return;}
      else if (c == ']') { tok = RB; goto Return;}
      else if (c == ',') { tok = COMMA; goto Return; }
      else if (c == '$') { tok = INTERP; goto Return; }
      else if (c == '%') { tok = PERCENT; goto Return; }
      else if (c == '#') { 
	do { LC->getNext(c,line,column); } while(c != '\n' && c != EOF); 
	LC->putBack(c,line,column); }
      else if (c == EOF) { retval = 1; goto Return; }
      else if (c == '_')
      {
	const char* target = "_root_";
	int i = 0;
	while(target[++i] != '\0' && (LC->getNext(c,line,column), target[i] == c))
	  value += c;
	if (target[i] == '\0') { tok = ROOT; goto Return; }
	else { tok = LEXERR; goto Return; }
      }
      else
      { value += c; tok = LEXERR; goto Return; }
      break;
    case 1:
      if (isdigit(c)) {	state = 1; value += c; }
      else { LC->putBack(c,line,column); tok = NUM; goto Return;}      
      break;
    case 2:
      if (isdigit(c) || isalpha(c) || c == '_') {  state = 2; value += c; }
      else { LC->putBack(c,line,column); tok = VAR; goto Return;}    
      break;
    case 3:
      if (c == '\\') { value += c; tok = AND; goto Return; }
      else if (c == '=') { value += c; tok = RELOP; goto Return; }
      else { LC->putBack(c,line,column); tok = DIV; goto Return; }
      break;
    case 4:
      if (c == '/') { value += c; tok = OR; goto Return; }
      else {  value += c; tok = LEXERR; goto Return; }
      break;
    case 5:
      if (c == '=') { value += c; tok = RELOP; goto Return; }
      else if (c == '>') { value = "/="; tok = RELOP; goto Return; }
      else { LC->putBack(c,line,column); tok = RELOP; goto Return; }
      break;
    case 6:
      if (c == '=') { value += c; tok = RELOP; goto Return; }
      else { LC->putBack(c,line,column); tok = RELOP; goto Return; }
      break;
    case 7:
      if (c == '=') { value += c; state = 8; }
      else { LC->putBack(c,line,column); tok = RELOP; goto Return; }
      break;
    case 8:
      if (c == '>') { value += c; tok = LIMPLR; goto Return; }
      else { value += c; tok = LEXERR; goto Return; }
      break;
    }
  }
  
 Return:
  if (tok == VAR)
  {
    if (value == "true") tok = TCONST;
    else if (value == "false") tok = FCONST;
    else if (value == "_root_") tok = ROOT;
    else if (value == "ex") tok = QF;
    else if (value == "all") tok = QF;
  }
  if (retval == 0)
  {
    (*pT).tokPtr = new TokenRef(new TokenObj(value,sline,scolumn,line,column));
    return tok;
  }
  else if (retval == 1)
    return 0; // End of file
  throw TarskiException("Unexpected condition in function alglex!");
}

}//end namespace tarski
