#include "sacMod.h"
#include "caserver/SingSacPolicy.h"
#include "caserver/CocoaSacPolicy.h"
#include "caserver/MathematicaSacPolicy.h"
#include "caserver/MapleSacPolicy.h"

using namespace std;

// extern void InputContextInit();
// extern void OutputContextInit();
// extern void PushInputContext(istream &in);
// extern void PopInputContext();

// CHRIS TEMP
#include "../../../qesource/source/qepcad.h"

//ServerBase GVSB;
//CAPolicy *GVCAP = 0;
void serverPrep(string policy, string server, string dirPath) 
{ 
  if (GVCAP == 0) 
  {
    if (server == "")
      ; // in this case, the policy must be "Saclib"!
    else if (server == "Maple")       
      GVSB.insert(pair<string,CAServer*>(server,new MapleServer(dirPath)));
    else if (server == "Mathematica") 
      GVSB.insert(pair<string,CAServer*>(server,new MathematicaServer(dirPath)));
    else if (server == "Singular")    
      GVSB.insert(pair<string,CAServer*>(server,new SingularServer(dirPath)));
    else if (server == "Cocoa")       
      GVSB.insert(pair<string,CAServer*>(server,new CocoaServer(dirPath)));
    else 
    {
      cerr << "Unknown CASSever '" << server << "'!" <<endl; 
      exit(0); 
    }

    if (policy == "Saclib")   
      GVCAP = new OriginalPolicy;
    else if (policy == "Singular") 
      GVCAP = new SingularPolicy;
    else if (policy == "SingSac")  
      GVCAP = new SingSacPolicy;
    else if (policy == "CocoaSac") 
      GVCAP = new CocoaSacPolicy;
    else if (policy == "MapleSac") 
      GVCAP = new MapleSacPolicy;
    else if (policy == "Maple")
      GVCAP = new MaplePolicy;
    else
    {
      cerr << "Unknown CAPolicy '" << policy << "'!" << endl; 
      exit(0); 
    }
  }
}

static void init_SIGINT_handler();

void SacModInit(int argc, char **argv, int &ac, char** &av,
		string policy, string server, string dirPath,
		void* topOfTheStack)
{
  ARGSACLIB(argc,argv,&ac,&av);
  //  BEGINSACLIB((Word *)&argc);
  BEGINSACLIB((Word *)topOfTheStack);
  InputContextInit();
  OutputContextInit();
  serverPrep(policy,server,dirPath);
  init_SIGINT_handler();
}


void SacModEnd()
{
  //cerr << "In SacModEnd!" << endl;
  /* Delete the CAServers ... mostly to let them kill child processes! */
  for(ServerBase::iterator p = GVSB.begin(); p != GVSB.end(); ++p)    
    delete p->second;
  GVSB.clear();
  if (GVCAP != 0) { delete GVCAP; GVCAP = 0; }

  ENDSACLIB(SAC_FREEMEM);
}

int CATime()
{
  int t = CLOCK();
  for(map<string,CAServer*>::iterator itr = GVSB.begin(); itr != GVSB.end(); ++itr)
    t += itr->second->serverTime();
  return t;
}

void CAStats()
{
  STATSACLIB();
  for(map<string,CAServer*>::iterator itr = GVSB.begin(); itr != GVSB.end(); ++itr)
    itr->second->reportStats(cout);
}

static void SIGINT_handler(int i, siginfo_t *sip,void* uap)
{
  SacModEnd();
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
  free(p);
}

