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
#include "../search/qfr-comm.h"
#include "../nucad/TestComm.h"
//#include "../fernando/sample-comm.h"
//#include "../fernando/type-comm.h"
//#include "../fernando/sactester.h"
#include "../bbwb/bbsat-comm.h"
#include "../bbwb/wbsat-comm.h"
#include "../bbwb/bbwb.h"
#include "../smtsolver/box-solver-comm.h"
#include "../smtsolver/qep-solver-comm.h"
#include "../nucad/TestComm.h"
#include "qepcad-inter/qepcad-api.h"  
#include "../search/degreereduce.h"
#include "misc-comms.h"

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
    if (args.size() <= 1)
      return new TarObj(exclose(T));

    vector<string> V;
    LisRef L = args[1]->lis();
    for(int i = 0; i < L->length(); i++)
    {
      SymRef s = L->get(i)->sym();
      if (s.is_null()) {
	return new ErrObj("Error! Second argument of exclose must be a list of symbols!");
      }
      V.push_back(s->getVal());
    }
    return new TarObj(exclose(T,V));
  }
  string testArgs(vector<SRef> &args)
  {
    return require(args,_tar) == "" ? "" : require(args,_tar,_lis);
  }
  string doc() 
  {
    return "(exclose F [L]), where F is a Tarski Formula, returns\
the existential closure of F. If the second argument is used, it should be a list\
of variables to ignore, i.e. not to add to the existential quantifier.";
  }
  string usage() { return "(exclose <tarski formula>) or (exclose <tarski formula> <vars to ignore list>)"; }
  string name() { return "exclose"; }
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
    else if (t->val == "level")
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
      return new ErrObj("Error in normalize: unknown norm type '" + t->val + "'.");
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
(normalize t k F), where F is a Tarski Formula, returns\
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
    else if ((tqb = asa<TQBObj>(T))) {
      LisRef Q = new LisObj();
      for(int i = tqb->numBlocks() - 1; i >= 0; i--)
      {
	LisRef W = new LisObj();
	VarSet V = tqb->blockVars(i);
	for(auto itr = V.begin(); itr != V.end(); ++itr) {
	  W->push_back(new SymObj(getPolyManagerPtr()->getName(*itr)));
	}
	Q->push_back(new LisObj(new SymObj(tqb->blockType(i) == EXIST ? "ex" : "all"),W));
      }
      res->push_back(Q);
      res->push_back(new TarObj(tqb->formulaPart));
    }
    return res;
  }
  string testArgs(vector<SRef> &args)
  {
    return require(args,_tar);
  }
  string doc() 
  {
    return "(getargs F), where F is a Tarski Formula returns a list of the argument subformulas if F is an AND or OR.  Id F is a quantified block, a list of quantifier-varlist pairs is paired with the formula body, e.g. ((ex (x y)) [x y > z^2]). An empty list is returned otherwise.";
  }
  string usage() { return "(getargs <tarski formula>)"; }
  string name() { return "getargs"; }
};

