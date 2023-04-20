/*======================================================================
                      CADautoConst(Word Fs)

CAD Atuomatic (i.e. non-interactive) Construction

NOTE:  Assumes the QepcadCAD already has all the relvent fields
       initialized - i.e. GVF, etc.

Inputs:
  instructions : a C++ string <describe later!>

Side Effects
This is a member function of a QepcadCls object.  Calling this function
actually constructs the CAD data structure associated to the object.
What gets constructed is a CAD for the formula Fs.  If the GVUA has
been set to something other than NIL, those assumptions are used.
======================================================================*/
#include "qepcad.h"
#include <sstream>

void QepcadCls::CADautoConst(const string& instructions )
{
       Word A,D,F,F_e,F_n,F_s,Fh,J,P,Q,Ths,f,i,r,t, T;
       /* hide Ths,i,t; */
       Word cL,**cC,cr,ce,ci,*cT,cj,cs,cl,ct; /* Chris variables. */
       Word Cs,Ps,Qs,Pps,Cps,Qps,SF; /* Chris variables. */
       char c1,c2; /* Chris variables. */
       istringstream sin(instructions);
       string next;
       
       
       
Step1: /* Normalize. */
       /* Process Instructions */
       while(sin >> next && next != "go") {
	 if (next == "prop-eqn-const") { PRPROPEC(); }
	 else if (next == "assume") {
	   ostringstream sout;
	   PushOutputContext(sout);
	   PushInputContext(sin);
	   ASSUME();
	   PopInputContext();
	   PopOutputContext();
	   if (sout.str() != "") { FAIL("CADautoConst",sout.str().c_str()); }
	 }
	 else if (next == "full" || next == "full-cad") {
	   if (PCMZERROR)
	     FAIL("CADautoConst","Command \"full-cad\" incompatible with \"measure-zero-erro\"! No change made!\n");
	   else
	     PCFULL = 'y'; 
	 }
	 else { FAIL("CADautoConst","Unknown command!"); }
       }
       FIRST4(GVF,&r,&f,&Q,&Fh);
       F = NORMQFF(Fh);
       if (GVUA != NIL) GVNA = NORMQFF(GVUA);
       GVNQFF = F;
       if (TYPEQFF(F) != UNDET) { goto Return; }

Step2: /* Projection. */
       if (GVUA != NIL) F = LIST3(ANDOP,GVNA,F);
       A = EXTRACT(r,F);
       if (GVUA != NIL) {
	 GVNA = SECOND(F);
	 F = THIRD(F); }
       GVNIP = A;
       GVPF = LLCOPY(A);
       GVLV = r;

       /* Process Instructions */
       while(sin >> next && next != "go") {
	 if (next == "eqn-const-poly") {
	   ostringstream sout;
	   PushOutputContext(sout);
	   PushInputContext(sin);
	   PREQNCONSTPOLY();
	   PopInputContext();
	   PopOutputContext();
	   if (sout.str() != "") { FAIL("CADautoConst",sout.str().c_str()); }
	 }
	 else { FAIL("CADautoConst","Unknown command!"); }
       }

       PROJECTauto(r,A,&P,&J);

Step3: /* Truth-invariant CAD. */
       D = TICADauto(Q,F,f,P,A);

Return: /* Prepare for return. */
       return;
}
