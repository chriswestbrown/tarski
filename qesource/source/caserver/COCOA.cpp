#include "COCOA.h"
#include "io/iocontexts.h"
#include <iostream>
#include <string>
using namespace std;


CocoaServer::CocoaServer(string dirPath)
{
  string CocoaBase = dirPath;
  string CocoaCall = CocoaBase + "/cocoa";
  
  // Fork
  childpid = fork();
  if (childpid == -1) { perror("Failed to fork!"); exit(1); }
  // Child process's code
  if (childpid == 0) {
    intoCocoa.setStdinToPipe();
    outofCocoa.setStdoutToPipe();

    // Begin: Just for debug!!
    system("/home/wcbrown/bin/cocoa | tee /tmp/CocoaOutLog");
    exit(0);
    // End:   Just for debug!!


    execlp(CocoaCall.c_str(),
	   CocoaCall.c_str(),
	   NULL);
      perror("CocoaServer Constructor: Cocoa startup failed! (Set COCOA environment variable)");
      outofCocoa.closeOut();
      exit(0);
  }
  else
  {
    // Read Cocoa's header garbage.
    intoCocoa.out() << "Unset Echo;" << endl << "\"XX\";" << endl;
    string s; 
    while(s != "XX" && outofCocoa.in() >> s );
    outofCocoa.in() >> s; // Read the line of -'s following result.
  }
}

CocoaServer::~CocoaServer()
{
  intoCocoa.out() << endl << "Quit;" << endl;
}

int CocoaServer::serverTime()
  {
//     intoCocoa.out() << "timer;" << endl;
//     int t;
//     outofCocoa.in() >> t;
//     return t;
    return 0; // I'll implement this later!
  }

