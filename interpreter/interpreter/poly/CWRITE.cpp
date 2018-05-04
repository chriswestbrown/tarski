/*======================================================================
                      CWRITE(C)

Character write.  C is written to the output stream.
This version replaces the saclib CWRITE.
======================================================================*/
extern "C" {
#include "saclib.h"
};
#include <stdio.h>
#include <stack>
#include <iostream>
#include <fstream>
using namespace std;

static Word OPOS = 0;

class OutputContext
{
public:
  ostream *pout;
  Word OPOS;
};

static stack<OutputContext> OCS;
ostream *currOut;

void PushOutputContext(ostream &out) 
{
  OCS.push(OutputContext());
  OCS.top().pout = currOut;
  OCS.top().OPOS = OPOS;
  currOut = &out;
  OPOS = 0;
}

void PopOutputContext()
{
  currOut = OCS.top().pout;
  OPOS = OCS.top().OPOS;
  OCS.pop();
}

ostream& TopOutputContext()
{
  return *currOut;
}

void OutputContextInit() 
{
  OPOS = 0;
  currOut = &cout;
}


void CWRITE(Word C)
{
       /* hide algorithm */

  //       if (OUTSTREAM == 0) { OUTSTREAM = fopen("/dev/tty","w"); }

Step1: /* Write. */
      (*currOut) << char(C); //putc(C,OUTSTREAM);

       if (C == '\n') 
         OPOS = 0;
       else
         OPOS++;

Return: /* Prepare for return. */
       return;
}
