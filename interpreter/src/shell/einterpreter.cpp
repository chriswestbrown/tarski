#include "einterpreter.h"
#include "writeForQE.h"
#include "utils.h"

#include "../smtlib/smtlib-comm.h"
#include "../realroots/RealAlgNumType.h"
#include "../search/clearAssignments-comm.h"
#include "qepcad-inter/qepcad-inter.h"
#include "../onecell/OCBuilderType.h"
#include "../onecell/OpenNuCADType.h"
#include "funcEvalFormAtRat.h"
#include "funcSyntax.h"
#include "../search/qfr.h"
#include "../nucad/TestComm.h"
//#include "../fernando/sample-comm.h"
//#include "../fernando/type-comm.h"
//#include "../fernando/sactester.h"
#include "../bbwb/BBSat.h"
#include "../bbwb/wbsat-comm.h"
#include "../bbwb/bbwb.h"
#include "../smtsolver/box-solver-comm.h"
#include "../smtsolver/qep-solver-comm.h"

using namespace std;

namespace tarski {

class FindFactors : public TFPolyFun // Finds non-constant factors of LHS's
{
public:
  set<IntPolyRef> res;
  virtual void action(TAtomObj* p)
  {
    for(map<IntPolyRef,int>::iterator itr = p->factorsBegin(); itr != p->factorsEnd(); ++itr)
      if (!itr->first->isConstant())
	res.insert(itr->first);
  } 
  virtual void action(TConstObj* p) {  }
  virtual void action(TAndObj* p)
  { 
    for(set<TFormRef>::iterator i = p->conjuncts.begin(); i != p->conjuncts.end(); ++i)
      this->actOn(*i);
  }
  virtual void action(TOrObj* p)
  { 
    for(set<TFormRef>::iterator i = p->disjuncts.begin(); i != p->disjuncts.end(); ++i)
      this->actOn(*i); 
  }
};


void generateQVTSpossibilities(TFormRef F, IntPolyRef p, VarSet v)
{
  PolyManager &PM = *(F->getPolyManagerPtr());
  if (p->degree(v) != 2) { cerr << "This requires a quadratic!" << endl; return; }
  IntPolyRef a = PM.coef(p,v,2);
  IntPolyRef b = PM.coef(p,v,1);
  IntPolyRef c = PM.coef(p,v,0);
  FactRef Fa = new FactObj(PM); Fa->addMultiple(a,1); 
  FactRef Fb = new FactObj(PM); Fb->addMultiple(b,1); 
  FactRef Fc = new FactObj(PM); Fc->addMultiple(c,1); 
  cout << "a = "; Fa->write(); cout << endl;
  cout << "b = "; Fb->write(); cout << endl;
  cout << "c = "; Fc->write(); cout << endl;

  FindFactors FF;
  FF(F);
  cout << "IN QVTS: " << endl;
  // Loop through each factor and print out a,b,c,s,r,a*,c*
//   for(set<IntPolyRef>::iterator itr = FF.res.begin(); itr != FF.res.end(); ++itr)
//   {
//     IntPolyRef g = *itr;
//     int d = g->degree(v); 
//     if (d <= 0) continue;
//     cout << "g: ";
//     g->write(PM);
//     cout << endl;

//     cout << "R: ";
//     FactRef R = new FactObj(PM); 
//     R->addMultiple(PM.resultant(p,g,v),1); 
//     if (d % 2) R->addFactors(Fa,1);
//     R->write();
//     cout << endl;

//     cout << "PR:";
//     IntPolyRef PR = PM.prem(g,p,v); 
//     PR->write(PM);
//     cout << endl;

//     IntPolyRef r = PM.coef(PR,v,1);
//     IntPolyRef s = PM.coef(PR,v,0);

//     FactRef Fr = new FactObj(PM); Fr->addMultiple(r,1); 
//     FactRef Fs = new FactObj(PM); Fs->addMultiple(s,1); 
//     cout << "r: "; Fr->write(); cout << endl;
//     cout << "s: "; Fs->write(); cout << endl;

//     IntPolyRef as = PM.dif(PM.prod(new IntPolyObj(2),PM.prod(a,s)),PM.prod(b,r));
//     FactRef Fas = new FactObj(PM); Fas->addMultiple(as,1);
//     cout << "as:";
//     Fas->write();
//     cout << endl;
    
//     IntPolyRef cs = PM.dif(PM.prod(new IntPolyObj(2),PM.prod(c,r)),PM.prod(b,s));
//     FactRef Fcs = new FactObj(PM); Fcs->addMultiple(cs,1);
//     cout << "cs:";
//     Fcs->write();
//     cout << endl;
//   }
  

  // Strict Weispfenning
  TAndRef FAWp1 = new TAndObj;
  TAndRef FAWm1 = new TAndObj;
  
  // Alt Weispfenning
  TAndRef FAWAp1 = new TAndObj;
  TAndRef FAWAm1 = new TAndObj;

  // Alt Brown Assuming a c < 0
  TAndRef FAWABp1 = new TAndObj;
  TAndRef FAWABm1 = new TAndObj;

  // Just for testing: Assume F is a conjunction single-factor atoms, and apply W's default subst
  TAndRef C = new TAndObj;; C->AND(F);
  for(TAndObj::conjunct_iterator itr = C->conjuncts.begin(); itr != C->conjuncts.end(); ++itr)
  {
    cout << "PROCESSING: "; (*itr)->write(); cout << endl << endl;
    TAtomRef A = asa<TAtomObj>(*itr); if (A.is_null()) { cerr << "Atom expected!" << endl; continue; }

    if (!(A->getVars() & v).any())
    { 
      FAWp1->AND(A);  FAWm1->AND(A);
      FAWAp1->AND(A); FAWAm1->AND(A); 
      FAWABp1->AND(A); FAWABm1->AND(A); 
      continue; 
    }

    if (A->F->content != 1) { cerr << "Not ready for unnormalized formulas!" << endl; continue; }
    if (A->F->numFactors() != 1) { cerr << "Not ready for multifactor or constant LHS's!" << endl; continue; }
    IntPolyRef g = A->factorsBegin()->first;
    if (A->relop != LTOP && A->relop != GTOP) { cerr << "Only ready for < and > right now!" << endl; continue; }
    if (A->relop == GTOP) { g = PM.neg(g); }

    int d = g->degree(v); 
    if (d <= 0) continue;
    cout << "g: ";
    g->write(PM);
    cout << endl;

    cout << "R: ";
    FactRef R = new FactObj(PM); 
    R->addMultiple(PM.resultant(p,g,v),1); 
    if (d % 2) R->addFactors(Fa,1);
    R->write();
    cout << endl;

    cout << "PR:";
    IntPolyRef PR = PM.prem(g,p,v); 
    PR->write(PM);
    cout << endl;

    IntPolyRef r = PM.coef(PR,v,1);
    IntPolyRef s = PM.coef(PR,v,0);

    FactRef Fr = new FactObj(PM); Fr->addMultiple(r,1); 
    FactRef Fs = new FactObj(PM); Fs->addMultiple(s,1); 
    cout << "r: "; Fr->write(); cout << endl;
    cout << "s: "; Fs->write(); cout << endl;

    IntPolyRef as = PM.dif(PM.prod(new IntPolyObj(2),PM.prod(a,s)),PM.prod(b,r));
    FactRef Fas = new FactObj(PM); Fas->addMultiple(as,1);
    cout << "as:";
    Fas->write();
    cout << endl;

    IntPolyRef cs = PM.dif(PM.prod(new IntPolyObj(2),PM.prod(c,r)),PM.prod(b,s));
    FactRef Fcs = new FactObj(PM); Fcs->addMultiple(cs,1);
    cout << "cs:";
    Fcs->write();
    cout << endl;

    FactRef F1 = d % 2 ? Fas*Fa : Fas;
    FactRef F2 = d % 2 ? Fr*Fa : Fas;
    
    // Strict Weispfenning
    FAWp1->AND(
	     atom(F1,LTOP) && atom(R,GTOP) || atom(F2,LEOP) && ( atom(F1,LTOP) || atom(R,LTOP) ) );
    FAWm1->AND(
	     atom(F1,LTOP) && atom(R,GTOP) || atom(F2,GEOP) && ( atom(F1,LTOP) || atom(R,LTOP) ) );
    
    // Alt Weispfenning
    FAWAp1->AND(
	      atom(F1,LTOP) && atom(R,GTOP) || atom(F2,LTOP) && ( atom(F1,LTOP) && atom(R,EQOP) || atom(R,LTOP) ) );
    FAWAm1->AND(
	      atom(F1,LTOP) && atom(R,GTOP) || atom(F2,GTOP) && ( atom(F1,LTOP) && atom(R,EQOP) || atom(R,LTOP) ) );

    // Alt Brown assuming a c < 0
    FactRef F1B = d % 2 ? Fs : Fs*Fa;
    FactRef F2B = d % 2 ? Fcs : Fcs*Fa;
    F2B->content = -F2B->content;
    FAWABp1->AND(
	      atom(F1B,LTOP) && atom(R,GTOP) || atom(F2,LTOP) && ( atom(F1B,LTOP) && atom(R,EQOP) || atom(R,LTOP) ) );
    FAWABm1->AND(
	      atom(F1B,LTOP) && atom(R,GTOP) || atom(F2,GTOP) && ( atom(F1B,LTOP) && atom(R,EQOP) || atom(R,LTOP) ) );
  }

  (FAWp1 || FAWm1)->write(); cout << endl;
  (FAWAp1 || FAWAm1)->write(); cout << endl;
  cout << endl << "BROWN: " << endl;
  FAWABp1->write();
  cout << endl << "\\/" << endl;
  FAWABm1->write();
  cout << endl;
  //  (FAWABp1 || FAWABm1)->write(); cout << endl;
  
}



//PolyManager PM;
//Normalizer *defaultNormalizer = NULL;
//bool finalcleanup = false;

bool dbf1 = false;
//int timing = 0;
//int verbose = 0;
//int developer = 0;

map<string,GC_Hand<GC_Obj> > symbolTable;

template<class ObjType>
ObjType* fetch(const string &fname)
{
  GC_Hand<GC_Obj> h = symbolTable[fname];
  return dynamic_cast<ObjType*>(&*h);
}

TFormRef QformToRegularForm(TFormRef F, VarSet QVars)
{
  if (asa<TAndObj>(F))
  {
    TAndRef Fa = asa<TAndObj>(F);
    VarSet QVarsAppearing = /*Fa->QVars*/ QVars & Fa->getVars();
    if (QVarsAppearing.none()) 
      return Fa;
    else 
    {
      TQBObj *p = new TQBObj(QVarsAppearing,EXIST,Fa,Fa->getPolyManagerPtr());
      return p;
    }
  }
  else if (asa<TOrObj>(F))
  {
    TOrRef Fo = asa<TOrObj>(F);
    TOrRef p = new TOrObj();
    for(set<TFormRef>::iterator itr = Fo->disjuncts.begin(); itr != Fo->disjuncts.end(); ++itr)
      p->OR(QformToRegularForm(*itr,QVars));
    return p;
  }
  else
  { 
    VarSet QVarsAppearing = QVars & F->getVars();
    if (QVarsAppearing.none()) 
      return F;
    else 
    {
      TQBObj *p = new TQBObj(QVarsAppearing,EXIST,F,F->getPolyManagerPtr());
      return p;
    }
  }
}


class CommFactor : public EICommand
{
public:
  CommFactor(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  { 
    //    vector<SRef> &vars = args[1]->lis()->elts;
    LisRef vars = args[1]->lis();
    VarSet S;
    for(int i = 0; i < vars->length(); ++i)
    {
      if (vars->get(i)->type() != _sym) 
      { 
	return new ErrObj("Function 'factor' received as 2nd argument a list that contains non-symbol element '" + vars->get(i)->toStr() + "'."); 
      }
      S = S + interp->PM->getVar(vars->get(i)->sym()->val);
    }
    
    vector<TFormRef> V = factorDisjointVars(args[0]->tar()->val,S);
    LisRef L = new LisObj();
    for(int i = 0; i < V.size(); ++i)
    {
      L->push_back(new TarObj(V[i]));
    }
    return L;
  }
  string testArgs(vector<SRef> &args)
  {
    return require(args,_tar,_lis);
  }
  string doc() 
  {
    return "(factor F L), where F is a Tarski Formula and L is a list of variables, returns\
a list of conjuncts representing a ???";
  }
  string usage() { return "(factor <tarski formula> <variable list>)"; }
  string name() { return "factor"; }
};

class CommUFactor : public EICommand
{
public:
  CommUFactor(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  { 
    TFormRef T = args[0]->tar()->val;
    map<TFormRef,VarSet> M = atomsContainingUniqueVar(T,T->getVars());
    LisRef L = new LisObj();
    for(map<TFormRef,VarSet>::iterator itr = M.begin(); itr != M.end(); ++itr)
    {
      LisRef A = new LisObj(); 
      A->push_back(new TarObj((*itr).first));
      VarSet V = itr->second;
      LisRef VL = new LisObj();
      for(VarSet::iterator vitr = V.begin(); vitr != V.end(); ++vitr)
	VL->push_back(new SymObj(T->getPolyManagerPtr()->getName(*vitr)));
      A->push_back(VL);
      L->push_back(A);
    }
    return L;
  }
  string testArgs(vector<SRef> &args)
  {
    return require(args,_tar);
  }
  string doc() 
  {
    return "(ufactor F), where F is a Tarski Formula, returns\
a list of conjuncts representing a ???";
  }
  string usage() { return "(ufactor <tarski formula>)"; }
  string name() { return "ufactor"; }
};

  
class CommExclose : public EICommand
{
public:
  CommExclose(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  { 
    TFormRef T = args[0]->tar()->val;
    return new TarObj(exclose(T));      
  }
  string testArgs(vector<SRef> &args)
  {
    return require(args,_tar);
  }
  string doc() 
  {
    return "(exclose F), where F is a Tarski Formula, returns\
the existential closure of F. ";
  }
  string usage() { return "(exclose <tarski formula>)"; }
  string name() { return "exclose"; }
};


class CommQFR : public EICommand
{
public:
  CommQFR(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  { 
    SRef res;
    {
      //      finalcleanup = false;
      TFormRef T = args[0]->tar()->val;
      QFR qfr;
      try {
	if (qfr.init(5,T,interp->PM)) { return new ErrObj("Bad input format for QFR."); }
	qfr.rewrite();
	TFormRef Tp = QformToRegularForm(qfr.getBest(),qfr.getQuantifiedVariables());
	res = new TarObj(Tp);
      }
      catch(TarskiException &e)
      {
	res = new ErrObj(e.what());
      }
      //      finalcleanup = true;
    } 
    return res;
  }
  string testArgs(vector<SRef> &args)
  {
    return require(args,_tar);
  }
  string doc() 
  {
    return "(qfr F), where F is a Tarski Formula, returns\
a Tarski Formula resulting from applying qfr.";
  }
  string usage() { return "(qfr <tarski formula>)"; }
  string name() { return "qfr"; }
};


class CommNormalize : public EICommand
{
public:
  CommNormalize(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  { 
    SymRef t;
    NumRef k;
    TFormRef F;
    if (args.size() == 1)
    {
      F = args[0]->tar()->val;
      if (isAndAndAtoms(F))
	return new TarObj(defaultNormalizer->normalize(F));
      t = new SymObj("raw");
      k = new NumObj(5);
    }
    else
    {
      t = args[0]->sym();
      k =  args[1]->num();
      F = args[2]->tar()->val;
    }
    if (t->val == "raw")
    {
      RawNormalizer R(defaultNormalizer);
      R(F);
      return new TarObj(R.getRes());
    }
    if (t->val == "level")
    {
      Normalizer* p = NULL;
      switch(k->numerator())
      {
      case 1: p = new Level1(); break;
      case 2: p = new Level2(); break;
      case 3:
      case 4: p = new Level3and4(7,7); break;
      case 15: p = new Level1_5(); break;
      default: throw TarskiException("Unknown normalization level!");
      }
      RawNormalizer R(*p);
      R(F);
      delete p;
      return new TarObj(R.getRes());
    }
    else
      return new ErrObj("Error in normtest: unknown norm type '" + t->val + "'.");
  }
  string testArgs(vector<SRef> &args)
  {
    string r1 = require(args,_tar);
    if (r1 == "") return r1;
    string r2 = require(args,_sym,_num,_tar);
    return r2 == "" ? r2 : r1;
  }
  string doc() 
  {
    return "(normalize F), where F is a Tarski Formula, returns\
a Tarski Formula resulting from applying the standard normalizer to F, or\
(normtest t k F), where F is a Tarski Formula, returns\
a Tarski Formula resulting from applying a normalizer of type t \
with normalization level k to F.  Supported types are 'raw or 'level.\
Values for 'level are 1, 2, 3, 4 or 15.";
  }
  string usage() { return "(normalize <tarski formula>) or (normalize <sym> <num> <tarski formula>)"; }
  string name() { return "normalize"; }
};


