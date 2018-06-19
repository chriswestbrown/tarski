#include "readSMTRetFormula.h"
#include "smtlib.h"
#include "../formula/formula.h"
#include "../formula/formmanip.h"
#include "../algparse/algparse.h"
#include "../tarskisysdep.h"
#include "../poly/tracker.h"


using namespace SMTLib;
using namespace std;
using namespace tarski;

uint64 GC_Obj::next_tag = 1;
uint64 GC_Obj::num_deleted = 0;
tarski::Tracker compTracker;

int main(int argc, char** argv)
{
  int ac; char **av;
  SacModInit(argc,argv,ac,av,"MapleSac","Maple",pathToMaple);
  PolyManager PM;

  ostringstream sout;
  readSMTRetTarskiString(cin,sout);

  //  TFormRef F = processExpFormula(sout.str(),&PM);
  TFormRef F = processExpFormulaClearDenominators(sout.str(),&PM);

  F->write();
  return 0;
}

