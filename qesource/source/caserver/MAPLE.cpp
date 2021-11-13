#ifndef __MINGW32__
#include "MAPLE.h"
#include "io/iocontexts.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;


MapleServer::MapleServer(string dirPath)
{
  string MapleCall = dirPath;

  // Fork
  childpid = fork();
  if (childpid == -1) { perror("Failed to fork!"); exit(1); }

  // Child process's code
  if (childpid == 0) {
    intoMaple.setStdinToPipe();
    outofMaple.setStdoutToPipe();

    // Begin: Just for debug!!
    //    system("/usr/local/bin/maple -q | tee /tmp/MapleOutLog");
    //    exit(0);
    // End:   Just for debug!!


    execlp(MapleCall.c_str(),
	   MapleCall.c_str(),
	   "-q",
	   NULL);
      perror("MapleServer Constructor: Maple startup failed! (Set MAPLE environment variable)");
      outofMaple.closeOut();
      exit(0);
  }
  else {
    // Even given an integral polynomial, maple sometimes returns a factor list in which
    // factors have rational coefficients (though, of course, multiplying out my the costant
    // term removes them).  This little function, massages the factors list using the icontent
    // function in order to ensure that all factors are primitive, integral polynomials.
    intoMaple.out() << 
"\
MFF := proc(F)\
local c, L;\
c := F[1];\
L := map(proc(x) local cp; cp := icontent(x[1]); c := c*cp; [ x[1]/cp , x[2] ] end proc, F[2]);\
[ c , L ];\
end proc:\
" 
    << endl;
  }

}

MapleServer::~MapleServer()
{
  //  cerr << "MapleServer Destructor!" << endl;
  intoMaple.out() << endl << "quit;" << endl;
}

int MapleServer::serverTime()
{
  intoMaple.out() << "time();" << endl;
  double t;
  outofMaple.in() >> t;
  return int(1000*t + 0.5);
}

void MapleServer::reportStats(ostream &out)
{
  out << "Time in Maple: " 
      << serverTime()
      << " ms." << endl;    
}


static char peekNonWS(istream &in) 
{ 
  char c; while(((c = in.peek()) && c == ' ')|| c == '\t' || c == '\n') in.get(); return c; 
}

static bool readBracketedInputFromStream(istream &in, ostream &out)
{
  char c;
  if ((c = peekNonWS(in)) && c != '[') return false;
  c = in.get(); 
  out << c;
  
  while((c = in.peek()))
  {
    if (c == '[') readBracketedInputFromStream(in,out);
    else if (c == '\\')
    {
      in.get();
      c = in.peek();
      if (c != '\n') { cerr << "Unexpected output from Maple: \\ not followed by \\n!\n"; exit(1); }
      in.get();
    }
    else 
    {
      in.get();
      out << c;
      if (c == ']') break; 
    }
  }
  return true;
}


inline bool polyCharX(char c)
{

  bool x = isalnum(c) || c== ' ' || c == '\n' || c == '+' || c == '-' || c == '^' || c == '*' || c == '(' || c == ')';
  return x;
}

static Word readMaplePoly(Word r, Word V, istream &in)
{
  Word A, t;
  string s;
  char c;
  while((c = in.peek()) && (c == '\\' || polyCharX(c)))
  {
    c = in.get();
    if (c == '\\') { while(in.get() != '\n'); }
    else if (c == '*') s += ' ';
    else s += c;
  }
  s += ".\n";
  istringstream si(s);
  PushInputContext(si);
  IPEXPREADR(r,V,&A,&t);
  PopInputContext();

  if (t == 0) 
  { 
    cerr << "QEPCADB could not read Maple's output!" << endl
         << s << endl;
    exit(1);
  }
  return A;
}

static Word CreateVariableList(Word r)
{
  Word V = NIL;
  char s[] = {'a','\0'};
  for(int i = 'a' + r - 1; i >= 'a'; --i)
  {
    s[0] = i;
    V = COMP(LFS(s),V);
  }
  return V;
}

static string WritePolyForMaple(Word r, Word P, Word V)
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
  // Put in * symbols
  for(unsigned int i = 1; i < out.length() - 1; ++i)
    if (out[i] == ' ' && out[i+1] != '+' && out[i+1] != '-'
	&& out[i-1] != '+' && out[i-1] != '-'
	)
      out[i] = '*';

  return out;
}

