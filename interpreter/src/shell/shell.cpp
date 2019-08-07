#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <iostream>
#include <stack>
#include "../algparse/algparse.h"
#include "formmanip.h"
#include "formula.h"
#include "normalize.h"
#include "linearSubs.h"
#include <algorithm>
#include "einterpreter.h"
#include "readlineistream.h"
#include "../onecell/memopolymanager.h"
#include "../tarskisysdep.h" /* defines pathToMaple variable */
#include <signal.h>

using namespace std;

uint64 GC_Obj::next_tag = 1;
uint64 GC_Obj::num_deleted = 0;


namespace tarski {

extern void RRIS_summary();
Tracker compTracker;


  
/**************************************************************
 * I'm keeping this bit in my hip pocket in case I need to add
 * helper files that the executable accesses for, for example,
 * documentation.
 **************************************************************/
// Returns full path to current executable (w/o executable name)
// but with trailing "/"
// as a string if successful, an empty string otherwise
string pathToCurrentExecutable()
{
  // Set up Buffer
  const int N = 1024;
  char buff[N] = {0,};

  // System independent part!
#ifdef ARCH_darwin_14_i86
  uint32_t size = N-1;
  if (_NSGetExecutablePath(buff, &size) != 0) {
    return "";
  }
  buff[size] = '\0';
#else
  int n = readlink("/proc/self/exe",buff,N);
  if (n == -1 || buff[N-1] != '\0') return "";
#endif

  // "erase" the executable name
  int i = 0, s = -1; 
  for(;buff[i] != '\0'; i++)
    if (buff[i] == '/') 
      s = i;
  buff[s+1] = '\0';
  return buff;
}

void help(ostream& out);
void printVersion(ostream& out);

void nln() { cout << endl; } // just to help with using gdb
void SIGINT_handler(int i, siginfo_t *sip,void* uap);
void init_SIGINT_handler();
int sendSignalAfterInterval(int seconds, int signum);

  
int mainDUMMY(int argc, char **argv, void* topOfTheStack)
{
    // Get the CA Server up and running!
    int ac = 0; char **av = NULL;
    string CASString = "Saclib";
    bool quiet = false;
    bool file = false;
    ifstream fin;
    vector<string> inputFileNames;
    int inputFilePos = 0;
    for(int i = 1; i < argc; ++i)
    {
      if (argv[i] == string("-CASSaclib")) CASString = "Saclib";
      else if (argv[i] == string("-CASMaple"))  CASString = "Maple";
      else if (argv[i] == string("-CASMapleSac")) CASString = "MapleSac";
      else if (argv[i] == string("-v")) { verbose = true; }
      else if (argv[i] == string("-q")) { quiet = true; }
      else if (argv[i][0] == '+') { ; }
      else if (argv[i] == string("-h")) { help(cout); exit(0); }
      else if (argv[i] == string("--version")) { printVersion(cout); exit(0); }
      else if (argv[i] == string("-t")) {
	int tout = -1;
	if (i + 1 < argc && (tout = atoi(argv[i+1])) && tout > 0) {
	  init_SIGINT_handler();
	  sendSignalAfterInterval(tout,SIGALRM);
	  i++;
	}
	else {
	  cerr << "tarski: Error! -t requires a poositive integer argument!\n";
	  exit(1);
	}
      }
      else
      {
	inputFileNames.push_back(argv[i]);
	quiet = file = true;
      }
    }
    if (CASString == "MapleSac") 
      SacModInit(argc,argv,ac,av,"MapleSac","Maple",pathToMaple,topOfTheStack);
    else if (CASString == "Saclib")
      SacModInit(argc,argv,ac,av,"Saclib","","",topOfTheStack);
    else if (CASString == "Maple")
    { 
      SacModInit(argc,argv,ac,av,"Maple","Maple",pathToMaple,topOfTheStack);
    }
    free(av);

    srand(time(0));
    
    //  istream *inptr = new readlineIstream();
    readlineIstream isin;
    if (!quiet) { isin.setPrompt("> "); }

    // Input?
    istream* piin =
      file ?
      (istream*)&fin :
      (quiet && !isatty(fileno(stdin)) ? (istream*)&cin : (istream*)&isin);
    istream &iin = *piin;
    LexContext LC(iin,';');
    defaultNormalizer = new Level3and4(7,7);

    MemoizedPolyManager PM;
    NewEInterpreter I(&PM);
    I.init();

    

    (*I.rootFrame)["%"] = new SObj(); // Seed the % variable with a void value;
    (*I.rootFrame)["%e"] = new SObj(); // Seed the %e variable with a void value;
    (*I.rootFrame)["%E"] = new SObj(); // Seed the %E variable with a void value;
    bool explicitQuit = false;
    while(iin || (file && inputFilePos < inputFileNames.size()))
    {
      if (!iin || (file && inputFilePos == -1)) 
      { 
	fin.clear(); 
	fin.close();
	fin.open(inputFileNames[inputFilePos].c_str());
	if (!fin) { cerr << "File \"" << inputFileNames[inputFilePos] << "\" not found!" << endl; exit(1); }
	++inputFilePos; 
      }
      SRef x = I.next(iin);
      if (x.is_null()) continue;
      SRef res = I.eval(I.rootFrame,x);
      if (res->type() == _err && res->err()->msg == "INTERPRETER:quit") { explicitQuit = true; break; }
      if (!quiet) { cout << res->toStr() << endl; }
      if (res->type() != _err && res->type() != _void) { I.rootFrame->set("%",res); }
      if (res->type() == _err) { I.rootFrame->set("%E",new StrObj(res->err()->getMsg())); }
      I.rootFrame->set("%e",res);
      I.markAndSweep();
    }      
    if (!quiet && !explicitQuit) { cout << endl; }

    if (verbose)
    {
      compTracker.report();
      compTracker.clear();
    }
    
    delete defaultNormalizer;
    SacModEnd();
    finalcleanup = true;
    return 0;
}

void printVersion(ostream& out)
{
  out << "tarski " << tarskiVersion << " " << tarskiVersionDate << endl;
}

void help(ostream& out)
{
  printVersion(out);	       
  out << "\
Usage: tarski [options] [file1 file2 ... filek]\n\
\n\
Tarski is a system for computing with Tarski formulas / semi-algebrac\n\
sets.  This is an interpreter shell allowing the user to define Tarski\n\
formulas and perform certain kinds of computations - most notably\n\
simplification, suggesting Qepcad formulations, Open NuCAD construction,\n\
and certain kinds of rewriting.\n\
\n\
Options:\n\
  -CASSaclib : use Saclib as the only computer algebra system.  Some\n\
       things are likely not to work in this case.\n\
  -CASMapleSac : use a combination of Saclib and Maple for computer\n\
       algebra computations.  This is the default behavior, and is\n\
       recommended ... though it requires having Maple, of course.\n\
  -h : print this helpful description\n\
  -q : quiet mode, no prompt, no echo results\n\
  -t <num> : terminates tarski after <num> seconds\n\
  -v : verbose, prints out debugging info\n\
  --version : prints out version number and exists\n\
  +N<numcells> : instructs Saclib to intialize with <numcells> cells\n\
       of memory in is garbage collected space.  Default is 2 million\n\
       i.e. +N2000000, which is pretty small.\n\
"
      << flush;
}

void SIGINT_handler(int i, siginfo_t *sip,void* uap)
{  
  if (sip->si_signo == SIGALRM)
    FAIL("TIMEOUT","Exiting Tarski due to timeout");
  SacModEnd();
  exit(1);
}

void init_SIGINT_handler() 
{
  struct sigaction *p;
  p = (struct sigaction *)malloc(sizeof(struct sigaction));
  p->sa_handler = NULL;
  p->sa_sigaction = SIGINT_handler;
  sigemptyset(&(p->sa_mask));
  p->sa_flags = SA_SIGINFO;
  sigaction(SIGINT,p,NULL);
  sigaction(SIGTERM,p,NULL);
  sigaction(SIGALRM,p,NULL);
  free(p);
}

int sendSignalAfterInterval(int seconds, int signum)
{
  /* Create timer */
  timer_t timerid;
  struct sigevent sev;
  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = signum;
  sev.sigev_value.sival_ptr = &timerid;
  if (timer_create(CLOCK_MONOTONIC, &sev, &timerid) == -1)
    return 1;

  /* Start timer */
  struct itimerspec its;
  its.it_value.tv_sec = seconds;
  its.it_value.tv_nsec = 0;
  its.it_interval.tv_sec = its.it_value.tv_sec;
  its.it_interval.tv_nsec = its.it_value.tv_nsec;
  if (timer_settime(timerid, 0, &its, NULL) == -1)
    return 2;

  return 0;
}
}//end namespace tarski


int main(int argc, char **argv)
{
  int dummy = 0;
  void *topOfTheStack = &dummy;
  if (strcmp(tarski::pathToQepcad, "") == 0) throw tarski::TarskiException("Invalid location for QEPCAD");
  tarski::mainDUMMY(argc,argv,topOfTheStack);
}

