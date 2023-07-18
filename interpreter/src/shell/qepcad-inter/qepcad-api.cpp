#include "qepcad-api.h"
#include "writeForQE.h"
#include "formula.h"
#include "../../../qesource/source/qepcad.h"

namespace tarski {


  string unNormForm2string(Word F, Word V)
  {
    // Convert from Unnormalized QEPCAD formula to string
    ostringstream OTS;
    ::PushOutputContext(OTS);
    try { ::QFFWR(V,F); } catch(exception &e) { ::PopOutputContext(); throw e; }
    ::PopOutputContext();
    return OTS.str();
  }

  SRef qepcadAPICall(std::string &input, char formType) {
    //cerr << std::endl << "qepcad-api-call input: " << input << std::endl << std::endl;
    string str_F = input;
    string res;
    string assumptionsAsUsed;
    bool errorFlag = false;
    try { 
      int ac=0;
      char *tmp[] = {NULL};
      char **av = tmp;
      ::BEGINQEPCADLIB(-1);

      // Read input, extracting the formula Fs and the variable list V
      Word Fs, V;
      istringstream sin(str_F);
      ostringstream sout;
      ::PushInputContext(sin);
      ::PushOutputContext(sout);
      try { ::INPUTRD(&Fs,&V); } catch(exception &e)
      { ::PopOutputContext(); ::PopInputContext(); throw TarskiException("qepcad-api fail (INPUTRD)"); }
      ::PopOutputContext();
      ::PopInputContext();

      // Initialize QEPCAD problem
      QepcadCls Q;
      Q.SETINPUTFORMULA(V,Fs);

      // Create CAD & get simplified equivalent formula
      ostringstream warningsAndErrors;
      std::stringstream rest;
      rest << sin.rdbuf();
      ::PushOutputContext(warningsAndErrors);
      try { Q.CADautoConst(rest.str()); } catch(exception &e)
      { ::PopOutputContext(); throw TarskiException("qepcad-api fail (CADautoConst)"); }
      ::PopOutputContext();

      // Output
      if (warningsAndErrors.str() != "") {
	errorFlag = true;
	res = warningsAndErrors.str();
      }
      else { 
	Word Fd = Q.GETDEFININGFORMULA(formType,1);
	res = unNormForm2string(Fd,V);
	if (res.length() > 0 && res[0] != '[') {
	  if (res == "0 = 0") { res = "[true]"; }
	  else if (res == "0 /= 0") { res = "[false]"; }
	  else { res = "[" + res + "]"; }
	}
	Word Ad = Q.GETASSUMPTIONS();
	if (Ad == NIL) { // No assumptions used
	  assumptionsAsUsed = "[true]";
	}
	else { 
	  assumptionsAsUsed = unNormForm2string(Ad,V);
	  if (assumptionsAsUsed.length() > 0 && assumptionsAsUsed[0] != '[')
	  { assumptionsAsUsed = "[" + assumptionsAsUsed + "]"; }
	}
      }
    }
    catch(QepcadException &e) { res = string("Error! ") + e.what(); errorFlag = true; }
    catch(exception &e) { res = string("Error! Qepcad-api-call failure!"); errorFlag = true; }

    // clean it all up
    ENDQEPCAD();
    if (errorFlag) { return new ErrObj(res); }
    // cerr << "res := " << res << endl;
    // cerr << "ass := " << assumptionsAsUsed << endl;
    return new LisObj(new StrObj(res), new StrObj(assumptionsAsUsed));
  }


  
  SRef CommQepcadAPICall::execute(SRef input, /*
						input: A string that is a valid qepcad input script
						formType: char T for Tarski, E for Extended G for Geometric
					      */
				  vector<SRef> &args) 
  {
    SRef res;
    try {
      TFormRef T = args[0]->tar()->val;

      // Do basic normalization to get rid of boolean constants, which qepcad
      // doesn't understand.
      RawNormalizer R(defaultNormalizer);
      R(T);
      T = R.getRes();

      // Bail out if this is already a constant
      { int tmp = T->constValue(); if (tmp != -1) { return new TarObj(new TConstObj(tmp)); } }
      char formType = 'E'; // Default!
      if (args.size() > 1) {
	SymRef s = args[1]->sym();
	if (!s.is_null()) {
	  switch(s->val[0]) {
	  case 'T': case 'E': formType = s->val[0]; break;
	  default: throw TarskiException("Invalid option \"" + s->val + "\".");
	  }
	}
      }
      TFormRef assumptions;
      std::string script = naiveButCompleteWriteForQepcad(T,assumptions);
      SRef qres = qepcadAPICall(script,formType);
      if (qres->type() == _err) { return qres; }
      LisRef qepcadOutput = qres;
      string tmp = "[ " + qepcadOutput->get(0)->str()->getVal() + " /\\ "
	+ qepcadOutput->get(1)->str()->getVal() + " ]";
      //      istringstream sin(qepcadOutput->str()->getVal());
      istringstream sin(tmp);
      {
	TarskiRef T;
	LexContext LC(sin);
	algparse(&LC,T);
	if (T.is_null() || asa<AlgebraicObj>(T)) 
	{ 
	  throw TarskiException("Could not parse formula returned by qepcad!");
	}
	
	// Interpret as formula and return TFormRef
	try 
	{ 
	  MapToTForm MF(*getPolyManagerPtr());
	  T->apply(MF);
	  res =  new TarObj(MF.res);
	}
	catch(TarskiException &e) { 
	  throw TarskiException(string("Could not interpret as Tarski Formula! ") + e.what());
	}
      }

      return res;
    }
    catch(TarskiException e)
    {
      return new ErrObj(e.what());
    }
    return new SObj();
  }


