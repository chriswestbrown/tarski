#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;
#include "../tarski.h"
#include "smtlib.h"

int lextest()
{
  // Test by inputing SMTLIB code on standard in.
  SMTLib::Lexer L(std::cin);
  SMTLib::Token t;
  do {
    t = L.next();
    std::cout << t.getType() << ' ' << t.getValue() << std::endl;    
  }while(t.getType() != SMTLib::END);
  return 0;
}

int parsetest()
{
  // Test by inputing SMTLIB code on standard in.
  SMTLib::Parser P(std::cin);
  while(true)
  {
    SMTLib::Exp *e = P.next();
    SMTLib::Token t = e->getLeadTok();
    if (t.getType() == SMTLib::END) break;
    if (t.getType() == SMTLib::ERR) { std::cerr << t.getValue() << std::endl; break; }
    e->write(std::cout);
    std::cout << std::endl;
  }  
  return 0;
}

std::string getTarskiFormat(SMTLib::Exp *formula)
{
  using namespace std;
  ofstream fout;
  char* temp = new char[8];
  strcpy(temp,"fXXXXXX");
  string tempout = (temp + string("out"));
  temp = mktemp(temp);
  fout.open(temp);
  fout << "(load \"scheme/rewriteForTarski.scm\")" << endl;
  fout << "(define F '";
  formula->write(fout);
  fout << ")" << endl;
  fout << "(rd-disp (rd F))" << endl;
  fout.close();
  string restr = string("mzscheme -f ") + temp + " > " + tempout;
  system(restr.c_str());

  // read in the output file
  ostringstream sout;
  ifstream fin(tempout.c_str());
  for(char c = fin.get(); c != EOF; c = fin.get())
    sout << c;
  fin.close();

  { string rmstr = string("rm -f ") + temp; system(rmstr.c_str()); }
  { string rmstr = string("rm -f ") + tempout; system(rmstr.c_str()); }

  return sout.str();
}

void readKeymaeraWriteForTarski(std::istream& in)
{
  using namespace std;
  SMTLib::Parser P(in);
  bool previousAsserts = false, done = false;
  while(!done)
  {
    SMTLib::Exp *e = P.next();
    switch(e->getLeadTok().getType())
    {
    case SMTLib::END: 
      done = true;
      break;

    case SMTLib::ERR: 
      cerr << e->getLeadTok().getValue() << ' ';
      cerr << "near line " << e->getLeadTok().getLineNum() << ", column " 
	   << e->getLeadTok().getColPos() << endl;
      done = true;
      break;

    case SMTLib::LP:
      if (e->size() >= 1 && e->get(0)->matches(SMTLib::Token(SMTLib::SYM,"exit")))
      {
	done = true;
	break;
      }
      else if (e->size() > 1 && e->get(0)->matches(SMTLib::Token(SMTLib::SYM,"assert")))
      {
	SMTLib::Exp* formula = e->get(1);
	cerr << endl << "READ: ";
	formula->write(cerr);
	cerr << endl << endl;;
	string formattedFormula = getTarskiFormat(formula);
	if (!previousAsserts) 
	{ 
	  cout << "(def F [ " << formattedFormula << " ])" << endl; previousAsserts = true; 
	}
	else 
	{ 
	  cout << "(def F (t-and F [ " << formattedFormula << " ]))" << endl; 
	}
      }
      else if (e->size() == 1 && e->get(0)->matches(SMTLib::Token(SMTLib::SYM,"check-sat")))
      {
	cout << "(def F (exclose F))" << endl;
	
      }
      break;
    default:
      cerr << "Expression '";
      e->write(cerr);
      cerr << "' not understood!" << endl;
      break;
    }
  }  
  P.cleanup();
}

int main()
{  
  using namespace std;
  readKeymaeraWriteForTarski(cin);
  return 0;
}
