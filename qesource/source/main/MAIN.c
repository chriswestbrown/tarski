#include "qepcad.h"
#include <iostream>
#include "caserver/convenientstreams.h"
#include <signal.h>
#include "caserver/CAPolicy.h"

#ifndef __MSYS__
#ifndef __MINGW32__
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#endif
#endif

#ifndef __MINGW32__
static void SIGINT_handler(int i, siginfo_t *sip,void* uap);
static void init_SIGINT_handler();
#endif
static int sendSignalAfterInterval(int seconds, int signum);
extern int GVTIMEOUTLIMIT;

/*====================================================================
                 main(argc,argv)

Main Algorithm which drives the QEPCAD sytem. 
====================================================================*/
int mainDUMMY(int argc, char **argv, void *topOfTheStack)
{
       Word Fs,F_e,F_n,F_s,V,t,ac;
       char **av;

Step1: /* Set up the system. */
       ARGSACLIB(argc,argv,&ac,&av);
       BEGINSACLIB((Word *)topOfTheStack);
       BEGINQEPCAD(ac,av);
#ifndef __MINGW32__
       init_SIGINT_handler(); /* A special handler for SIGINT is needed
                                 to shut down child processes. Also used
			         for SIGTERM. */
       if (GVTIMEOUTLIMIT > 0)
	 sendSignalAfterInterval(GVTIMEOUTLIMIT,SIGALRM);
#endif

Step2: /* Read input, create CAD, write result */
       PCCONTINUE = FALSE;
       PRINTBANNER();
       do {
	 // Reinitialize system in between runs
	 if (PCCONTINUE == TRUE) 
	 { 
	   INITSYS();
	   PCCONTINUE = FALSE;
	 }
	 INPUTRD(&Fs,&V,1); // errMode 1 is print and continue
	 QepcadCls Q(V,Fs);
	 BTMQEPCAD = ACLOCK();
	 Q.QEPCAD(Fs,&t,&F_e,&F_n,&F_s);
       } while (PCCONTINUE == TRUE);
       
Step3: /* Clean up the system. */
       SWRITE("\n=====================  The End  =======================\n");
       STATSACLIB();
       ENDQEPCAD();
       ENDSACLIB(SAC_FREEMEM);
       free(av); /* Frees the array malloc'd by ARGSACLIB */

Return: /* Prepare for return. */
       return 0;
}

#ifndef __MINGW32__
static void SIGINT_handler(int i, siginfo_t *sip,void* uap)
{  
  if (sip->si_signo == SIGALRM)
    FAIL("TIMEOUT","Exiting QEPCADB due to timeout");
  ENDQEPCAD(); // Kill child CAServer processes
  ENDSACLIB(SAC_FREEMEM);
  exit(1);
}

static void init_SIGINT_handler() 
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
#endif

static int sendSignalAfterInterval(int seconds, int signum)
{
#if defined(__APPLE__) || defined(__MINGW32__)
  return 1;
#else
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
#endif
}

int main(int argc, char **argv)
{
  int dummy;
  void *topOfTheStack = &dummy;
  mainDUMMY(argc,argv,topOfTheStack);
}

#ifndef __MSYS__
#ifndef __MINGW32__
// Taken from https://stackoverflow.com/a/77336/1044586
void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}
#endif
#endif

void mainLIB(int numcells, int timeout) {
#ifndef __MSYS__
#ifndef __MINGW32__
  signal(SIGSEGV, handler);
#endif
#endif

  int dummy;
  void *topOfTheStack = &dummy;
  Word ac;
  char **av;
  if (numcells == 0)
    numcells = 20000000;
  char argv1[20];
  sprintf(argv1, "+N%d", numcells);
  char* argv[1];
  argv[0] = argv1;
  int argc = 1;

  ARGSACLIB(argc,argv,&ac,&av);
  BEGINSACLIB((Word *)topOfTheStack);
  BEGINQEPCADLIB(timeout);
  HELPFRD();
  }

string PCLIB(string input) {
  Word Fs,F_e,F_n,F_s,V,t;

  stringstream outputbuffer;
  istringstream iss(input);
  istream& inputbuffer = iss;
  INITIO(&inputbuffer,&outputbuffer);
  INPUTRD(&Fs,&V);
  QepcadCls Q(V,Fs);
  BTMQEPCAD = ACLOCK();
  Q.QEPCAD(Fs,&t,&F_e,&F_n,&F_s);
  string output = outputbuffer.str();
  return output;
  }

// Chris' method to obtain data...
Word string2UnNormForm(const string &S, Word V)
{
  // Convert from string to QEPCAD Unnormalized Formula
  Word F, t;
  istringstream sin(S + ".\n");

  PushInputContext(sin);
  QFFRDR(V,&F,&t);
  CREAD();
  PopInputContext();

  if (t == 0)
  {
    cerr << "QEPCADB could not understand the formula:" << endl
      << sin.str() << endl;
    exit(1);
  }

  return F;
}

string unNormForm2string(Word F, Word V)
{
  // Convert from Unnormalized QEPCAD formula to string
  ostringstream OTS;
  PushOutputContext(OTS);
  QFFWR(V,F);
  PopOutputContext();
  return OTS.str();
}

string SLFQLIB(string str_formula, string str_assumptions)
{
  // Read input
  Word Fs, V;
  istringstream sin(str_formula);
  ostringstream sout;
  PushInputContext(sin);
  PushOutputContext(sout);
  INPUTRD(&Fs,&V);
  PopOutputContext();
  PopInputContext();

  // Initialize QEPCAD problem
  QepcadCls Q;
  Q.SETINPUTFORMULA(V,Fs);

  // Add assumptions if any
  Word As = NIL;
  if (str_assumptions != "")
    Q.SETASSUMPTIONS(As = string2UnNormForm(str_assumptions,V));

  // Create CAD & get simplified equivalent formula
  Q.CADautoConst("");
  Word Fd = Q.GETDEFININGFORMULA('E'); // NOTE: The 'T' option won't work until I fix
                                       // GETDEFININGFORMULA to return not only the formula
                                       // but also the projection factor set, since it may be
                                       // enlarged when the 'T' option is used.

  // Translate to string and output
  string OTS = unNormForm2string(Fd,V);
  string ret = OTS + (As == NIL ? "" : " under assumption " + unNormForm2string(As,V));
  return ret;
}

void ENDQEPCADLIB()
{
  // clean it all up
  ENDQEPCAD();
  ENDSACLIB(SAC_FREEMEM);
}
