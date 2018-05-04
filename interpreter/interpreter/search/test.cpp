/*******************************************************************/
const char *helpstring = "\
\n\
Options:\n\
-T      : 'time', causes qfr to print out timing info.\n\
-v      : 'verbose' with it extra information is printed out.\n\
-d      : 'derivation', causes qfr to print out its derivation\n\
-dev    : 'developer' flag.  As developer, I can do what I want with this!\n\
-t <n>  : 'time-out'. Means I give up after <n> minutes.\n\
-l3 <n> : Level 3 normalization flag to be passed along to the normalizer.\n\
-l4 <n> : Level 4 normalization flag to be passed along to the normalizer.\n\
-h      : prints out help info.\n\
-w      : Wraps an extra set of []'s around output\n\
-M <n>  : Set search manager:\n\
          1 - breadth first\n\
          2 - depth first\n\
          3 - smallest dimension first\n\
          4 - fewest quantified variables first\n\
          5 - pure greedy\n\
-G      : Fully generic.  This means that the vanishing leading coefficient\n\
          case for a linear substitution is not generated, regardless of\n\
          whether that coefficient contains bound variables.\n\
-CASSaclib : use Saclib as your only Computer Algebra Server\n\
-CASMaple  : use Maple as your only Computer Algebra Server\n\
-CASMapleSac : use Maple & Saclib together as Computer Algebra Server\n\
\n\
Enter [ <formula> ] where formula is either \"ex <var>,...,<var>[F]\",\n\
\"~ all <var>,...,<var>[F]\", or \"F\", and \"F\" is a quantifier-free\n\
formula.  Either space or * may be used for multiplication.  Otherwise,\n\
operators are as in QEPCAD, except that precedence is understood. Currently\n\
<==> and <== have not been implemented.  The '#' character is for comments,\n\
and comments everything following it until the next newline.  Example:\n\
\n\
# Silly example\n\
[ \n\
  ex x [ a x = b /\\ b /= 0 \\/ x = 1 /\\ b = 0 ] # more comments!\n\
]\n\
";
/*******************************************************************/
#include <algorithm>
#include <sys/types.h>
#include <time.h>
#include <iostream>
#include <stack>
#include "../algparse/algparse.h"
#include "search.h"
#include "grade.h"
#include "rewrite.h"
#include "../formula/formmanip.h"
#include "../tarski.h"
#include "../tarskisysdep.h"
#include "qfr.h"
#include "../poly/tracker.h"

using namespace std;
using namespace tarski;

tarski::TFormRef testmi(tarski::TFormRef F);


void nln() { cout << endl; } // just to help with using gdb
void owrite(Word w) { OWRITE(w); } // just to help with using gdb



/************************************************************
 * BEGIN GLOBAL VARIABLES
 ************************************************************/
uint64 GC_Obj::next_tag = 1;
uint64 GC_Obj::num_deleted = 0;
namespace tarski {
Tracker compTracker;
}
/************************************************************
 * END GLOBAL VARIABLES
 ************************************************************/

