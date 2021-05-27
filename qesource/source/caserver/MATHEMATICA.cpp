#include "MATHEMATICA.h"
#include "io/iocontexts.h"
#include <iostream>
#include <string>
using namespace std;


MathematicaServer::MathematicaServer(string dirPath)
{
  // Should search PATH here!
  string MathematicaBase = dirPath;
  string MathematicaCall = MathematicaBase + "/math";
  
  // Fork
  childpid = fork();
  if (childpid == -1) { perror("Failed to fork!"); exit(1); }
  // Child process's code
  if (childpid == 0) {
    intoMathematica.setStdinToPipe();
    outofMathematica.setStdoutToPipe();

    // Begin: Just for debug!!
    // system("/usr/local/bin/math | tee /tmp/MathematicaOutLog");
    // exit(0);
    // End:   Just for debug!!


    execlp(MathematicaCall.c_str(),
	   MathematicaCall.c_str(),
	   NULL);
      perror("MathematicaServer Constructor: Mathematica startup failed! (Set MATHEMATICA environment variable)");
      outofMathematica.closeOut();
      exit(0);
  }
  else
  {
//     // Read Mathematica's header garbage.
//     char c1 = 0, c2 = 0;
//     while(!(c1 == '-' && c2 != '\n')) 
//     { 
//       c1 = c2; 
//       c2 = outofMathematica.in().get(); 
//     }
  }
}

MathematicaServer::~MathematicaServer()
{
  intoMathematica.out() << endl << "Quit" << endl;
}

int MathematicaServer::serverTime()
  {
//     intoMathematica.out() << "timer;" << endl;
//     int t;
//     outofMathematica.in() >> t;
//     return t;
    return 0; // I'll implement this later!
  }

void MathematicaServer::reportStats(ostream &out)
{
  out << "Time in Mathematica: " 
      << serverTime()
      << " ms." << endl;    
}

/*
static char peekNonWS(istream &in) 
{ 
  char c; while((c = in.peek()) && c == ' ' || c == '\t' || c == '\n') in.get(); return c; 
}
*/

inline bool polyChar(char c)
{ return isalnum(c) || c== ' ' || c == '+' || c == '-' || c == '^' || c == '/' || c == '*'
    || c == '(' || c == ')'; }

static Word readMathematicaPoly(Word r, Word V, istream &in)
{
  Word A, t;
  string s;
  for(char curr;polyChar(in.peek()) && (curr = in.get()); )
  {
    if (curr == '\\') {
      while(in.get() != '>')
        ;
      continue;
    }
    if (curr == '*' || curr == '\\' || curr == '>') curr = ' ';
    s += curr;    
  }
  s += ".\n";
  istringstream si(s);
  PushInputContext(si);
  IPEXPREADR(r,V,&A,&t);
  PopInputContext();

  if (t == 0) 
  { 
    cerr << "QEPCADB could not read Mathematica's output!" << endl
         << s << endl;
    exit(1);
  }
  return A;
}

static Word CreateVariableList(Word r)
{
  if (r > 26) { 
    cerr << "Mathematica communication system assumes 26 or fewer variables!"; 
    exit(0); }
  Word V = NIL;
  char s[] = {'a','\0'};
  for(int i = 'a' + r - 1; i >= 'a'; --i)
  {
    s[0] = i;
    V = COMP(LFS(s),V);
  }
  return V;
}

static string WritePolyForMathematica(Word r, Word P, Word V)
{
  //write poly to string
  string out;
  {
    ostringstream sout;
    PushOutputContext(sout);
    IPDWRITE(r,P,V);
    PopOutputContext();
    out = sout.str();
  }
  return out;
}



void MathematicaServer::IPFAC(Word r, Word P, Word *s_, Word *c_, Word *L_)
{
  int T1 = serverTime();

  Word V = CreateVariableList(r);
  string out =  WritePolyForMathematica(r,P,V);

  // Send Mathematica commands
  //cerr << out << endl;
  intoMathematica.out() << "Format[FactorList[" << out << "],InputForm]" << endl << endl;

  //Read until "Out[x]=" that preceds output
  for(char c1 = 0, c2 = 0;!(c1 == ']' && c2 == '=');) 
  { c1 = c2; c2 = outofMathematica.in().get(); }
  
  // Set up a string of the Mathematica line
  string inString;
  char c1 = 0, c2 = 0;
  while((c1 = outofMathematica.in().get()) && c1 != '\n')
  {
    if (c1 == '\\')
    { 
      while(outofMathematica.in().get() != '>');
      while (!polyChar(c1))
	c1 = outofMathematica.in().get();       
    }
    inString += c1;    
  }
  
  //  cerr << "inString = " << inString << endl;

  // Read & parse Mathematica output
  Word L = NIL;
  Word sn = 1, ct = 1;
  //  istream &in = outofMathematica.in();
  istringstream in(inString);
  char c;
  in >> c; // Reads the initial {
  in >> c;
  while(c != '}')
  {
    if (c == ',') in >> c;

    // Read poly p and multiplicity m
    Word p = readMathematicaPoly(r,V,in);
    int m;
    in >> c >> m >> c;
    
    // Make factor "positive" according to saclib's def
    if (ISIGNF(PLBCF(r,p)) == -1)
    {
      if (m % 2 == 1)
	sn *= -1;
      p = IPNEG(r,p);
    }

    L = COMP(LIST2(m,p),L);
    in >> c; // Read char after closing ]
  }

  // Reversing puts content in front
  L = CINV(L);

  // Pull out content, if there is any (actually, in mathematica there always is!)
  if (PCONST(r,SECOND(FIRST(L))))
  {
    ct = IEXP(PLBCF(r,SECOND(FIRST(L))),FIRST(FIRST(L)));
    L = RED(L);
  }


  // Figure out how long that took!
  int T2 = serverTime();

  // RETURN
  *s_ = sn;
  *c_ = ct;
  *L_ = L;
  return;
}

