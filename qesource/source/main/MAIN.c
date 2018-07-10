#include "qepcad.h"
#include <iostream>
#include "db/convenientstreams.h"
#include <signal.h>
#include "db/CAPolicy.h"

static void SIGINT_handler(int i, siginfo_t *sip,void* uap);
static void init_SIGINT_handler();
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
       init_SIGINT_handler(); /* A special handler for SIGINT is needed
                                 to shut down child processes. Also used
			         for SIGTERM. */
       if (GVTIMEOUTLIMIT > 0)
	 sendSignalAfterInterval(GVTIMEOUTLIMIT,SIGALRM);

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
	 INPUTRD(&Fs,&V);
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

static int sendSignalAfterInterval(int seconds, int signum)
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

int main(int argc, char **argv)
{
  int dummy;
  void *topOfTheStack = &dummy;
  mainDUMMY(argc,argv,topOfTheStack);
}