int mainDUMMY(int argc, char **argv, void* topOfTheStack)
{
  int l3normalizerFlag = 1; // Default
  int l4normalizerFlag = 1; // Default
  int QManagerTypeFlag = 4; // Default
  bool tackw = false;
  bool printderivation = false;
  { 
    int ac = 0; char **av; // These store arg & argv after saclib strips out its arguments
    string CASString = "MapleSac";
    for(int i = 1; i < argc; ++i)
    {
      if (argv[i] == string("-CASSaclib")) CASString = "Saclib";
      if (argv[i] == string("-CASMaple"))  CASString = "Maple";
      if (argv[i] == string("-CASSingSac"))  CASString = "SingSac";
      if (argv[i] == string("-CASMapleSac")) CASString = "MapleSac";
    }
    if (CASString == "MapleSac") 
      SacModInit(argc,argv,ac,av,"MapleSac","Maple",pathToMaple,topOfTheStack);
    else if (CASString == "SingSac") 
      SacModInit(argc,argv,ac,av,"SingSac","Singular",pathToSingular,topOfTheStack);
    else if (CASString == "Saclib")
      SacModInit(argc,argv,ac,av,"Saclib","","",topOfTheStack);
    else if (CASString == "Maple")
    { 
      SacModInit(argc,argv,ac,av,"Maple","Maple",pathToMaple,topOfTheStack);
    }

    double timeLimit = 60*24*365; // can't let it run for more than a year. :(
    for(int i = 1; i < ac; ++i)
    {
      if (av[i] == string("-v")) { verbose = 1; }
      else if (av[i] == string("-T")) { timing = 1; }
      else if (av[i] == string("-dev")) { developer = 1; }
      else if (av[i] == string("-d")) { printderivation = true; }
      else if (av[i] == string("-t")) {
	if (++i >= ac) { cerr << "Number expected after `-t' option!" << endl; exit(1); }
	timeLimit = atof(av[i]);
	if (timeLimit <= 0) { cerr << "Positive integer argument to `-t' required!" << endl; exit(1); }
      }
      else if (av[i] == string("-l3"))
      {
	if (++i >= ac) { cerr << "Number expected after `-l3' option!" << endl; exit(1); }
	l3normalizerFlag = atoi(av[i]);
      }
      else if (av[i] == string("-l4"))
      {
	if (++i >= ac) { cerr << "Number expected after `-l4' option!" << endl; exit(1); }
	l4normalizerFlag = atoi(av[i]);
      }
      else if (av[i] == string("-h")) {
	cout << "qrf : Quantified Formula Rewriter, Version " 
	     << tarskiVersion << ", " << tarskiVersionDate << endl
	     << helpstring;
	exit(0);
      }
      else if (av[i] == string("-w")) {
	tackw = true;
      }
      else if (av[i] == string("-M"))
      {
	if (++i >= ac) { cerr << "Number expected after `-M' option!" << endl; exit(1); }
	QManagerTypeFlag = atoi(av[i]);
      }
      else if (av[i] == string("-G"))
      {
	fullyGeneric = true;
      }
    }


  PolyManager M;

  // Parse input ... right now we'll assume prenex, existentially quantified, conjunctive normal form
  LexContext LC(cin,';');
  TarskiRef T;
  algparse(&LC,T);    
  if (T.is_null()) { cerr << "Parse error!" << endl; exit(1); }

  // Convert to Varset QVars and quantifier-free TForm F
  TFormRef F, Forig = parseTreeToTForm(T,M);

  if (!isPrenex(Forig))
  {
    cerr << "Input formula must be prenex!" << endl; exit(1);
  }  
  VarSet QVars;
  TQBRef FQB = asa<TQBObj>(Forig);
  if (!FQB.is_null())
  {
    if (FQB->size() > 1 || FQB->outermostBlockType != EXIST) 
    { cerr << "Existentially quantified formulas only!" << endl; exit(1); }
    F = FQB->formulaPart;
    QVars = FQB->blocks[0];
  }
  else
    F = Forig;

  F = getDNF(F);

  // Normalize 
  defaultNormalizer = new Level3and4(l3normalizerFlag,l4normalizerFlag);
  if (asa<TOrObj>(F))
  { 
    TOrRef tor = asa<TOrObj>(F);
    TOrRef tmp = new TOrObj;
    for(set<TFormRef>::iterator itr = tor->disjuncts.begin(); itr != tor->disjuncts.end(); ++itr)
      tmp->OR(defaultNormalizer->normalize(*itr));
    if (tmp->size() == 0) F = new TConstObj(FALSE);
    if (tmp->size() == 1) F = *tmp->disjuncts.begin();
    else F = tmp;
  }
  else
    F = defaultNormalizer->normalize(F);
  if (verbose != 0) {
    cout << "Input formula: " << endl;
    Forig->write(); cout << endl;
    cout << "Normalized to: " << endl;
    F->write(true);  cout << endl;
    if (asa<TConstObj>(F) != NULL) { SacModEnd(); exit(0); }}
  if (asa<TConstObj>(F) != NULL) { F->write(true); cout << endl; SacModEnd(); exit(0); }

  // create the initial queue Q and QueueManager
  QueueManagerAndChooser *QMaC;
  switch(QManagerTypeFlag)
  {
  case 1: QMaC = new BFQueueManager; break; 
  case 2: QMaC = new DFQueueManager; break; 
  case 3: QMaC = new PrioritySearchQueueManager<SmallestDimensionFirst>; break;
  case 4: QMaC = new PrioritySearchQueueManager<FewestQuantifiedVariablesFirst>; break;
  case 5: QMaC = new GreedyQueueManager; break;
  } 
  QueueManager *globalQM = QMaC;
  BasicRewrite ReW(globalQM);
  TFQueueRef Q = new TFQueueObj(globalQM);

  
  // finish intialization of QueueManager if needed
  SimpleGradeForQEPCAD tmpG;
  if (QManagerTypeFlag == 5) { dynamic_cast<GreedyQueueManager*>(QMaC)->setData(Q,tmpG); }

  // Initialize queue Q
  if (asa<TAndObj>(F) || asa<TAtomObj>(F))
    Q->enqueue(new QAndObj(F,QVars,0,new PInitialObj()));
  else if (asa<TOrObj>(F))
  {
    TOrRef tor = asa<TOrObj>(F);
    vector<QAndRef> AV;
    for(set<TFormRef>::iterator itr = tor->disjuncts.begin(); itr != tor->disjuncts.end(); ++itr)
      AV.push_back(new QAndObj(*itr,QVars,0,new PInitialObj()));
    globalQM->enqueueOR(AV,Q);
    QMaC->reorganize(Q); //-- Must do to clean up in case of e.g. duplicate disjuncts
  }
  else
  { cerr << "Error! Unknown formula type to begin search!" << endl; exit(1); }



  bool gaveUpFlag = false;
  int mainLoopCount = 0;
  double giveUpTime = 60*timeLimit; // giveUpTime in seconds!
  Word t0 = ACLOCK();
  time_t t0_wall_clock = time(0);
  QAndRef nextAnd;
  while(Q->constValue.is_null() && !(nextAnd = QMaC->next()).is_null())
  {    
    //cerr << "Q(" << Q->tag << ")->nodes.size() == " << Q->nodes.size() << endl;
    //graphFile(Q);
    // Expand the node "nextAnd"
    TFormRef res = ReW.refine(nextAnd,QMaC->find(nextAnd));

    // React if refine returned nextAnd either identically TRUE or FALSE
    if (constValue(res) == 1) { Q->constValue = res; break; }
    if (constValue(res) == 0) { QMaC->notify(QInfo::equivFalse(nextAnd->parentQueue)); }

    // Reorganization of "the graph" might be necessary based on things found equiv or false
    QMaC->reorganize(Q);

    //graphStats(Q);
    //cout << "Total # of canonical poly factors = " << M.polyset.size() << endl;


    ++mainLoopCount;    
    if (verbose) { cerr << "Completed loop " << mainLoopCount << endl; }
    if (time(0) - t0_wall_clock >= giveUpTime)
    {
      gaveUpFlag = true;
      cerr << "Stopped search because of " << giveUpTime/60 << " minute time limit!" << endl;
      break;
    }
  }


  // EXPREIMENTAL
  if (false)
  {
    TAndRef Coll = new TAndObj();
    for(set<QNodeRef>::iterator itr = Q->nodes.begin(); itr != Q->nodes.end(); ++itr)
    {
      QAndRef r = dynamic_cast<QAndObj*>(&**itr);
      if (!r.is_null()) { Coll->AND(r->F); }
    }
    cout << "Conjunction of everything in root node is: " << endl;
    Coll->write();
    cout  << endl;
    tarski::testmi(Coll);
  }
  if (false)
  {
    cout << "Polynomials deduced non-zero in original context: " << endl;
    set<IntPolyRef> zp;
    
    for(map<uint64, set<IntPolyRef> >::iterator itr = QMaC->nonZeroDeductions.begin();
	itr != QMaC->nonZeroDeductions.end(); ++itr)
    {
      TFQueueObj *tmpp = QMaC->currentQueue((*itr).first);
      if (tmpp != NULL && tmpp->tag == Q->tag)
	for(set<IntPolyRef>::iterator pitr = (*itr).second.begin(); pitr != (*itr).second.end(); ++pitr)
	  zp.insert((*pitr));
    }
    
    for(set<IntPolyRef>::iterator itr = zp.begin(); itr != zp.end(); ++itr)
    {
      (*itr)->write(M);
      cout << endl;
    }
    cout << endl;
  }


  // OUTPUT SOME INFO
  if (gaveUpFlag == false && false) printUnexpandedNodes(Q); // Look at this!
  if (verbose) {
    cerr << "mainLoopCount = " << mainLoopCount << endl;
    STATSACLIB();
    M.getStats()->print(cerr); }

  if (!Q->constValue.is_null())
  {
    if (verbose) { cout << "Input Equivalent to: "; }
    if (tackw) { cout << "[ "; }
    Q->constValue->write();
    if (tackw) { cout << " ]"; }
    cout << endl;
  }    
  else
  {
    if (verbose) QMaC->graphFile(Q);
    if (verbose) { cout << "Input Equivalent to: "; }

    SimpleGradeForQEPCAD SG;
    MinFormFinder MF; MF.process(Q,SG); 
    pair<TFormRef,double> minp; minp.first = MF.getMinFormula(Q); minp.second = MF.getMinGrade(Q);
    if (tackw) { cout << "[ "; }
    minp.first->write(true);
    if (tackw) { cout << " ]"; }
    cout << endl;
    if (verbose || printderivation)
    {
      TOrRef tor = new TOrObj();
      tor->OR(minp.first);
      for(set<TFormRef>::iterator itr = tor->disjuncts.begin(); itr != tor->disjuncts.end(); ++itr)
      {
	QNodeRef qnsa = MF.fromTtoQ[(*itr)->tag];
	QAndRef sa = asa<QAndObj>(qnsa);
	if (!sa.is_null())
	{
	  cout << "Derivation is:" << endl;
	  stack<QAndRef> S_p;
	  while(!sa->PR->initial())
	  {
	    S_p.push(sa);
	    sa = sa->PR->predecessor();
	  }
	  while(!S_p.empty())
	  {
	    S_p.top()->PR->write();
	    cout << endl;
	    S_p.pop();
	  }
	}
      }
    }
    cout << endl;
    if (verbose) { cout << "min grade  = " << minp.second << endl; }

    if (verbose)
    {
      cout << endl << endl;
      vector<QAndRef> MM;
      for(set<QNodeRef>::iterator itr = Q->nodes.begin(); itr != Q->nodes.end(); ++itr)
	if (asa<QAndObj>(*itr))
	{
	  MM.push_back(asa<QAndObj>(*itr));
	}
      sort(MM.begin(),MM.end(),QAndGradeComp(SG));

      for(int i = MM.size()-1; i >= 0; --i)
      { 
	MM[i]->write(true);
	cout << endl;
	printDerivation(MM[i]);
	cout << endl << endl;
      }
    }
    //    exploreQueue(Q,QVars,SG);

  }

  if (verbose || timing)
  {
    cout << "Wall clock time (after initialization) is " << time(0) - t0_wall_clock << " seconds." << endl;  
    CAStats();
    QMaC->graphStats(Q);
    //QMaC->graphFile(Q);
  }

  // This is the crucial cleanup!  I have to do nextAnd.free() and Q.free() before the others!
  finalcleanup = true;
  nextAnd.free();
  Q.free();
  delete QMaC;
  SacModEnd();
  }
  if (verbose) {
    cerr << "Cleanup: " << GC_Obj::next_tag - 1 << " " << GC_Obj::num_deleted;
    nln(); 
    owrite(0);
  }
  exit(0);
  return 0;
}

int main(int argc, char **argv)
{
  int dummy;
  mainDUMMY(argc,argv,&dummy);
}