  class CommSuggQepcad : public EICommand
  {
  public:
    CommSuggQepcad(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, vector<SRef> &args) 
    { 
      TFormRef T = args[0]->tar()->val;      
      string res;
      try {
	TFormRef assumptions;
	res = naiveButCompleteWriteForQepcad(T,assumptions);
      }
      catch(TarskiException e)
      {
	return new ErrObj(e.what());
      }
      return new StrObj(res);
    }
    string testArgs(vector<SRef> &args)
    {
      return require(args,_tar);
    }
    string doc() 
    {
      return "(suggest-qepcad F), where F is a Tarski Formula, prints out a \
suggested qepcad phrasing for input F.  Note that F must be a quantified, prenex formula.";
    }
    string usage() { return "(suggest-qepcad <tarski formula>)"; }
    string name() { return "suggest-qepcad"; }
  };

class CommGetSub : public EICommand
{
public:
  CommGetSub(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  {
    TFormRef T = args[0]->tar()->val;
    int i = args[1]->num()->numerator();      
    GetSubFormula GS(i);
    GS(T);
    switch(GS.getFailCode())
    {
    case 0:{
      return new TarObj(GS.getRes());
    }break;
    case 1:{
      return new ErrObj("Function getsub called on formula that doesn't support it.");
    }break;
    case 2:{
      return new ErrObj("Function getsub called with too large of an index.");
    }break;
    default:
      throw TarskiException("Unexpected condition in CommGetSub::execute!");
    }
  }
  string testArgs(vector<SRef> &args)
  {
    return require(args,_tar,_num);
  }
  string doc() 
  {
    return "(getsub F i), where F is a Tarski Formula and i an integer, returns the ith element of F.  This requires that F is a conjunction or disjunction. Indices start at 0.";
  }
  string usage() { return "(getsub <tarski formula> <integer>)"; }
  string name() { return "getsub"; }
};


class CommGetArgs : public EICommand
{
public:
  CommGetArgs(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  {
    TFormRef T = args[0]->tar()->val;
    LisRef res = new LisObj(); 
    TAndObj *tand = 0; TOrObj* tor = 0; TQBObj *tqb = 0;
    if ((tand = asa<TAndObj>(T))) {
      for(TAndObj::conjunct_iterator itr = tand->conjuncts.begin(); itr != tand->conjuncts.end(); ++itr)
	res->push_back(new TarObj(*itr));
    }
    if ((tor = asa<TOrObj>(T))) {
      for(TOrObj::disjunct_iterator itr = tor->disjuncts.begin(); itr != tor->disjuncts.end(); ++itr)
	res->push_back(new TarObj(*itr));
    }
    else if ((tqb = asa<TQBObj>(T))) { res->push_back(new TarObj(tqb->formulaPart)); }
    return res;
  }
  string testArgs(vector<SRef> &args)
  {
    return require(args,_tar);
  }
  string doc() 
  {
    return "(getargs F), where F is a Tarski Formula returns a list of the argument subformulas if F is an AND, OR, or quantified block, and an empty list otherwise.";
  }
  string usage() { return "(getargs <tarski formula>)"; }
  string name() { return "getargs"; }
};



class CommAnd : public EICommand
{
public:
  CommAnd(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  {
    TFormRef T1 = args[0]->tar()->val;
    TFormRef T2 = args[1]->tar()->val;
    return new TarObj(T1 && T2);
  }
  string testArgs(vector<SRef> &args) { return require(args,_tar,_tar); }
  string doc() 
  {
    return "(t-and F1 F2), where F1 and F2 are Tarski Formulas, returns the Tarski formula F1 /\\ F2.";
  }
  string usage() { return "(t-and <t-tarski formula> <tarski formula>)"; }
  string name() { return "t-and"; }
};


class CommOr : public EICommand
{
public:
  CommOr(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  {
    TFormRef T1 = args[0]->tar()->val;
    TFormRef T2 = args[1]->tar()->val;
    return new TarObj(T1 || T2);
  }
  string testArgs(vector<SRef> &args) { return require(args,_tar,_tar); }
  string doc() 
  {
    return "(t-or F1 F2), where F1 and F2 are Tarski Formulas, returns the Tarski formula F1 \\/ F2.";
  }
  string usage() { return "(t-or <t-tarski formula> <tarski formula>)"; }
  string name() { return "t-or"; }
};

class CommNeg : public EICommand
{
public:
  CommNeg(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  {
    TFormRef T1 = args[0]->tar()->val;
    return new TarObj(T1->negate());
  }
  string testArgs(vector<SRef> &args) { return require(args,_tar); }
  string doc() 
  {
    return "(t-neg F), where F is a Tarski Formula, returns the Tarski formula ~F.";
  }
  string usage() { return "(t-neg <t-tarski formula>)"; }
  string name() { return "t-neg"; }
};


class CommEx : public EICommand
{
public:
  CommEx(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  {
    PolyManager* pPM = getPolyManagerPtr();
    LisRef L = args[0]->lis();
    VarSet V = 0;
    for(int i = 0; i < L->length(); i++)
    {
      SymRef s = L->get(i)->sym();
      if (s.is_null()) { throw TarskiException("t-ex expects first argument to be a list of symbols."); }
      VarSet t = pPM->addVar(s->getVal()); 
      V = V + t;
    }
    TFormRef T1 = args[1]->tar()->val;
    return new TarObj(new TQBObj(V,EXIST,T1));
  }
  string testArgs(vector<SRef> &args) { return require(args,_lis,_tar); }
  string doc() 
  {
    return "(t-ex L F), where L = (x1 x2 ... xk) is a list of symbols and F is a Tarski Formula, returns the Tarski formula ex x1, x2, ..., xk [ F ].";
  }
  string usage() { return "(t-ex <var-list> <tarski-formula>)"; }
  string name() { return "t-ex"; }
};


class CommAll : public EICommand
{
public:
  CommAll(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  {
    PolyManager* pPM = getPolyManagerPtr();
    LisRef L = args[0]->lis();
    VarSet V = 0;
    for(int i = 0; i < L->length(); i++)
    {
      SymRef s = L->get(i)->sym();
      if (s.is_null()) { throw TarskiException("t-all expects first argument to be a list of symbols."); }
      VarSet t = pPM->addVar(s->getVal()); 
      V = V + t;
    }
    TFormRef T1 = args[1]->tar()->val;
    return new TarObj(new TQBObj(V,UNIVER,T1));
  }
  string testArgs(vector<SRef> &args) { return require(args,_lis,_tar); }
  string doc() 
  {
    return "(t-all L F), where L = (x1 x2 ... xk) is a list of symbols and F is a Tarski Formula, returns the Tarski formula all x1, x2, ..., xk [ F ].";
  }
  string usage() { return "(t-all <var-list> <tarski-formula>)"; }
  string name() { return "t-all"; }
};



class CommSub : public EICommand
{
public:
  CommSub(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  {
    // Deal properly with formulas that have quantifiers ... what about ORs?
    
    const string s = args[0]->sym()->val;
    VarSet x = interp->PM->getVar(s);
    if (x.none()) return args[2]; // variable x not found in PM, thus not in F
    IntPolyRef p = args[1]->alg()->val;
    TFormRef F = args[2]->tar()->val;
    VarSet V = p->getVars() + F->getVars() + x;
    VarSet z = interp->PM->newVar(V);
    IntPolyRef q = interp->PM->dif(new IntPolyObj(z),new IntPolyObj(x));
    TAtomRef A = makeAtom(*(interp->PM),q,EQOP);
    TFormRef target = A && F;
    TFormRef Fp = linearSubstGenericCase(*(interp->PM),target,q,x,A,false);
    
    IntPolyRef qp = interp->PM->dif(new IntPolyObj(z),p);
    TAtomRef Ap = makeAtom(*(interp->PM),qp,EQOP);
    TFormRef targetp = Ap && Fp;
    TFormRef res = linearSubstGenericCase(*(interp->PM),targetp,qp,z,Ap,false);
    return new TarObj(res);
  }
  string testArgs(vector<SRef> &args) { return require(args,_sym,_alg,_tar); }
  string doc() 
  {
    return "(sub x P F), where x is a vairable, P is a polynomial, and F is a tarski formula, returns the result of substituting P for x in F  Example: (sub 's [t + 1] [ s t < 0 ]) gives [(t)(t + 1) < 0]:tar.";
  }
  string usage() { return "(sub <variable> <polynomial> <tarski formula>)"; }
  string name() { return "sub"; }
};