  SRef QepcadAPICallback::operator()(QepcadCls &Q) {
    Word V = Q.GVVL;
    char formType = 'E';
    string res, assumptionsAsUsed;
    Word Fd = Q.GETDEFININGFORMULA(formType,1);
    res = unNormForm2string(Fd,V);
    if (res.length() > 0 && res[0] != '[') {
      if (res == "0 = 0") { res = "[true]"; }
      else if (res == "0 /= 0") { res = "[false]"; }
      else { res = "[" + res + "]"; }
    }
    Word Ad = Q.GETASSUMPTIONS();
    if (Ad == NIL) { // No assumptions used
      assumptionsAsUsed = "[true]";
    }
    else { 
      assumptionsAsUsed = unNormForm2string(Ad,V);
      if (assumptionsAsUsed.length() > 0 && assumptionsAsUsed[0] != '[')
      { assumptionsAsUsed = "[" + assumptionsAsUsed + "]"; }
    }
    return new LisObj(new StrObj(res), new StrObj(assumptionsAsUsed));
  }

  /*
    NOTE: 
   */
  SRef qepcadAPICall(std::string &input, QepcadAPICallback &f, bool use2DOpts) {
    /*chris*/ //use2DOpts=false;
    QepcadCls* p = use2DOpts ? new QepcadCls2D() : new QepcadCls();
    string str_F = input;
    SRef res;
    bool errorFlag = false;
    try { 
      int ac=0;
      char *tmp[] = {NULL};
      char **av = tmp;
      ::BEGINQEPCADLIB(-1);

      // Read input, extracting the formula Fs and the variable list V
      Word Fs, V;
      istringstream sin(str_F);
      ostringstream sout;
      ::PushInputContext(sin);
      ::PushOutputContext(sout);
      try { ::INPUTRD(&Fs,&V); } catch(exception &e)
      { ::PopOutputContext(); ::PopInputContext(); throw TarskiException("qepcad-api fail (INPUTRD)"); }
      ::PopOutputContext();
      ::PopInputContext();

      // Initialize QEPCAD problem
      QepcadCls &Q = *p;
      Q.SETINPUTFORMULA(V,Fs);

      /*chris*/ int t0 = CATime();
      
      // Create CAD & get simplified equivalent formula
      ostringstream warningsAndErrors;
      std::stringstream rest;
      rest << sin.rdbuf();
      ::PushOutputContext(warningsAndErrors);
      try { Q.CADautoConst(rest.str()); } catch(exception &e)
      { ::PopOutputContext(); throw TarskiException("qepcad-api fail (CADautoConst)"); }
      ::PopOutputContext();

      /*chris*/ //int t1 = CATime();
      /*chris*/ //cerr << "CAD construciton time: " << t1-t0 << endl;

      // Output
      if (warningsAndErrors.str() != "") {
	errorFlag = true;
	res = new ErrObj(warningsAndErrors.str());
      }
      else {
	res = f(Q);
	/*chris*/ //int t2 = CATime();
	/*chris*/ //cerr << "plot time: " << t2-t1 << endl;
      }
    }
    catch(QepcadException &e) {
      res = new ErrObj(string("Error! ") + e.what()); errorFlag = true;
    }
    catch(TarskiException &e) {
      res = new ErrObj(string("Error! ") + e.what()); errorFlag = true;
    }
    catch(exception &e) {
      res = new ErrObj(string("Error! Exception!")); errorFlag = true;
    }
    
    // clean it all up
    delete p;
    ENDQEPCAD();
    return res;
  }


  
}
