#ifndef _ALGLEX_
#define _ALGLEX_
#include <string>
#include <iostream>
#include <ctype.h>
#include <stdio.h>
#include "cwbpt.h"

union algValue;

namespace tarski {
class LexContext;

// Note: line & column positions start at 1, not 0.
class LexContext
{
public:
  LexContext(std::istream &in, char ts = EOF) : 
    InPtr(&in), PosNext(1), pbCount(0) { line[PosNext] = column[PosNext] = 1; terminatingSymbol = ts; }

  // Returns next symbol from the stream, along with its line and column position
  void getNext(char &ic, int &iline, int &icolumn);

  // Returns symbol,line,column info for the symbol before the one being putback
  bool putBack(char &ic, int &iline, int &icolumn);

private:
  std::istream *InPtr;
  int PosNext;

  // These are circular buffers
  static const int BUFFSIZE = 3;
  int line[BUFFSIZE];
  int column[BUFFSIZE];
  char symbol[BUFFSIZE];
  int bufIndex(int k) { return (k + BUFFSIZE) % BUFFSIZE; }
  int pbCount;
  char terminatingSymbol;
  char tsFilter(char c) { return c == terminatingSymbol ? EOF : c; }
};

 // This is the prototype for the lex function bison's parser will call for the next token.
 int alglex(union ::algValue *pT, tarski::LexContext *LC);

}//end namespace tarski

#endif