  class CommHelp : public EICommand
  {
  public:
    CommHelp(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef eval(SRef input, vector<SRef> &args) 
    {
      string tmp;
      if (args.size() == 0)
      {
	ostringstream sout;
	sout << interp->builtinHelpString() << endl;

	sout << "Extended Types:" << endl;
	for(int i = 0; i < interp->typeCache.size(); i++)
	  sout << interp->typeCache[i]->name() << " - "
	       << interp->typeCache[i]->shortDescrip()
	       << endl;
	sout << endl;

	sout << "Extended Functions:" << endl;
	for(int i = interp->OFFSET; i < interp->X.size(); ++i)
	  sout << interp->X[i]->name() << endl;
	writeToFit(sout.str(),cout,getTermWidth());
	return new SObj();
	//return new StrObj(sout.str());
      }
      else if ((tmp = require(args,_sym)) != "")
	return new ErrObj(tmp);
      else
      {
	ostringstream sout;
	int tindex = 0;
	while(tindex < interp->typeCache.size() && 
	      interp->typeCache[tindex]->name() != 
	      args[0]->sym()->val)
	  tindex++;
	if (tindex != interp->typeCache.size())
	{
	  sout << interp->typeCache[tindex]->name() << " - "
	       << interp->typeCache[tindex]->shortDescrip()
	       << endl 
	       << "Messages defined for type " << interp->typeCache[tindex]->name() << " are:" << endl;
	  for(int i = 0; !interp->typeCache[tindex]->isMsgNull(i); i++)
	    sout <<  interp->typeCache[tindex]->getMsgLabel(i) << " : " <<  interp->typeCache[tindex]->getMsgDesc(i) << endl;
	}
	else
	{
	  SRef f = interp->eval(interp->rootFrame,args[0]);
	  if (f->type() == _err) { return new ErrObj("Name '" + args[0]->sym()->val + "' unkown in help!"); }
	  int n = -f->fun()->builtin; 
	  if (f->type() != _fun ||  n < interp->OFFSET || n >= interp->X.size())
	  {
	    return new ErrObj("Argument to help not a builtin or extended function name.");
	  }
	  else
	  {
	    EICommand *p = interp->X[n];
	    sout << p->name() << ", usage: " << p->usage() << "\n" << p->doc();
	  }
	}
	writeToFit(sout.str(),cout,getTermWidth());
	return new SObj();	
	//return new StrObj(p->name() + ", usage: " + p->usage() + "\n" + p->doc());
      }
    }
    SRef execute(SRef input, vector<SRef> &args) { return 0; }
    string testArgs(vector<SRef> &args) { return ""; }
    string doc() 
    {
      return "The help command with no arguments lists commands.  With one argument prints the documentation for that command.";
    }
    string usage() { return "(help [<command-name>)]"; }
    string name() { return "help"; }
  };

class CommDnf : public EICommand
{
public:
  CommDnf(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  {
    return new TarObj(getDNF(args[0]->tar()->val));
  }
  string testArgs(vector<SRef> &args) { return require(args,_tar); }
  string doc() 
  {
    return "(dnf F), where F is a tarski formula, returns a DNF equivalent to F.";
  }
  string usage() { return "(dnf <tarski formula>)"; }
  string name() { return "dnf"; }
};


class CommNormTest : public EICommand
{
public:
  CommNormTest(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  { 
    SymRef t = args[0]->sym();
    NumRef k =  args[1]->num();
    TFormRef F = args[2]->tar()->val;
    if (t->val == "raw")
    {
      RawNormalizer R(defaultNormalizer);
      R(F);
      return new TarObj(R.getRes());
    }
    else
      return new ErrObj("Error in normtest: unknown norm type '" + t->val + "'.");
  }
  string testArgs(vector<SRef> &args)
  {
    return require(args,_sym,_num,_tar);
  }
  string doc() 
  {
    return "(normtest t k F), where F is a Tarski Formula, returns\
a Tarski Formula resulting from applying a normalizer of type t \
with normalization level k to F.  Currently 'raw' is the only \
supported type.";
  }
  string usage() { return "(normtest <sym> <num> <tarski formula>)"; }
  string name() { return "normtest"; }
};


class CommSuggestVarOrder : public EICommand
{
public:
  CommSuggestVarOrder(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  { 
    try {
      TFormRef F = args[0]->tar()->val;
      vector<VarSet> V = getBrownVariableOrder(F);
      LisRef res = new LisObj();
      for(int i = 0; i < V.size(); i++)
	res->push_back(new SymObj(getPolyManagerPtr()->getName(V[i])));
      return res;
    }
    catch(TarskiException &e) { return new ErrObj(e.what()); }    
  }
  string testArgs(vector<SRef> &args)
  {
    return require(args,_tar);
  }
  string doc() 
  {
    return "(suggest-var-order F), where F is a Tarski formula, \
suggests a variable order to use in CAD construction for formula F. \
The order follows the 'Brown' hueristic.";
  }
  string usage() { return "(suggest-var-order <tarski formula>)"; }
  string name() { return "suggest-var-order"; }
};

class CommCollectFactors : public EICommand
{
public:
  CommCollectFactors(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  { 
    try {
      TFormRef F = args[0]->tar()->val;
      set<IntPolyRef> polys = collectFactors(F);
      set<IntPolyRef,PolyManager::PolyLT> polysord;
      for(set<IntPolyRef>::iterator itr = polys.begin(); itr != polys.end(); ++itr)
	polysord.insert(*itr);
      LisRef res = new LisObj();
      for(set<IntPolyRef,PolyManager::PolyLT>::iterator itr = polysord.begin(); itr != polysord.end(); ++itr)
	res->push_back(new AlgObj(*itr,*getPolyManagerPtr()));
      return res;
    }
    catch(TarskiException &e) { return new ErrObj(e.what()); }    
  }
  string testArgs(vector<SRef> &args)
  {
    return require(args,_tar);
  }
  string doc() 
  {
    return "TO APPEAR";
  }
  string usage() { return "(collect-factors <tarski formula>)"; }
  string name() { return "collect-factors"; }
};


class CommRenameVariables : public EICommand
{
private:
  bool distinctSymbolLists(LisRef L1, LisRef L2)
  {
    int N = L1->length();
    set<string> s1, s2;
    for(int i = 0; i < N; ++i)
    {
      SymRef y1 = L1->get(i)->sym();
      SymRef y2 = L2->get(i)->sym();
      if (y1.is_null() || y2.is_null()) { throw TarskiException("Invalid arguments to rename-variables."); }
      string v1 = y1->val;
      string v2 = y2->val;
      s1.insert(v1);
      s2.insert(v2);
      if (s1.find(v2) != s1.end() || s2.find(v1) != s2.end())
	throw TarskiException("Variables list arguments to rename-variables must be distinct."); 
    }
    if (s1.size() != N || s2.size() != N)
      throw TarskiException("Variables list arguments to rename-variables may not have repititions."); 
    return true;
  }
public:
  CommRenameVariables(NewEInterpreter* ptr) : EICommand(ptr) { }  
  SRef execute(SRef input, vector<SRef> &args) 
  { 
    try {
      LisRef L1 = args[0]->lis();
      LisRef L2 = args[1]->lis();
      if (L1.is_null() || L2.is_null()) { throw TarskiException("Invalid arguments to rename-variables."); }
      int n = L1->length();
      if (L2->length() != n) { throw TarskiException("Invalid arguments to rename-variables."); }
      distinctSymbolLists(L1,L2);
      TFormRef F = args[2]->tar()->val;
      VarSet V = F->getVars();
      VarKeyedMap<VarSet> M;
      for(VarSet::iterator itr = V.begin(); itr != V.end(); ++itr)
	M[*itr] = *itr;
      for(int i = 0; i < L1->length(); ++i)
      {
	VarSet v1 = getPolyManagerPtr()->addVar(L1->get(i)->sym()->val);
	VarSet v2 = getPolyManagerPtr()->addVar(L2->get(i)->sym()->val);
	if ((v2 & V).any()) { throw TarskiException("Error in rename-variables - variables in second argument may not appear in the formula argument."); } 
	M[v1] = v2;
      }
      return new TarObj(F->renameVars(M));
    }
    catch(TarskiException &e) { return new ErrObj(e.what()); }    
  }
  string testArgs(vector<SRef> &args)
  {
    return require(args,_lis,_lis,_tar);
  }
  string doc() 
  {
    return "(rename-variables Vold Vnew F), where Vold is a list of distinct variables, Vnew is a list of the same length as Vold, but of distinct variables that appear in neither Vold nor F; returns a formula in which the ith variable in Vold has been replaced with the ith variable in Vnew.";
  }
  string usage() { return "(rename-variables <oldvars> <newvars> <tarski formula>)"; }
  string name() { return "rename-variables"; }
};


class CommMakePrenex : public EICommand
{
public:
  CommMakePrenex(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  { 
    TFormRef F = args[0]->tar()->val;
    try {
      TFormRef G = makePrenex(F);
      return new TarObj(G);
    }
    catch(TarskiException &e) { return new ErrObj(e.what()); }
  }
  string testArgs(vector<SRef> &args)
  {
    return require(args,_tar);
  }
  string doc() 
  {
    return "(make-prenex F), where F is a Tarski formula, returns an equivalent Tarski formula in prenex form.";
  }
  string usage() { return "(make-prenex <tarski formula>)"; }
  string name() { return "make-prenex"; }
};

void NewEInterpreter::init() 
{
  Interpreter::init();
  // add extended functions
  add(new CommHelp(this));

  add(new CommAnd(this));
  add(new CommOr(this));
  add(new CommNeg(this));
  add(new CommEx(this));
  add(new CommAll(this));
  add(new CommNormalize(this));
  add(new CommNormTest(this));
  add(new CommExclose(this));
  add(new CommDnf(this));
  add(new CommMakePrenex(this));
  add(new CommRenameVariables(this));
  add(new CommQFR(this));
  add(new ClearAssignmentsComm(this));

  add(new CommFactor(this));
  add(new CommUFactor(this));
  add(new CommCollectFactors(this));
  add(new EvalFormAtRat(this));

  add(new CommSub(this));
  add(new CommGetSub(this));
  add(new CommGetArgs(this));

  add(new CommSuggestVarOrder(this));
  add(new CommSuggQepcad(this));
  add(new CommQepcadQE(this));
  add(new CommQepcadSat(this));
  add(new CommSyntax(this));
  
  add(new NewOCBuilderComm(this));
  add(new OCMergeInto(this));
  add(new OCProjFacSummary(this));
  add(new OCProjFacSet(this));
  add(new OCPointInCell(this));
  add(new OCMcCallumProjSummary(this));    
  add(new OCMcCallumProjClosure(this));
  add(new OCErrorPoly(this));
  add(new OCChoosePointCompletion(this));
  add(new OCSplit(this));
  add(new OCPoint(this));
  add(new OCMovePoint(this));

  add(new OCMakeNuCAD1Comm(this));
  add(new OCMakeNuCADConjunctionComm(this));
  add(new OCMakeNuCADDNFComm(this));
  add(new OCSATNuCADConjunctionComm(this));

  add(new SMTLibLoad(this));
  add(new SMTLibStore(this));

  add(new RootsComm(this));
  add(new RanCompComm(this));
  add(new NumToRanComm(this));
  add(new IndexedRootToRanComm(this));
  add(new TestComm(this));

  //Fernando Additions
  //add(new SampleComm(this));
  //add(new TypeComm(this));
  //add(new SacTesterComm(this));
  add(new BBSatComm(this));
  add(new WBTesterComm(this));
  add(new BBWBComm(this));
  add(new BoxSolverComm(this));
  add(new QepSolverComm(this));

  // add extended types
  addType(new RealAlgNumTypeObj(NULL));
  addType(new OCBuilderObj(NULL));
  addType(new OpenNuCADObj(NULL));

}

}//end namespace tarski
