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
    bool trackNumberOfLeafCells;
    int numberOfLeafCells;
    char solFormType;
    VarOrderRef ord;
  public:
    QepcadConnection() {
      trackUnsatCore = false;
      trackNumberOfLeafCells = false;
      solFormType = 'T';
      numberOfLeafCells = -1;
    }
    void setSolFormType(char val) { solFormType = val; } // set to 0 for no sol form
    void setTrackNumberOfLeafCells(bool val) { trackNumberOfLeafCells = val; }
    void setVarOrder(VarOrderRef ord) { this->ord = ord; }
    int getNumberOfLeafCells() {
      if (numberOfLeafCells < 0)
	throw TarskiException("Number of leaf cells not recorded!");
      return numberOfLeafCells;
    }
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
      string qscript = naiveButCompleteWriteForQepcad(F,introducedAssumptions,false,
						      this->trackUnsatCore,
						      this->solFormType,
						      this->ord
						      );

      if (qscript == "true" || qscript == "false")
      {
	// TODO: need to set things up so call to get witness or unsat core doesn't barf!
	return new TConstObj(qscript == "true");
      }

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
      //cerr << "qscript START>>\n" << qscript << "<<END qscript\n";
      intoQepcad.out() << qscript << flush;
      if (trackNumberOfLeafCells)
	intoQepcad.out() << "d-number-leaf-cells\n" << flush;
      // intoQepcad.out() << "quit\n" << flush; [NOTE! I commented this out 5/13/2020]
	
      bool success = true;
      string emsg = "";
      TFormRef result;
    
      // Read formula
      auto pos = string::npos;
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
	if ((pos = ln.find("Number of leaf cells = ")) != string::npos)
	{
	  istringstream sin(ln);
	  char tmp;
	  while(sin >> tmp && tmp != '=');
	  sin >> numberOfLeafCells;
	}
	if (ln == "An equivalent quantifier-free formula:")
	  count = 0;
	else if (count >= 0 && ++count == 2)
	  break;
      }    

      try {
        if (warningCount > 0) throw TarskiException("Qepcad failure: input not well-oriented!");
        else if (timeoutCount > 0) throw TarskiException("Qepcad timeout!");
	else if (solFormType == 0) {
	  result = NULL;
	}
	else {
	  if (count == -1) throw TarskiException("Qepcad failure!");
	  else if (ln == "TRUE") {
	    intoQepcad.out() << "d-witness-list\n" << flush;
	    int found = 0;	
	    while(getline(outofQepcad.in(),ln))
	    {
	      //cerr << "err2: " << ln << " " << (ln.find("-   Sample point  -") != string::npos) << endl;
	      if (found == 0 && (ln.find("Command only valid for SAT problems!") != string::npos) ||
		  (ln.find("Before Solution >") != string::npos)
		  )
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
	      result = introducedAssumptions.is_null() ? MF.res : (TFormRef)(introducedAssumptions && MF.res);
	    }
	    catch(TarskiException &e) { 
	      throw TarskiException(string("Could not interpret Qepcad output as Tarski Formula! ") + e.what());
	    }
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