class CommTType : public EICommand
{
public:
  CommTType(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  {
    TFormRef T = args[0]->tar()->val;
    return new NumObj(T->getTFType());
  }
  string testArgs(vector<SRef> &args) { return require(args,_tar); }
  string doc() 
  {
    return "(t-type F), where F is a Tarski Formula, returns a number indexing the formula type: TF_ERROR = 0, TF_CONST = 1, TF_ATOM = 2, TF_EXTATOM = 3, TF_AND = 4, TF_OR = 5, TF_QB = 6.";
  }
  string usage() { return "(t-type <t-tarski formula>)"; }
  string name() { return "t-type"; }
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
	try { 
	  SRef comm = interp->eval(interp->rootFrame,args[0]); 
	  FunRef f = comm->fun();
	  if (!f.is_null() && f->builtin > 0) {
	    sout << builtinFunctionsDoc;
	    writeToFit(sout.str(),cout,getTermWidth());
	    return new SObj();
	  }
	}catch(exception &e) { };
	
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
	  if (f->type() == _err) { return new ErrObj("Name '" + args[0]->sym()->val + "' unknown in help!"); }
	  int n =  f->type() == _fun ? -f->fun()->builtin : -1; 
	  if (f->type() != _fun ||  n < interp->OFFSET || n >= interp->X.size())
	  {
	    return new ErrObj("Argument to help not a builtin or extended function name.");
	  }
	  else
	  {
	    EICommand *p = interp->X[n];
	    sout << p->name() << ", usage: " << p->usage() << "\n" << p->doc() << "\n";
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
    TFormRef F;
    if (args.size() == 3)
    {
      if (args[0]->sym()->val != "limit")
	return new ErrObj("Unknown flag '" + args[0]->sym()->val + " ... 'limit expected!");
      double lim = (double)RNCEIL(args[1]->num()->getVal());
      F = args[2]->tar()->val;
      double est = getDNFNumDisjuncts(F);
      if (est > lim)	
	return new ErrObj("DNF size exceeds limit.");
    }
    else
      F = args[0]->tar()->val;
    return new TarObj(getDNF(F));
  }
  string testArgs(vector<SRef> &args)
  {
    string res = require(args,_tar);
    if (res == "") return res;
    return require(args,_sym,_num,_tar);
  }
  string doc() 
  {
    return "(dnf F), where F is a tarski formula, returns a DNF equivalent to F.";
  }
  string usage() { return "(dnf <tarski formula>) or (dnf 'limit <num> <tarski formula>)"; }
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
    return "(collect-factors F), where F is a Tarski formula, \
returns a list of the distinct irreducible factors of all the \
polynomials appearing in F.";
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

class CommSplitNonStrict : public EICommand
{
public:
  CommSplitNonStrict(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  { 
    TFormRef F = args[0]->tar()->val;
    try {
      TFormRef G = splitNonStrict(F);
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
    return "(split-non-strict F), where F is a Tarski formula, returns an equivalent Tarski formula with all non-strict inequalities split.";
  }
  string usage() { return "(split-non-strict <tarski formula>)"; }
  string name() { return "split-non-strict"; }
};


class CommPolySizeStats : public EICommand
{
  //-- score is defined as follows:
  class Score : public TFPolyFun
  {
    int sscore;
    int sumOfTotalDegree;
    int numConjuncts;
    int factorWeightedNumConjuncts;
  public:
    int getScore() { return sscore; }
    int getSotd() { return sumOfTotalDegree; }
    int getNumConstraints() { return numConjuncts; }
    int getFactorWeightedNumConstraints() { return factorWeightedNumConjuncts; }
    Score() { sscore = 0; sumOfTotalDegree = 0; numConjuncts = 0; }
    virtual void action(TAtomObj* p)
    {
      numConjuncts++;
      int tscore = 0, fcount = 0;
      for(auto itr = p->factorsBegin(); itr != p->factorsEnd(); ++itr) {
	IntPolyRef f = itr->first;
	int a, b, c, t;
	t = f->sizeStats(a,b,c);
	tscore += 10*b + (t == 0 ? 0 : 5) + c;
	sumOfTotalDegree += b;
	++fcount;
      }
      factorWeightedNumConjuncts += (fcount == 0 ? 1 : fcount);
      if (p->getRelop() != NEOP)
	tscore = fcount * tscore;
      sscore = tscore;
    }
    virtual void action(TAndObj* p)
    {
      int tscore = 0;
      for(auto itr = p->begin(); itr != p->end(); ++itr)
      {
	sscore = 0;
	(*this)(*itr);
	tscore += sscore;
      }
      sscore = tscore;
      return;
    }
  };

  static int compscore(IntPolyRef F)
  {
      int a, b, c, t;
      t = F->sizeStats(a,b,c);
      return 10*b + (t == 0 ? 0 : 5) + c;
  }
public:
  CommPolySizeStats(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  {
    if (args[0]->type() == _alg)
    {
      IntPolyRef F = args[0]->alg()->val;
      try {
	int a, b, c, t;
	t = F->sizeStats(a,b,c);
	int score = 10*b + (t == 0 ? 0 : 5) + c;
	LisRef L = new LisObj(new NumObj(a), new NumObj(b), new NumObj(c));
	L->push_back(new NumObj(t));
	L->push_back(new NumObj(score));
	return L;
      }
      catch(TarskiException &e) { return new ErrObj(e.what()); }
    }
    else if (args[0]->type() == _tar)
    {
      TFormRef F = args[0]->tar()->val;
      Score S;
      S(F);
      int res = 0;
      if (args.size() > 1 && args[1]->type() == _sym)
      {
	if (args[1]->sym()->val == "sotd")
	  res = S.getSotd();
	else if (args[1]->sym()->val == "constraints")
	  res = S.getNumConstraints();
	else if (args[1]->sym()->val == "wconstraints")
	  res = S.getFactorWeightedNumConstraints();
	else
	  return new ErrObj(name() + " invalid argument!");
      }
      else
	res = S.getScore();
      return new NumObj(res);
    }
    else
      return new ErrObj(name() + " invalid argument!");
  }
  string testArgs(vector<SRef> &args)
  {
    return "";
  }
  string doc() 
  {
    return "(poly-size-stats F), where F is a Tarski formula or algebraic object, returns returns list (num-terms sum-of-total-degree maximum-coef-bit-length) .";
  }
  string usage() { return "(poly-size-stats <tarski formula>|<alg obj>)"; }
  string name() { return "poly-size-stats"; }
};


class CommLinsubG :  public EICommand
{
public:
  CommLinsubG(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  {
    TFormRef F = args[0]->tar()->getValue();
    TFormRef eq = args[1]->tar()->getValue();
    PolyManager &M = *(eq->getPolyManagerPtr());
    string name = args[2]->sym()->getVal();
    VarSet x = M.getVar(name);
    TFormRef Fp = mkAND(F,eq);
    bool dontAddGuard = false;

    // process other arguments (if they are there)
    for(int i = 3; i < args.size(); i++)
    {
      if (!args[i]->sym().is_null() && args[i]->sym()->getVal() == "no-guard")
	dontAddGuard = true;
      else
	throw TarskiException("Unknown argument '" + args[i]->toStr() + "'.");	
    }
    
    // get poly to sub with
    TAtomRef atom = asa<TAtomObj>(eq);
    if (atom.is_null()) throw TarskiException("Argument eq must be an atom.");
      
    // NEED TO CHECK THAT atom IS OF RIGHT FORM!    
    FactRef eqf = atom->getFactors();
    IntPolyRef A = eqf->factorBegin()->first;    
    Fp = linearSubstGenericCase(M,Fp,A,x,atom,false,dontAddGuard);
    return new TarObj(Fp);
  }
  string testArgs(vector<SRef> &args)
  {
    return "";
  }
  string doc() 
  {
    return "(linsub-g F eq x) - performs on F the generic linear substitution defined by solving eq for x. It is required that eq is an equation in which x appears linearly.  With the 'no-guard option, the constraint that the leading coefficient of x is non-zero will not be added.";
  }
  string usage() { return "(linsub-g <tarski formula> <tarski formula> <var> ['no-guard])"; }
  string name() { return "linsub-g"; }
};

class CommGet2VarFeatures : public EICommand
{
public:
  CommGet2VarFeatures(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  {
    try {
      TFormRef F = args[0]->tar()->getValue();
      string n1 = args[1]->sym()->getVal();
      string n2 = args[2]->sym()->getVal();
      VarSet x = F->getPolyManagerPtr()->getVar(n1);
      if (x.isEmpty())
	throw TarskiException("Error!  " + n1 + " is not a variable in formula!");
      VarSet y = F->getPolyManagerPtr()->getVar(n2);
      if (y.isEmpty())
	throw TarskiException("Error!  " + n2 + " is not a variable in formula!");
      vector<float> Fv = getFeatures2Vars(F,x,y);
      ostringstream sout;
      sout << "[" << Fv[0];
      for(int i = 1; i < Fv.size(); i++)
	sout << "," << Fv[i];
      sout << "]";
      return new StrObj(sout.str());
    }
    catch(TarskiException &e)
    {
      return new ErrObj(e.what());
    }
  }
  string testArgs(vector<SRef> &args)
  {
    return require(args,_tar,_sym,_sym);
  }
  string doc() 
  {
    return "Returns a string representation of feature vector.";
  }
  string usage() { return "(get-2var-features <tarski formula> <sym> <sym>)"; }
  string name() { return "get-2var-features"; }
};

//////////////////////////////////////////////////////////////

class CommChristest : public EICommand
{

  
public:
  CommChristest(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  {
    // Word r = 2, A = LIST4(2,LIST2(0,1),0,LIST2(1,3));
    // Word V = LIST2(LFS("y"),LFS("x"));
    // IntPolyRef p = IntPolyObj::saclibToNonCanonical(r,A,V,*getPolyManagerPtr());
    // return new AlgObj(p,*getPolyManagerPtr());

    if (true) {
      TFormRef T = args[0]->tar()->val;
      GeneralDegreeReduce GDR;
      TFormRef R = GDR.exec(T);
      return new TarObj(R);
    }
    
    SRef res;
    try {
      TFormRef T = args[0]->tar()->val;
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

    /////////////////////////////////////////////////////
    
    PolyManager* pPM = getPolyManagerPtr();
    TFormRef F = args[0]->tar()->val;
    TAndRef G = asa<TAndObj>(F);
    if (!G.is_null()) {
      VarSet V = F->getVars();
      set<IntPolyRef> S;
      getFactors(F,S);

      VarKeyedMap<VarSet> M;
      for(auto pitr = S.begin(); pitr != S.end(); ++pitr)
	for(auto itr = (*pitr)->getVars().begin();  itr != (*pitr)->getVars().end(); ++itr)
	{
	  auto jtr = itr;
	  for(++jtr; jtr != (*pitr)->getVars().end(); ++jtr)
	    M[*itr] = M[*itr] + *jtr;
	}

      cout << "import networkx as nx" << endl;
      cout << "import matplotlib.pyplot as plt" << endl;
      cout << "G = nx.Graph()" << endl;
      cout << "G.add_nodes_from([";
      for(auto itr = V.begin(); itr != V.end(); ++itr)
	cout  << (itr != V.begin() ? "," : "") << "'" << pPM->getName(*itr) << "'";
      cout << "])" << endl;
      cout << "G.add_edges_from([";
      for(auto itr = V.begin(); itr != V.end(); ++itr) {
 	VarSet a = *itr;
	VarSet N = M[a];
	for(auto jtr = N.begin(); jtr != N.end(); ++jtr)
	  cout << (itr != V.begin() || jtr != N.begin() ? "," : "")
	       << "('" << pPM->getName(a) << "','" <<  pPM->getName(*jtr) << "')";
      }
      cout << "])" << endl;
      cout << "nx.draw_networkx(G)" << endl;
      cout << "plt.show()" << endl;

    }
    return new SObj();
  }
  string testArgs(vector<SRef> &args)
  {
    return require(args,_tar) == "" ? "" :  require(args,_tar,_sym);
  }
  string doc() 
  {
    return "(christest F), where F is a Tarski formula does some test stuff.";
  }
  string usage() { return "(christest <tarski formula> ['T | 'E])"; }
  string name() { return "christest"; }
};

class CommSymList : public EICommand
{
public:
  CommSymList(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  {
    LisRef L = new LisObj();

    for(auto itr = interp->rootFrame->begin(); itr != interp->rootFrame->end(); ++itr)
      L->push_back(new SymObj(itr->first));
    
    return L;
  }
  string testArgs(vector<SRef> &args) { return ""; }
  string doc() 
  {
    return "Returns a list of all top-level symbols in the interpreter.";
  }
  string usage() { return "(sym-list)"; }
  string name() { return "sym-list"; }
};





class Plot2D : public EICommand
{

  class Callback : public QepcadAPICallback {
    double x,X,y,Y,e;
    int Id1, Id2;
    string fname;
    bool showset;
  public:
    Callback(int h, int w, double x, double X, double y, double Y, bool showset, const string &fn) {
      Id1 = w; Id2 = h;
      this->x = x;
      this->X = X;
      this->y = y;
      this->Y = Y;
      this->showset = showset;
      e = (X - x)/(w/2);
      fname = fn;
    }
    SRef operator()(QepcadCls &Q) {
      bool c = true, z = true;
      ostream* pout;
      ofstream out;
      ostringstream sout;
      if (fname == "-")
	pout = &sout;
      else {
	out.open(fname);
	pout = &out;
      }
      if (!*pout) {
	throw TarskiException("Could not open file \"" + fname + "\"");
      }
      Q.PLOT2DTOOUTPUTSTREAM(Id1,Id2,x,X,y,Y,e,*pout,c,z,showset);
      if (fname == "-") {
	return new StrObj(sout.str());
      }
      else {	
	return new SObj();
      }
    }
  };
  
public:
  Plot2D(NewEInterpreter* ptr) : EICommand(ptr) { }
  SRef execute(SRef input, vector<SRef> &args) 
  {
    try {
    // options
    bool optord = false;
    bool optshowset = false;
    VarSet v1, v2;
    string nm1, nm2;
    
    // These are guaranteed to be succussfull based on testArgs
    TFormRef F = args[0]->tar()->getValue();
    string str = args[1]->str()->getVal();

    // process remaining arguments
    for(int i = 2; i < args.size(); i++) {
      LisRef arg = args[i]->lis();
      if (arg.is_null() || arg->length() != 2) { throw TarskiException("option arguments must be of form (<name> <value>)"); }
      SymRef name = arg->get(0)->sym();
      if (name.is_null()) { throw TarskiException("the first component of an option argument must be a sym."); }
      if (name->getVal() == "ord") {
	LisRef L = arg->get(1)->lis();
	if (L.is_null() || L->length() != 2) {
	  throw TarskiException("value of 'ord option must be a list of two variables."); }
	SymRef n1 = L->get(0)->sym(), n2 = L->get(1)->sym();
	if (n1.is_null() || n2.is_null()) {
	  throw TarskiException("variables in value of 'ord option must be given as sym objects"); }
	optord = true;
	nm1 = n1->getVal();
	nm2 = n2->getVal();
      }
      else if (name->getVal() == "sset") {
	BooRef L = arg->get(1)->boo();
	optshowset = (L.is_null() || L->val != false); // permissive!
      }
      else {
	throw TarskiException("unknown option \"" + name->getVal() + "\"");
      }
    }
    
    //*** get formula and decide variable order
    // Do basic normalization to get rid of boolean constants, which qepcad doesn't understand.
    RawNormalizer R(defaultNormalizer);
    R(F);
    F = R.getRes();
    PolyManager &PM = *(F->getPolyManagerPtr());    
    VarSet Vall = F->getVars();
    if (Vall.numElements() != 2) { throw TarskiException("Plot2d requries a formula in two variables!"); }
    if (optord) {
      for(auto itr = Vall.begin(); itr != Vall.end(); ++itr) {
	const string& s = PM.getName(*itr);
	if (s == nm1) { v1 = *itr; }
	else if (s == nm2) { v2 = *itr; }
      }
      if (Vall != (v1|v2)) { throw TarskiException("Variables in option 'ord must match the variables in the formula"); }          }
    else {
      int i = 0;
      for(auto itr = Vall.begin(); itr != Vall.end(); ++itr,i++) {
	if (i == 0) { v1 = *itr; } else { v2 = *itr; }
      }	
    }
    ostringstream sout;
    sout << "[]\n(" << PM.getName(v1) << "," << PM.getName(v2) << ")\n";
    sout << "2\n";
    sout << "[";
    PushOutputContext(sout);
    F->write();
    PopOutputContext();
    sout << "].\n";
    if (optshowset) { sout << "full" << endl; }
    sout << "go\ngo\ngo\n";
    
    //*** parse remaining stuff!
    istringstream sin(str);
    int h, w;
    double x,X,y,Y;
    string fname;
    if (!(sin >> h >> w >> x >> X >> y >> Y)) {
      throw TarskiException("format of second argument incorrect (should be e.g. \"300 300 -2 2 -2 2 foo.svg\")");
    }
    if (!(sin >> fname))
      fname = "-";
    if (! (h > 0 && w > 0)) { throw TarskiException("pixel height and width must both be positive"); }
    if (!(x < X)) { throw TarskiException("x-coordinate range must be non-empty"); }
    if (!(y < Y)) { throw TarskiException("y-coordinate range must be non-empty"); }

    //ex: (plot2d [x^4 + y^4 < 1 /\ y > (2 x - 1)^2 x] "600 600 -2 2 -2 2 foo.svg")
    string script = sout.str();
    Callback f(h,w,x,X,y,Y,optshowset,fname);
    SRef res = qepcadAPICall(script,f,true);
    
    return res;
    }
    catch (TarskiException& e) {
      return new ErrObj(string("Error in plot2d: ") + e.what() + "!");
    }
  }
  string testArgs(vector<SRef> &args) {
    return requirepre(args,_tar,_str);
  }
  string doc() 
  {
    return "Given a formula F in two variables, (plot2d F str) produces an svg plot of the formula.  The second argument, 'str', is a string that defines the basic plot parameters: pixelheight, pixelwidth, range of x and range of y, and output file name (or - for string output).  For example, (plot2d [x^4 + y^4 < 1 /\\ y >= (2 x - 1)^2 x] \"400 600 -2 2 -2 2 foo.svg\") produces an output file foo.svg, with height 400 and width 600, showing -2 < x < 2, and -2 < y < 2. Additional optional arguments may follow.  They take the form of (<name> <value>) pairs as described below:\n\
* ord - specify which variable goes to which axis.  '(ord (<var1> <var2>)) puts <var1> on the horizontal axis and <var2> to the vertical axis.  Example: (plot2d [ y^2 = (2 x+1) (3 x - 1) x ] \"400 400 -2 2 -2 2 foo.svg\" '(ord (y x)))\n\
* sset - set to true to show the semi-algebraic set rather than the CAD.  This plot option attempts to depict boundaries and special points in a nice way.  True points are blue, false boundaries and points of special significance are shown in gold.";
  }
  string usage() { return "(plot2d F str [ <opt> ]+)"; }
  string name() { return "plot2d"; }  
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
  add(new CommTType(this));  
  add(new CommNormalize(this));
  add(new CommNormTest(this));
  add(new CommExclose(this));
  add(new CommDnf(this));
  add(new CommMakePrenex(this));
  add(new CommSplitNonStrict(this));
  add(new CommRenameVariables(this));
  add(new CommQFR(this));
  add(new ClearAssignmentsComm(this));
  add(new ClearExpComm(this));

  add(new CommFactor(this));
  add(new CommUFactor(this));
  add(new CommCollectFactors(this));
  add(new EvalFormAtRat(this));
  add(new CommPolySizeStats(this));
  
  add(new CommSub(this));
  add(new CommGetSub(this));
  add(new CommGetArgs(this));

  add(new CommSuggestVarOrder(this));
  add(new CommSuggQepcad(this));
#ifndef __MINGW32__
  add(new CommQepcadQE(this));
  add(new CommQepcadSat(this));
#endif
  add(new CommQepcadAPICall(this));
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
  add(new MakeVarMapTranslate(this));

  add(new RootsComm(this));
  add(new RanCompComm(this));
  add(new NumToRanComm(this));
  add(new IndexedRootToRanComm(this));
  add(new CommGet2VarFeatures(this)); // this is for 1/C Daves research project
  add(new CommLinsubG(this));
  add(new TestComm(this));

  //Fernando Additions
  //add(new SampleComm(this));
  //add(new TypeComm(this));
  //add(new SacTesterComm(this));
  add(new BBSatComm(this));
  add(new WBTesterComm(this));
  add(new BBWBComm(this));
  add(new BoxSolverComm(this));
#ifndef __MINGW32__
  add(new QepSolverComm(this));
#endif
  add(new CommChristest(this));
  add(new CommSymList(this));
  add(new CommGetFreeVars(this)); 
  add(new Plot2D(this));
  add(new CommSolutionDimension(this));
  add(new CommDiscriminant(this));
  add(new CommSubDiscSeq(this));
  add(new CommNullifySys(this));
  add(new CommClear(this));
  
  // add extended types
  addType(new RealAlgNumTypeObj(NULL));
  addType(new OCBuilderObj(NULL));
  addType(new OpenNuCADObj(NULL));
  addType(new VarMapTranslateTypeObj(NULL));
}

}//end namespace tarski
