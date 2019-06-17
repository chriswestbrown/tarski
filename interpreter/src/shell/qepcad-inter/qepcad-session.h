#ifndef _QEPCAD_SESSION_
#define _QEPCAD_SESSION_

#include <fstream>
#include <cstdlib>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include "../../poly/caserver/unnamedpipe.h"
#include "../../tarskisysdep.h"
#include "../../formula/writeForQE.h"

namespace tarski {

  class QepcadConnection
  {
    std::ostringstream samplePointOut;
    std::vector<VarSet> order;
    bool trackUnsatCore;
    std::vector<int> unsatCore;
  public:
    QepcadConnection() { trackUnsatCore = false; }
    const std::ostringstream& getSamplePointOut() { return samplePointOut; }
    vector<int> getUnsatCore() { return unsatCore; }
    std::string samplePointPrettyPrint(PolyManager* pPM)
    {
      ostringstream sout;
      istringstream sin(samplePointOut.str());
      std::string ln, next;
      getline(sin,ln);
      if (ln.find("PRIMITIVE") == string::npos) { return samplePointOut.str(); }
      if (sin >> next && getline(sin,ln) && ln.find("the unique root of x between 0 and 0") != string::npos)
        {
          while(getline(sin,ln))
            {
              istringstream lin(ln);
              if (lin >> next && next == "Coordinate")
                {
                  int i; lin >> i;
                  std::string sln; getline(lin,sln);
                  sout << pPM->getName(order[i-1]) << sln << endl;
                }
            }
          return sout.str();
        }
      else
        return samplePointOut.str();
    }

    //F: The formula to call
    //A: The assumptions to be made. The single arg version just sets A to TRUE (ie, F is true)
    //Can check result with F->constValue(), which is tarski::TRUE, tarski::FALSE, or tarski::UNDET
    //Sample Point methods work when F->constValue() == TRUE (samplePointPrettyPrint and getSamplePointOut)
    TFormRef basicQepcadCall(TFormRef F, bool trackUnsatCore = false) { return basicQepcadCall(F,TFormRef(new TConstObj(TRUE)), trackUnsatCore); }

    TFormRef basicQepcadCall(TFormRef F, TFormRef A, bool trackUnsatCore = false)
    {
      using namespace std;
      this->trackUnsatCore = trackUnsatCore;
      TAndRef tmp = new TAndObj();
      tmp->AND(F);
      tmp->AND(A);
      order = getBrownVariableOrder(tmp);
      TFormRef introducedAssumptions;
      string qscript = naiveButCompleteWriteForQepcad(F,introducedAssumptions,false,this->trackUnsatCore);
      //cerr << "qscript START>>\n" << qscript << endl << "<<END qscript\n";
      UnnamedPipe intoQepcad, outofQepcad;
      
      int childpid = fork();
      if (childpid == -1) { perror("Failed to fork!"); exit(1); }
      if (childpid == 0) { // Child process's code
        intoQepcad.setStdinToPipe();
        outofQepcad.setStdoutToPipe();
        outofQepcad.closeIn();
        outofQepcad.closeOut();
        intoQepcad.closeIn();
        intoQepcad.closeOut();

	// Set the qe environment variable for the qe process
	std::string tmps = pathToQepcad;
	int n = tmps.size();
	string qeroot = tmps.substr(0,n - 11);
	setenv("qe",qeroot.c_str(),1);
	
        const char *arg1 = "+N10000000", *arg2 = "-t", *arg3 = "200";
        execlp(pathToQepcad,
               pathToQepcad,
               arg1,arg2,arg3,NULL);
        throw TarskiException("QepcadB startup failed!");

      }

      // Send Qepcad the script & close unneeded pipes
      intoQepcad.closeIn();
      outofQepcad.closeOut();
      intoQepcad.out() << qscript << flush;
      //      intoQepcad.closeOut();

      bool success = true;
      string emsg = "";
      TFormRef result;
    
      // Read formula
      string ln;
      int count = -1, warningCount = 0, timeoutCount = 0;
      while(getline(outofQepcad.in(),ln))
        {
          //cerr << "err1: " << ln << endl;
          if (ln.find("WARNING!") != string::npos || ln.find("Warning!") != string::npos
              ||
              ln.find("ERROR!") != string::npos || ln.find("Error!") != string::npos
              )
            ++warningCount;
          if (ln.find("Failure occurred in") != string::npos &&
              ln.find("TIMEOUT") != string::npos)
            ++timeoutCount;
          if (ln == "An equivalent quantifier-free formula:")
            count = 0;
          else if (count >= 0 && ++count == 2)
            break;
        }    

      try {
        if (warningCount > 0) throw TarskiException("Qepcad failure: input not well-oriented!");
        if (timeoutCount > 0) throw TarskiException("Qepcad timeout!");
        if (count == -1) throw TarskiException("Qepcad failure!");
        if (ln == "TRUE") {
	  intoQepcad.out() << "d-witness-list\n" << flush;
          int found = 0;	
          while(getline(outofQepcad.in(),ln))
	  {
	    //cerr << "err2: " << ln << " " << (ln.find("-   Sample point  -") != string::npos) << endl;
	    if (found == 0 && (ln.find("Command only valid for SAT problems!") != string::npos))
	      break;
	    
	    if (found == 0 && (ln.find("-   Sample point  -") != string::npos))
	      found = 1;
	    else if (found == 1 && (ln.find("---------------------") != string::npos))
	    {
	      found = 2;
	      break;
	    }
	    else if (found == 1)
	    {
	      samplePointOut << ln << endl;	
	    }
	  }
          result = introducedAssumptions.is_null() ? new TConstObj(TRUE) : introducedAssumptions;
        }
        else if (ln == "FALSE")
	{
	  if (this->trackUnsatCore)
	  {
	    intoQepcad.out() << "d-unsat-core-by-index\n" << flush;
	    
	    char c = 0;
	    while(outofQepcad.in() && (outofQepcad.in() >> c) && c != '[');
	    int k;
	    do {
	      outofQepcad.in() >> k >> c;
	      if (!outofQepcad.in()) { throw TarskiException("Error reading unsat core!"); }
	      unsatCore.push_back(k);
	    } while(c != ']');
	  }
	  result = new TConstObj(false);
	}
	else {
          istringstream sin("[ " + ln + " ]");
          TarskiRef T;
          LexContext LC(sin);
          algparse(&LC,T);
          if (T.is_null() || asa<AlgebraicObj>(T)) 
            { 
              throw TarskiException("Could not parse formula returned by Qepcad!");
            }
	
          // Interpret as Tarski formul and return TFormRef
          try 
            { 
              MapToTForm MF(*(F->getPolyManagerPtr()));
              T->apply(MF);
              return introducedAssumptions.is_null() ? MF.res : (TFormRef)(introducedAssumptions && MF.res);
            }
          catch(TarskiException &e) { 
            throw TarskiException(string("Could not interpret Qepcad output as Tarski Formula! ") + e.what());
          }
        }
      }
      catch(TarskiException &e) {
        success = false;
        emsg = e.what();
      }
      // We got what we need!  Kill qepcad and clean-up
      outofQepcad.closeIn();
      kill(childpid,SIGKILL);
      int qepcadExitStatus;
      waitpid(childpid,&qepcadExitStatus,0);    
      if (!success) { throw TarskiException(emsg); }
      return result;
    }  
  };
}; // END namespace tarski

#endif