/*
static void DefineRing(ostream &out, const string &name, Word r, const string &order="lp")
{
  out << "ring " << name << " = 0, (";
  for(int i = r-1; i > 0; --i)
    out << char('a' + (r-1) - i) << ',';
  out << char('a' + r-1) << "), " << order << ";" << endl;
  out << "short = 0;" << endl;
}
*/

void MapleServer::IPFAC(Word r, Word P, Word *s_, Word *c_, Word *L_)
{
  int T1 = serverTime();

  // Send Maple commands
  Word V = CreateVariableList(r);
  string out =  WritePolyForMaple(r,P,V);

  //  cout << out << endl << endl;

  intoMaple.out() << "lprint(MFF(factors(" << out << ")));" << endl;
  //{ ofstream debout("/tmp/lastpol.txt",ios_base::out | ios_base::app); 
  //  debout << "lprint(MFF(factors(" << out << ")));" << endl; }
  /********************************************************
   * Read & parse Maple output 
   ********************************************************/
  stringstream ss;
  readBracketedInputFromStream(outofMaple.in(),ss);
  istream &in = ss;
  //  istream &in = outofMaple.in();
  
  // Read initial "[", content, and ","
  Word ct = 0;
  {
    char c1; string s1;
    in >> c1;
    while((c1 = in.get()) && c1 != ',') s1 += c1;
    s1 += ';';
    istringstream si(s1);
    PushInputContext(si);
    ct = IREAD();
    PopInputContext();
  }

  // Read & parse list of factors
  Word L = NIL;
  Word sn = 1;
  char c;
  in >> c; // Reads the initial [
  in >> c;
  while(c != ']')
  {
    if (c == ',') in >> c;

    // Read poly p and multiplicity m
    Word p = readMaplePoly(r,V,in);
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
  in >> c; // Read ] that ends the entire "factors" output.

  // Adjust content & sign if ct is negative
  if (ISIGNF(ct) < 0) { ct = INEG(ct); sn *= -1; }
 
  // Figure out how long that took!
  int T2 = serverTime();

  // RETURN
  *s_ = sn;
  *c_ = ct;
  *L_ = L;
  return;
}


/*
  Produces the fully factored discriminant
 */
void MapleServer::IPFACDSCR(Word r, Word P, Word *s_, Word *c_, Word *L_)
{
  int T1 = serverTime();

  // Send Maple commands
  Word V = CreateVariableList(r);
  string out =  WritePolyForMaple(r,P,V);  
  //  cout << out << endl << endl;

  intoMaple.out() << "lprint(MFF(factors(discrim(" << out << "," << (char)('a' + r-1) << "))));" << endl;
  //{ ofstream debout("/tmp/lastpol.txt",ios_base::out | ios_base::app); 
  //  debout << "lprint(MFF(factors(" << out << ")));" << endl; }
  // cerr << "lprint(MFF(factors(discrim(" << out << "," <<  (char)('a' + r-1) << "))));" << endl;
  /********************************************************
   * Read & parse Maple output 
   ********************************************************/
  stringstream ss;
  readBracketedInputFromStream(outofMaple.in(),ss);
  istream &in = ss;
  //  istream &in = outofMaple.in();
  --r; // discrim eliminates a var!
  // Read initial "[", content, and ","
  Word ct = 0;
  {
    char c1; string s1;
    in >> c1;
    while((c1 = in.get()) && c1 > 0 && c1 != ',') s1 += c1;
    if (c1 <= 0) { cerr << "Error in IPFACDSCR: Maple has some kind of error!" << endl; exit(1); }
    s1 += ';';
    istringstream si(s1);
    PushInputContext(si);
    ct = IREAD();
    PopInputContext();
  }

  // Read & parse list of factors
  Word L = NIL;
  Word sn = 1;
  char c;
  in >> c; // Reads the initial [
  in >> c;
  while(c != ']')
  {
    if (c == ',') in >> c;

    // Read poly p and multiplicity m
    Word p = readMaplePoly(r,V,in);
    //PushOutputContext(cerr); SWRITE("MAPLE: ");  OWRITE(p); SWRITE("\n"); PopOutputContext();
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
  in >> c; // Read ] that ends the entire "factors" output.

  // Adjust content & sign if ct is negative
  if (ISIGNF(ct) < 0) { ct = INEG(ct); sn *= -1; }
 
  // Figure out how long that took!
  int T2 = serverTime();

  // RETURN
  *s_ = sn;
  *c_ = ct;
  *L_ = L;
  return;
}


/*
  Produces the fully factored leading coefficient
 */
void MapleServer::IPFACLDCF(Word r, Word P, Word *s_, Word *c_, Word *L_)
{
  int T1 = serverTime();

  // Send Maple commands
  Word V = CreateVariableList(r);
  string out =  WritePolyForMaple(r,P,V);  
  //  cout << out << endl << endl;

  intoMaple.out() << "lprint(MFF(factors(lcoeff(" << out << "," << (char)('a' + r-1) << "))));" << endl;
  //{ ofstream debout("/tmp/lastpol.txt",ios_base::out | ios_base::app); 
  //  debout << "lprint(MFF(factors(" << out << ")));" << endl; }
  //  cerr << "lprint(MFF(factors(lcoeff(" << out << "," <<  (char)('a' + r-1) << "))));" << endl;
  /********************************************************
   * Read & parse Maple output 
   ********************************************************/
  stringstream ss;
  readBracketedInputFromStream(outofMaple.in(),ss);
  istream &in = ss;
  //  istream &in = outofMaple.in();
  --r; // lcoeff eliminates a var!
  // Read initial "[", content, and ","
  Word ct = 0;
  {
    char c1; string s1;
    in >> c1;
    while((c1 = in.get()) && c1 != ',') s1 += c1;
    s1 += ';';
    istringstream si(s1);
    PushInputContext(si);
    ct = IREAD();
    PopInputContext();
  }

  // Read & parse list of factors
  Word L = NIL;
  Word sn = 1;
  char c;
  in >> c; // Reads the initial [
  in >> c;
  while(c != ']')
  {
    if (c == ',') in >> c;

    // Read poly p and multiplicity m
    Word p = readMaplePoly(r,V,in);
    //PushOutputContext(cerr); SWRITE("MAPLE: ");  OWRITE(p); SWRITE("\n"); PopOutputContext();
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
  in >> c; // Read ] that ends the entire "factors" output.

  // Adjust content & sign if ct is negative
  if (ISIGNF(ct) < 0) { ct = INEG(ct); sn *= -1; }
 
  // Figure out how long that took!
  int T2 = serverTime();

  // RETURN
  *s_ = sn;
  *c_ = ct;
  *L_ = L;
  return;
}

/*
  Produces the fully factored resultant
 */
void MapleServer::IPFACRES(Word r, Word A, Word B, Word *s_, Word *c_, Word *L_)
{
  int T1 = serverTime();

  // Send Maple commands
  Word V = CreateVariableList(r);
  string outA =  WritePolyForMaple(r,A,V);  
  string outB =  WritePolyForMaple(r,B,V);  
  //  cout << out << endl << endl;

  intoMaple.out() << "lprint(MFF(factors(resultant(" 
		  << outA <<"," << outB << "," << (char)('a' + r-1) << "))));" << endl;
  //  cerr << "lprint(MFF(factors(resultant("  << outA <<"," << outB << "," << (char)('a' + r-1) << "))));" << endl;
  /********************************************************
   * Read & parse Maple output 
   ********************************************************/
  stringstream ss;
  readBracketedInputFromStream(outofMaple.in(),ss);
  istream &in = ss;
  --r; // res eliminates a var!
  // Read initial "[", content, and ","
  Word ct = 0;
  {
    char c1; string s1;
    in >> c1;
    while((c1 = in.get()) && c1 > 0 && c1 != ',') s1 += c1;
    if (c1 <= 0) 
    { 
      cerr << "Error in IPFACRES: Maple has some kind of error!" << endl; 
      cerr << "Occurred computing: resultant(" << outA <<"," << outB << "," 
	   << (char)('a' + r) << "))))" << endl;
      exit(1); 
    }
    s1 += ';';
    istringstream si(s1);
    PushInputContext(si);
    ct = IREAD();
    PopInputContext();
  }

  // Read & parse list of factors
  Word L = NIL;
  Word sn = 1;
  char c;
  in >> c; // Reads the initial [
  in >> c;
  while(c != ']')
  {
    if (c == ',') in >> c;

    // Read poly p and multiplicity m
    Word p = readMaplePoly(r,V,in);
    //PushOutputContext(cerr); SWRITE("MAPLE: ");  OWRITE(p); SWRITE("\n"); PopOutputContext();
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
  in >> c; // Read ] that ends the entire "factors" output.

  // Adjust content & sign if ct is negative
  if (ISIGNF(ct) < 0) { ct = INEG(ct); sn *= -1; }
 
  // Figure out how long that took!
  int T2 = serverTime();

  // RETURN
  *s_ = sn;
  *c_ = ct;
  *L_ = L;
  return;
}
#endif