void CocoaServer::reportStats(ostream &out)
{
  out << "Time in Cocoa: " 
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
{ return isalnum(c) || c== ' ' || c == '+' || c == '-' || c == '^' || c == '/'; }
static Word readCocoaPoly(Word r, Word V, istream &in)
{
  Word A, t;
  string s;
  for(char curr,prev = ' ';polyChar(in.peek()) && (curr = in.get()); prev = curr)
  {
    if ('a' <= curr && curr <= 'z' && prev != ' ') s += ' ';
    s += curr;    
  }
  s += ".\n";
  istringstream si(s);
  PushInputContext(si);
  IPEXPREADR(r,V,&A,&t);
  PopInputContext();

  if (t == 0) 
  { 
    cerr << "QEPCADB could not read Cocoa's output!" << endl
         << s << endl;
    exit(1);
  }
  return A;
}

static Word CreateVariableList(Word r)
{
  if (r > 26) { 
    cerr << "Cocoa communication system assumes 26 or fewer variables!"; 
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

static string WritePolyForCocoa(Word r, Word P, Word V)
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

static void DefineRing(ostream &out, const string &name, Word r, const string &order="")
{
  out << "Use Q[";
  for(int i = r-1; i > 0; --i)
    out << char('a' + (r-1) - i) << ',';
  out << char('a' + r-1) << "];" << endl;
}

Word CocoaServer::IPRES(Word r, Word A, Word B)
{
  DefineRing(intoCocoa.out(),"myring",r);
  Word V = CreateVariableList(r);
  string s_A =  WritePolyForCocoa(r,A,V);
  intoCocoa.out() << "A := " << s_A << ";" << endl;
  string s_B =  WritePolyForCocoa(r,B,V);
  intoCocoa.out() << "B := " << s_B << ";" << endl;
  intoCocoa.out() << "Resultant(A,B," << char('a' + r - 1) << ");" << endl;
  
  // Read & parse Cocoa output
  Word p = readCocoaPoly(r-1,V,outofCocoa.in());

  return p;
}

Word CocoaServer::IPDSCR(Word r, Word A)
{
  DefineRing(intoCocoa.out(),"myring",r);
  Word V = CreateVariableList(r);
  string s_A =  WritePolyForCocoa(r,A,V);
  intoCocoa.out() << "A := " << s_A << ";" << endl;
  intoCocoa.out() << "Discriminant(A," << char('a' + r - 1) << ");" << endl;
  
  // Read & parse Cocoa output
  Word p = readCocoaPoly(r-1,V,outofCocoa.in());

  return p;
}

void CocoaServer::IPFAC(Word r, Word P, Word *s_, Word *c_, Word *L_)
{
  int T1 = serverTime();

  Word V = CreateVariableList(r);
  string out =  WritePolyForCocoa(r,P,V);

  // Send Cocoa commands
  DefineRing(intoCocoa.out(),"myring",r);
  intoCocoa.out() << "F := " << out << ";" << endl;
  intoCocoa.out() << "Factor(F);" << endl;
  
  // Read & parse Cocoa output
  Word L = NIL;
  Word sn = 1, ct = 1;
  istream &in = outofCocoa.in();
  char c;
  in >> c; // Reads the initial [
  in >> c;
  while(c != ']')
  {
    if (c == ',') in >> c;

    // Read poly p and multiplicity m
    Word p = readCocoaPoly(r,V,in);
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
  { string s; in >> s; } // Read the line of -'s terminating result 

  // Pull out content, if there is any
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

Word CocoaServer::IPFACTGB(Word r, Word I, Word N)
{
  return 0;
//   // WRITE COCOA INPUT
//   DefineRing(intoCocoa.out(),"myring",r);
//   Word V = CreateVariableList(r);
//   intoCocoa.out() << "ideal I = ";
//   for(Word L = I; L != NIL; L = RED(L))
//   {
//     intoCocoa.out() << WritePolyForCocoa(r,FIRST(L),V);
//     if (RED(L) != NIL)
//       intoCocoa.out() << ", ";
//     else
//       intoCocoa.out() << ";" << endl;
//   }
//   intoCocoa.out() << "option(\"redSB\");" << endl;
//   intoCocoa.out() << "facstd(I);" << endl;
//   intoCocoa.out() << "\"[end]:\";" << endl;

//   // READ COCOA OUTPUT
//   Word F = NIL;
//   string marker; 
//   while(outofCocoa.in() >> marker && marker != "[end]:")
//   {
//     Word GBi = NIL;
//     while(peekNonWS(outofCocoa.in()) == '_')
//     {
//       while(outofCocoa.in().get() != '=');
//       Word p = readCocoaPoly(r,V,outofCocoa.in());
//       GBi = COMP(p,GBi);      
//     }
//     F = COMP(CINV(GBi),F);
//   }
//   return CINV(F);
}
/*
static Word TDEG(Word r, Word A)
{
  if (r == 0) return 0;
  Word M = 0;
  for(Word L = A; L != NIL; L = RED2(L))
    M = IMAX(M,FIRST(L) + TDEG(r-1,SECOND(L)));
  return M;
}
*/
 Word CocoaServer::CONSTORDTEST(Word r, Word A, Word L)
 {
   return 0;
//   // COCOA INPUT: Define Ideal A + L & compute groebner basis G
//   DefineRing(intoCocoa.out(),"myring",r,"dp");
//   Word V = CreateVariableList(r);
//   intoCocoa.out() << "ideal I = ";
//   for(Word Lp = L; Lp != NIL; Lp = RED(Lp))
//     intoCocoa.out() << WritePolyForCocoa(r,FIRST(Lp),V) << ", ";
//   intoCocoa.out() 
//     << WritePolyForCocoa(r,A,V) << ";" << endl
//     << "ideal G = std(I);" << endl
//     << "G;" << endl
//     << "\"[end]:\";" << endl;

//   // COCOA OUTPUT: Read G
//   Word G = NIL;
//   while(peekNonWS(outofCocoa.in()) == 'G')
//   {
//     while(outofCocoa.in().get() != '=');
//     Word p = readCocoaPoly(r,V,outofCocoa.in());
//     G = COMP(p,G);      
//   }
//   { string em; while(outofCocoa.in() >> em && em != "[end]:"); }
//   G = CINV(G);

//   // COCOA INPUT: Find lowest order partials that are not all reduced to 0 by G
//   Word M = NIL, k;
//   for(k = 1; M == NIL; k++)
//   {
//     for(Word P = IPALLPARTIALS(r,A,k,1); P != NIL; P = RED(P))
//     {
//       intoCocoa.out() 
// 	<< "reduce(" << WritePolyForCocoa(r,FIRST(P),V) << ",G);" << endl;
//       Word p = readCocoaPoly(r,V,outofCocoa.in());
//       if (p != 0)
// 	M = COMP(p,M);
//     }    
//   }
  
//   // COCOA INPUT: Compute groebner basis for G + all partials
//   intoCocoa.out() << "ideal J = ";
//   for(Word Lp = G; Lp != NIL; Lp = RED(Lp))
//     intoCocoa.out() << WritePolyForCocoa(r,FIRST(Lp),V) << ", ";
//   for(Word Lp = M; Lp != NIL; Lp = RED(Lp))
//     intoCocoa.out() << WritePolyForCocoa(r,FIRST(Lp),V) << (RED(Lp) != NIL ? ", " : ";\n");
//   intoCocoa.out() << "std(J);" << endl;
//   intoCocoa.out() << "\"[end]:\";" << endl;

//   // COCOA OUTPUT:
//   Word GBi = NIL;
//   while(peekNonWS(outofCocoa.in()) == '_')
//   {
//     while(outofCocoa.in().get() != '=');
//     Word p = readCocoaPoly(r,V,outofCocoa.in());
//     GBi = COMP(p,GBi);      
//   }
//   { string em; while(outofCocoa.in() >> em && em != "[end]:"); }
//   GBi = CINV(GBi);
  
//   if (LENGTH(GBi) == 1) 
//   { 
//     Word rs,ps; PSIMREP(r,FIRST(GBi),&rs,&ps); 
//     if (rs == 0) return TRUE;
//   }
//   return GBi;
    
 }
