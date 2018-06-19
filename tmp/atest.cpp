#include <iostream>
#include <sstream>
#include "../tarskisysdep.h" /* defines pathToMaple variable */
#include "../poly/sacMod.h"
#include "../formula/normalize.h"
#include "../onecell/memopolymanager.h"
#include "../smtlib/readSMTRetFormula.h"
#include "../shell/qepcad-inter/qepcad-inter.h"

#include "store.h"

using namespace std;
using namespace tarskiapi;

uint64 GC_Obj::next_tag = 1;
uint64 GC_Obj::num_deleted = 0;
namespace tarski { Tracker compTracker; }
void nln() { cout << endl; } // just to help with using gdb

int mainDUMMY(int argc, char **argv, void* topOfTheStack)
{
  using namespace tarski;
  
    // Get the CA Server up and running!
    int ac; char **av = NULL;
    string CASString = "Saclib";
    for(int i = 1; i < argc; ++i)
    {
      if (argv[i] == string("-CASSaclib")) CASString = "Saclib";
      else if (argv[i] == string("-CASMaple"))  CASString = "Maple";
      else if (argv[i] == string("-CASMapleSac")) CASString = "MapleSac";
      else { cerr << "Unknown option '" << argv[i] << "'" << endl; return 1; }
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

    // Set up the basic components
    defaultNormalizer = new Level3and4(7,7);
    MemoizedPolyManager PM;
    Store  ST(defaultNormalizer,&PM);

    // REP-Loop
    string comm, in,sigmas;
    while(cin >> comm)
    {
      if (comm == "r") { // register atoms
	(cin >> sigmas) && getline(cin,in);
	int sigma = ST.SMTLIBRelopInt(sigmas);
	vector<int> res = ST.registerAtom(in,sigma);
	for(int i = 0; i < res.size(); i++)
	  cout << res[i] << ' ';
	cout << endl;
      }
      else if (comm == "g")
      { // get atom in tarski format (mostly for internal use)
	int idx;
	(cin >> sigmas) && (cin >> idx);
	int sigma = ST.SMTLIBRelopInt(sigmas);
	TAtomRef A = ST.getAtom(idx,sigma);
	A->write(); cout << endl;
      }
      else if (comm == "s")
      { // get the string form of a conjunction
	getline(cin,in);
	istringstream sin(in);
	vector<int> q;
	int idx, sigma;
	while(sin >> idx >> sigma) { q.push_back(idx); q.push_back(sigma); }
	string res = ST.getString(q);
	cout << res << endl;
      }
      else if (comm == "t")
      { // test conjunction for satisfiability (printing witness if sat)
	getline(cin,in);
	istringstream sin(in);
	vector<int> q;
	int idx, sigma;
	while(sin >> idx >> sigma) { q.push_back(idx); q.push_back(sigma); }
	int res = 2;
	string witness;
	vector<int>  unsatCore;
	try { res = ST.isSATUC(q,witness,unsatCore); }
	catch(TarskiException &e) {
	  cout << "Error! " << e.what() << endl;
	}
	cout << res << endl;
	if (res == 1)
	  cout << witness;
	else
	{
	  cout << "[";
	  for(int i = 0; i < unsatCore.size(); i++)
	    cout << (i > 0 ? ", " : " ") << unsatCore[i];
	  cout << " ]" << endl;
	}
      }
      
    }
    
    delete defaultNormalizer;
    SacModEnd();
    finalcleanup = true;
    return 0;
}



int main(int argc, char **argv)
{
  int dummy;
  void *topOfTheStack = &dummy;
  mainDUMMY(argc,argv,topOfTheStack);
}
