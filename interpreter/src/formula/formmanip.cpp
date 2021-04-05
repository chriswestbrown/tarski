#include <iostream>
#include <sstream>
#include <queue>
#include "../poly/factor.h"
#include "../poly/sacMod.h"
#include "../algparse/getvars.h"
#include "../algparse/treemanip.h"
#include "../algparse/prettyprint.h"
#include "normalize.h"
#include "formmanip.h"
#include "../realroots/realroots.h"

namespace tarski {
void MapToTForm::operator()(TarskiConst* p) 
{ 
  res = new TConstObj(p->value); 
}
void MapToTForm::operator()(TarskiAtom* p) 
{
  // Get variable names & add them to M
  VarSet S;
  GetVars V;
  p->apply(V);
  for(set<string>::iterator itr = V.vars.begin(); itr != V.vars.end(); ++itr)
    S = S + pM->addVar(*itr);
  
  //    AlgebraicRef A = new BinaryOp('-',p->LHS,p->RHS);    
  
  ostringstream sout, saclibOut;
  PrettyPrint PP(sout,PrettyPrint::qepcad);
  
  AlgebraicRef ts = new BinaryOp('-',p->LHS,p->RHS);
  ts->apply(PP);
  const char termchar = ';';
  sout << termchar; // Saclib requires that terminating character!
  istringstream sin(sout.str());
  PushInputContext(sin);
  IntPolyRef f = new IntPolyObj();
  PushOutputContext(saclibOut);
  int status = f->readRatSetSim(*pM,S);
  PopOutputContext();
  char nextchar = CREAD(); BKSP();
  PopInputContext();  
  if (!status || nextchar != termchar) 
  { 
   throw TarskiException("Could not parse as an rational polynomial!"); 
  }
  
  /*
  //    A->apply(PP);
    p->LHS->apply(PP);
    sout << " - (";
    p->RHS->apply(PP);
    sout << ")";
    sout << "."; // Saclib requires that terminating character!
    istringstream sin(sout.str());
    PushInputContext(sin);
    IntPolyRef f = new IntPolyObj();
    f->readRatSetSim(*pM,S);
    PopInputContext();
    */
  TAtomRef AF = new TAtomObj(*pM);
  AF->F->addMultiple(f,1);
  AF->relop = p->relop;
  res = AF;
}

void MapToTForm::operator()(ExtTarskiAtom* p) 
{
  // Get variable names & add them to M
  VarSet S;
  GetVars V;
  p->apply(V);
  for(set<string>::iterator itr = V.vars.begin(); itr != V.vars.end(); ++itr)
    S = S + pM->addVar(*itr);
  
  ostringstream sout, saclibOut;
  PrettyPrint PP(sout,PrettyPrint::qepcad);
  
  AlgebraicRef ts = p->RHS;
  ts->apply(PP);
  const char termchar = ';';
  sout << termchar; // Saclib requires that terminating character!
  istringstream sin(sout.str());
  PushInputContext(sin);
  IntPolyRef f = new IntPolyObj();
  PushOutputContext(saclibOut);
  int status = f->readRatSetSim(*pM,S);
  PopOutputContext();
  char nextchar = CREAD(); BKSP();
  PopInputContext();  
  if (!status || nextchar != termchar) 
  { 
   throw TarskiException("Could not parse as an rational polynomial!"); 
  }
  
  TExtAtomRef AF = new TExtAtomObj(*pM);
  AF->F->addMultiple(f,1);
  AF->relop = p->relop;
  AF->var = pM->getVar(p->getVar());
  AF->rootIndex = p->getRootIndex();
  res = AF;
}


void MapToTForm::operator()(NotOp* p)
{
  p->arg->apply(*this);
  res = res->negate();
}

void MapToTForm::operator()(BinaryLogOp* p) 
{ 
  p->LHS->apply(*this);
  TFormRef a1 = res;
  p->RHS->apply(*this);
  TFormRef a2 = res;
  
  if (p->logop == ANDOP)
  {
    TAndRef A = new TAndObj;
    A->AND(a1);
    A->AND(a2);
    if (A->size() > 1) res = A;
      else if (A->size() == 1) res = *(A->conjuncts.begin());
      else res = new TConstObj(TRUE);
  }
  else if (p->logop == OROP || p->logop == RIGHTOP || p->logop == LEFTOP)
  {
    if (p->logop == RIGHTOP)
      a1 = a1->negate();
    else if (p->logop == LEFTOP)
      a2 = a2->negate();
    TOrRef A = new TOrObj;
    A->OR(a1);
    A->OR(a2);
    if (A->size() > 1) res = A;
    else if (A->size() == 1) res = *(A->disjuncts.begin());
    else res = new TConstObj(FALSE);
  }
}

void MapToTForm::operator()(QuantifierBlock* p) 
{ 
  p->arg->apply(*this);
  TFormRef a = res;
  VarSet S;
  for(unsigned int i = 0; i < p->vars.size(); ++i)
  {
    Var *tp = asa<Var>(p->vars[i]);      
    S = S + pM->addVar(tp->value); // important to use "addVar" so that if tp->value has 
    // not already appeared as a variable somewhere, it's
    // added to the PolyManager.
  }
  TQBRef qb = asa<TQBObj>(a);
  if (!qb.is_null()) { qb->add(S,p->kind); res = qb; }
  else { res = new TQBObj(S,p->kind,a,pM); }
}

IntPolyRef parseTreeToPoly(TarskiRef parseTree, PolyManager &M)
{
  // Get variable names & add them to M
  VarSet S;
  GetVars V;
  parseTree->apply(V);
  for(set<string>::iterator itr = V.vars.begin(); itr != V.vars.end(); ++itr)
    S = S + M.addVar(*itr);

  ostringstream sout;
  PrettyPrint PP(sout,PrettyPrint::qepcad);
  parseTree->apply(PP);
  const char termchar = ';';
  sout << termchar; // Saclib requires that terminating character!
  //  cerr << ">>> " << sout.str() << endl;
  istringstream sin(sout.str());
  PushInputContext(sin);
  IntPolyRef f = new IntPolyObj();
  int status = f->read(M,S);
  //  cerr << ">>> NEXT CREAD = '" << (char)CREAD() << "'" << endl; BKSP();
  char nextchar = CREAD(); BKSP();
  PopInputContext();  
  if (!status || nextchar != termchar) 
  { 
    throw TarskiException("Could not parse as an integer polynomial!"); 
  }
  return f;
}

TFormRef parseTreeToTForm(TarskiRef parseTree, PolyManager &M)
{
  MapToTForm MTTF(M);
  parseTree->apply(MTTF);
  return MTTF.res;
}

// determine whether all inequalities are strict (in total and per variable)
class AnalyzeStrictness : public TFPolyFun
{
private:
  VarSet appearsInNonStrict;

public:
  AnalyzeStrictness() : appearsInNonStrict(0) { }
  VarSet getResult() { return appearsInNonStrict; }
  void action(TConstObj* p) {  }
  void action(TAtomObj* p) {
    if (relopIsNonStrict(p->getRelop()))
      appearsInNonStrict = appearsInNonStrict + p->getVars();
  }
  void action(TExtAtomObj* p) {
    if (relopIsNonStrict(p->getRelop()))
      appearsInNonStrict = appearsInNonStrict + p->getVars();
  }
  void action(TAndObj* p) 
  { 
    for(TAndObj::conjunct_iterator itr = p->conjuncts.begin(); itr != p->conjuncts.end(); ++itr)
      actOn(*itr);
  }
  void action(TOrObj* p)
  { 
    for(TOrObj::disjunct_iterator itr = p->disjuncts.begin(); itr != p->disjuncts.end(); ++itr)
      actOn(*itr);
  }
  void action(TQBObj* p) { actOn(p->formulaPart); }
};

VarSet appearsInNonStrict(TFormRef F)
{
  AnalyzeStrictness  AS;
  AS.actOn(F);
  return AS.getResult(); 
};


class IsPrenex : public TFPolyFun
{
private:
  int res;
  bool failflag;
public:
  IsPrenex() { res = 0; failflag = false; }
  bool getFailFlag() const { return failflag; }
  void action(TConstObj* p) { }
  void action(TAtomObj* p) { }
  void action(TExtAtomObj* p) { }
  void action(TAndObj* p) 
  {
    int cnt=0;
    for(set<TFormRef>::iterator itr = p->conjuncts.begin(); itr != p->conjuncts.end(); ++itr)
    { res = 0; actOn(*itr); cnt += res; }
    if (cnt > 0 && p->conjuncts.size() > 1) failflag = true;
    res = cnt;
  }
  void action(TOrObj* p)
  { 
    int cnt=0;
    for(set<TFormRef>::iterator itr = p->disjuncts.begin(); res==0 && itr != p->disjuncts.end(); ++itr)
    { res = 0; actOn(*itr); cnt += res; }
    if (cnt > 0 && p->disjuncts.size() > 1) failflag = true;
    res = cnt;
  }
  void action(TQBObj* p) { res = 0; actOn(p->formulaPart); res = res + 1; }
};

bool isPrenex(TFormRef F)
{
  IsPrenex IP;
  IP.actOn(F);
  return !IP.getFailFlag();
};

class AndsAndAtoms : public TFPolyFun
{
public:
  int res;

  AndsAndAtoms() { res = 1; }
  void action(TAndObj* p) 
  {
    for(set<TFormRef>::iterator itr = p->conjuncts.begin(); res == 1 && itr != p->conjuncts.end(); ++itr)
    { actOn(*itr); }
  }
  void action(TOrObj* p) { res = 0; }
  void action(TQBObj* p) { res = 0;  }  
  void action(TConstObj* p) {  }
  void action(TAtomObj* p) {  }
  void action(TExtAtomObj* p) {  }
};

bool isAndAndAtoms(TFormRef F) { AndsAndAtoms A; A(F); return A.res == 1; }

bool isQuantifiedConjunctionOfAtoms(TFormRef F)
{
  TQBRef p = asa<TQBObj>(F);
  if (!p.is_null() && p->outermostBlockType == EXIST) { AndsAndAtoms A; A(p->formulaPart); return A.res == 1; }
  return false;
};

bool isConjunctionOfAtoms(TFormRef F)
{
  bool res = true;
  TAndRef A = asa<TAndObj>(F);
  if (A.is_null()) return false;
  for(TAndObj::conjunct_iterator itr = A->begin(); res && itr != A->end(); ++itr)
    res = asa<TAtomObj>(*itr) != NULL || asa<TExtAtomObj>(*itr) != NULL;
  return res;
}

//-- GetDNFNumDisjuncts
double getDNFNumDisjuncts(TFormRef F)
{
  double n = -1;
  switch(F->getTFType())
  {
  case TF_CONST:
  case TF_ATOM:
  case TF_EXTATOM:
    n = 1.0; break;
  case TF_AND: {
    TAndRef C = asa<TAndObj>(F);
    n = 1.0;
    for(auto itr = C->begin(); itr != C->end(); ++itr)
      n *= getDNFNumDisjuncts(*itr);
  } break;
  case TF_OR: {
    TOrRef C = asa<TOrObj>(F);
    n = 0.0;
    for(auto itr = C->begin(); itr != C->end(); ++itr)
      n += getDNFNumDisjuncts(*itr);
  } break;
  case TF_QB: {
    TQBRef Q = asa<TQBObj>(F);
    n = getDNFNumDisjuncts(Q);
  } break;
  default:
    throw TarskiException("Unknown TFormObj type!");
  }
  return n;
}


//-- DNF!
class MakeDNF : public TFPolyFun
{
public:
  TFormRef res;
  void action(TConstObj* p);
  void action(TAtomObj* p);
  void action(TAndObj* p);
  void action(TOrObj* p);
  void action(TQBObj* p);
};

TFormRef getDNF(TFormRef F)
{
  MakeDNF X;
  X(F);
  return X.res;
}
void MakeDNF::action(TConstObj* p) { res = p; }
void MakeDNF::action(TAtomObj* p) { res = p; }

TOrRef distribute(TOrRef D1, TOrRef D2)
{
  TOrRef out = new TOrObj;

  for(set<TFormRef>::iterator itr1 = D1->disjuncts.begin(); itr1 != D1->disjuncts.end(); ++itr1)
    for(set<TFormRef>::iterator itr2 = D2->disjuncts.begin(); itr2 != D2->disjuncts.end(); ++itr2)
    {
      TAndRef p = new TAndObj;
      p->AND(*itr1);
      p->AND(*itr2);
      out->OR(p);
    }
  return out;
}

void MakeDNF::action(TAndObj* p)// let's assume size > 1
{
  TAndRef C = new TAndObj;
  TAndRef D = new TAndObj;
  for(set<TFormRef>::iterator itr = p->conjuncts.begin(); itr != p->conjuncts.end(); ++itr)
  {
    TFormRef tmp = getDNF(*itr);
    if (asa<TOrObj>(tmp) == NULL) C->AND(tmp); else D->AND(tmp);
  }
  if (D->size() == 0)
    res = C;
  else
  {
    TOrRef X = new TOrObj;
    if (C->size() > 0) X->OR(C); else X->OR(new TConstObj(TRUE));
    for(set<TFormRef>::iterator itr = D->conjuncts.begin(); itr != D->conjuncts.end(); ++itr)
    {
      TOrRef dummy = asa<TOrObj>(*itr);
      if (dummy.is_null()) { dummy = new TOrObj; dummy->OR(*itr); }      
      X = distribute(dummy,X);
    }
    res = X;
  }
}
void MakeDNF::action(TOrObj* p)
{
  TOrRef D = new TOrObj;
  for(set<TFormRef>::iterator itr = p->disjuncts.begin(); itr != p->disjuncts.end(); ++itr)
    D->OR(getDNF(*itr));
  res = D;
}
void MakeDNF::action(TQBObj* p)
{
  TQBRef tmp = new TQBObj(p->getPolyManagerPtr());
  tmp->outermostBlockType = p->outermostBlockType;
  tmp->blocks = p->blocks;
  tmp->formulaPart = getDNF(p->formulaPart);
  res = tmp;
}

class GetFreeVars : public TFPolyFun
{
public:
  VarSet res;
  void action(TConstObj* p) { res = p->getVars(); }
  void action(TAtomObj* p) { res = p->getVars(); }
  void action(TAndObj* p)
  { 
    VarSet V;
    for(TAndObj::conjunct_iterator itr = p->conjuncts.begin(); itr != p->conjuncts.end(); ++itr)
    {
      (*itr)->apply(*this);
      V = V + res;
    }
    res = V; 
  }
  void action(TOrObj* p) 
  { 
    VarSet V;
    for(TOrObj::disjunct_iterator itr = p->disjuncts.begin(); itr != p->disjuncts.end(); ++itr)
    {
      (*itr)->apply(*this);
      V = V + res;
    }
    res = V; 
  }
  void action(TQBObj* p)
  {
    p->formulaPart->apply(*this);
    VarSet V = res;
    VarSet BV;
    for(unsigned int i = 0; i < p->blocks.size(); ++i) BV |= p->blocks[i];
    res = V ^ (V & BV);
  }
};

VarSet getFreeVars(TFormRef F)
{
  GetFreeVars X;
  X(F);
  return X.res;
}

// void GetFreeVars::action(TQBObj* p) 
// { 
//   VarSet BV;
//   for(int i = 0; i < p->blocks.size(); ++i) BV |= p->blocks[i];
//   VarSet V = getFreeVars(p->formulaPart);
//   res = V ^ (V & BV);
// }



//--- Implmentation of evalFormulaAtRational
class EvalFormulaAtRational : public TFPolyFun
{
public:
  TFormRef res;
  VarSet X;
  GCWord val;
  EvalFormulaAtRational(VarSet _x, Word _val) : X(_x), val(_val) { } 
  virtual void action(TConstObj* p) { res =  new TConstObj(p->value); }
  virtual void action(TAtomObj* p) 
  {
    FactRef F = new FactObj(*(p->getPolyManagerPtr()),p->F->content); 
    for(map<IntPolyRef,int>::iterator itr = p->factorsBegin(); itr != p->factorsEnd(); ++itr)
    {
      // SWRITE("\nEvaluating factor:\n");
      // itr->first->write(*(p->getPolyManagerPtr())); SWRITE("\n");
      IntPolyRef A = itr->first->evalAtRationalMakePrim(X,val);
      // A->write(*(p->getPolyManagerPtr())); SWRITE("\n");
      F->addMultiple(A,itr->second);	
    }

    TAtomRef A = new TAtomObj(F,p->relop);

    // constant LHS case
    int relop = (ICOMP(A->F->content,0) < 0) ? reverseRelop(A->relop) : A->relop;
    if (A->F->isZero())
    { res = new  TConstObj(relop == EQOP || relop == LEOP || relop == GEOP); return; }
    if (A->F->isConstant())
    {
      res = new TConstObj(relop == NEOP || relop == GTOP || relop == GEOP); return; 
    }
    res = A;
  }

  // virtual void action(TExtAtomObj* p) 
  // {
  //   VarSet V_F = F->getVars(), z = getLHSVar();

  //   FactRef F = new FactObj(*(p->getPolyManagerPtr()),p->F->content); 
  //   for(map<IntPolyRef,int>::iterator itr = p->factorsBegin(); itr != p->factorsEnd(); ++itr)
  //   {
  //     IntPolyRef A = itr->first->evalAtRationalMakePrim(X,val);
  //     F->addMultiple(A,itr->second);	
  //   }

  //   TAtomRef A = new TAtomObj(F,p->relop);

  //   // constant LHS case
  //   int relop = (ICOMP(A->F->content,0) < 0) ? reverseRelop(A->relop) : A->relop;
  //   if (A->F->isZero())
  //   { res = new  TConstObj(relop == EQOP || relop == LEOP || relop == GEOP); return; }
  //   if (A->F->isConstant())
  //   {
  //     res = new TConstObj(relop == NEOP || relop == GTOP || relop == GEOP); return; 
  //   }
  //   res = A;
  // }

  virtual void action(TAndObj* p) {
    TAndRef A = new TAndObj();
    for(TAndObj::conjunct_iterator itr = p->conjuncts.begin(); itr != p->conjuncts.end(); ++itr) {
      this->actOn(*itr);
      int cv = constValue(res);
      if (cv == 0) { res = new TConstObj(false); return; }
      if (cv != 1) { A->AND(res); }
    }
    res = A;
 }
  virtual void action(TOrObj* p) {
    TOrRef A = new TOrObj();
    for(TOrObj::disjunct_iterator itr = p->disjuncts.begin(); itr != p->disjuncts.end(); ++itr) {
      this->actOn(*itr);
      int cv = constValue(res);
      if (cv == 1) { res = new TConstObj(true); return; }
      if (cv != 0) { A->OR(res); }
    }
    res = A;
 }
  virtual void action(TQBObj* p) 
  {
    if ((p->getFullQuantifiedVarSet() & X).any())
      res = p;
    else
    {
      TQBRef t = new TQBObj(p);
      this->actOn(p->getFormulaPart());
      t->formulaPart = res;
      res = t;
    }
  }
};

TFormRef evalFormulaAtRational(VarSet X, GCWord R, TFormRef F)
{
  EvalFormulaAtRational E(X,R);
  E(F);
  return E.res;
}



//--- Implmentation of evalFormulaAtRational point rather than value
class EvalFormulaAtRationalPoint : public TFPolyFun
{
public:
  TFormRef res;
  VarKeyedMap<GCWord>* pValue;

  EvalFormulaAtRationalPoint(VarKeyedMap<GCWord>& value) { pValue = &value; }
  virtual void action(TConstObj* p) { res =  new TConstObj(p->value); }
  virtual void action(TAtomObj* p) 
  {
    FactRef F = new FactObj(*(p->getPolyManagerPtr()),p->F->content); 
    for(map<IntPolyRef,int>::iterator itr = p->factorsBegin(); itr != p->factorsEnd(); ++itr)
    {
      //SWRITE("\nEvaluating factor:\n");
      //itr->first->write(*(p->getPolyManagerPtr())); SWRITE("\n");
      GCWord content;
      IntPolyRef A = itr->first->evalAtRationalPointMakePrim(*pValue,content);
      //A->write(*(p->getPolyManagerPtr())); SWRITE(" content = "); RNWRITE(content); SWRITE("\n");
      F->addMultiple(A,itr->second);	
      if (RNSIGN(content) < 0 && itr->second % 2 == 1) { F->negateContent(); }
    }

    int relop = p->relop;
    TAtomRef A = new TAtomObj(F,relop);

    // constant LHS case
    if (A->F->isZero())
    { res = new  TConstObj(relop == EQOP || relop == LEOP || relop == GEOP); return; }
    if (A->F->isConstant())
    {
      int s = F->signOfContent();
      int truth = signSatSigma(s,relop);
      res = new TConstObj(truth); return; 
    }
    res = A;
  }

  /*
    The semantics of evaluation for indexed root expressions is a bit tricky.
    I'm not going to try to handle the broadest interpretation of them - at least for now.
    Instead, I'm going for essentially the most limited version, i.e. the evaluation
    has to include values for all variables appearing in the indexed root expression.
   */
  virtual void action(TExtAtomObj* p) 
  {
    VarKeyedMap<GCWord>& values = *pValue;
    VarSet z = p->getLHSVar(), X = p->getVars();
    Word alpha_z = values[z];
    if (alpha_z == NIL)
      throw TarskiException("Error in EvalFormulaAtRationalPoint! partial eval not yet supported!");
    RealAlgNumRef alpha_z_ran = rationalToRealAlgNum(alpha_z);
    
    // Verify that all variables appearing have values
    for(VarSet::iterator itr = X.begin(); itr != X.end(); ++itr)
      if (values[*itr] == NIL)
	throw TarskiException("Error in EvalFormulaAtRationalPoint! missing value for " + 
			      p->getPolyManagerPtr()->getName(*itr)
			      + "!");
   
    // For each factor, count the number of roots less than and greater than alpha_z
    // NOTE: this could be done a lot faster than I'm doing it!
    values[z] = NIL;
    int n_less = 0, n_greater = 0, n_equal = 0;
    for(map<IntPolyRef,int>::iterator itr = p->factorsBegin(); itr != p->factorsEnd(); ++itr)
    {
      GCWord content;
      IntPolyRef A = itr->first->evalAtRationalPointMakePrim(values,content);
      vector<RealRootIUPRef> roots =  RealRootIsolateRobust(A);
      for(unsigned int i = 0; i < roots.size(); i++)
      {
	int c = roots[i]->compareToRobust(alpha_z_ran);
	if (c == -1) n_less++;
	else if (c == 0) n_equal++;
	else n_greater++;
      }
    }
    values[z] = alpha_z;

    // get relop of alpha_z w.r.t. _root_index
    int truth = p->detTruth(n_less, n_equal, n_greater);
    res = new TConstObj(truth);
  }

  virtual void action(TAndObj* p) {
    TAndRef A = new TAndObj();
    for(TAndObj::conjunct_iterator itr = p->conjuncts.begin(); itr != p->conjuncts.end(); ++itr) {
      this->actOn(*itr);
      int cv = constValue(res);
      if (cv == 0) { res = new TConstObj(false); return; }
      if (cv != 1) { A->AND(res); }
    }
    res = A;
 }
  virtual void action(TOrObj* p) {
    TOrRef A = new TOrObj();
    for(TOrObj::disjunct_iterator itr = p->disjuncts.begin(); itr != p->disjuncts.end(); ++itr) {
      this->actOn(*itr);
      int cv = constValue(res);
      if (cv == 1) { res = new TConstObj(true); return; }
      if (cv != 0) { A->OR(res); }
    }
    res = A;
  }
  virtual void action(TQBObj* p) 
  {
    // remove from values any variable assignment for a variables covered by a quantified variable
    VarKeyedMap<GCWord>& values = *pValue;
    VarKeyedMap<GCWord> values_covered(NIL);
    VarSet X = p->getFullQuantifiedVarSet(), Xcovered = 0;
    for(VarSet::iterator itr = X.begin(); itr != X.end(); ++itr)
      if (values[*itr] != NIL)
      {
	values_covered[*itr] = values[*itr];
	values[*itr] = NIL;
	Xcovered = Xcovered + *itr;
      }

    // evaluate with remaining assignment and create new TQBObj
    this->actOn(p->getFormulaPart());    
    TQBRef t = new TQBObj(p);
    this->actOn(p->getFormulaPart());
    t->formulaPart = res;
    res = t;
    
    // restore values
    for(VarSet::iterator itr = Xcovered.begin(); itr != Xcovered.end(); ++itr)
      values[*itr] = values_covered[*itr];
  }
};

TFormRef evalFormulaAtRationalPoint(VarKeyedMap<GCWord> &value, TFormRef F)
{
  EvalFormulaAtRationalPoint E(value);
  E(F);
  return E.res;
}


/*************************************************************
 * M A K E P R E N E X
 *************************************************************/
class MakePrenexHelper : public TFPolyFun
{
private:
  TQBRef res;
  PolyManager* pM;
  VarKeyedMap<int> nextSubscript;

  VarSet newVar(VarSet v)
  {
    int k = nextSubscript[v]++;
    ostringstream sout;
    sout << pM->getName(v) << 'x' << k;
    return pM->addVar(sout.str());
  }

public:
  MakePrenexHelper(PolyManager* pM) { this->pM = pM; }

  // this is meant to be used outside of this class.  if the 
  // quantifier blocks are empty, they are stripped away.
  TFormRef getResult()
  {
    if (res->numBlocks() == 0)
      return res->getFormulaPart();
    else
      return res;
  }
  void action(TConstObj* p) 
  {
    TQBRef g = new TQBObj(pM);
    g->formulaPart = p;
    res = g;
  }
  void action(TAtomObj* p) 
  {
    TQBRef g = new TQBObj(pM);
    g->formulaPart = p;
    res = g;    
  }
  void action(TAndObj* p) 
  {
    vector<TQBRef> V;
    for(TAndObj::conjunct_iterator itr = p->conjuncts.begin(); itr != p->conjuncts.end(); ++itr)
    {
      (*this)(*itr);
      V.push_back(res);
    }
    
    for(unsigned int i = 1; i < V.size(); i++)
    {
      VarSet qv0 = V[0]->getFullQuantifiedVarSet();
      VarSet qv1 = V[i]->getFullQuantifiedVarSet();
      VarSet fv0 = V[0]->getVars() - qv0;
      VarSet fv1 = V[i]->getVars() - qv1;
      VarSet rnv1 = qv1 & (qv0 + fv0); // variables in V[1] that need renaming
      VarSet rnv0 = qv0 & fv1; // variables in V[0] that need renaming
      VarSet vold = qv0 + fv0 + qv1 + fv1; 
    
      VarKeyedMap<VarSet> map1;
      for(VarSet::iterator itr = vold.begin(); itr != vold.end(); ++itr)
	map1[*itr] = *itr;
      for(VarSet::iterator itr = rnv1.begin(); itr != rnv1.end(); ++itr)
	map1[*itr] = newVar(*itr); //pM->addVar(string("x") + char('a' + (++i)) );

      VarKeyedMap<VarSet> map0;
      for(VarSet::iterator itr = vold.begin(); itr != vold.end(); ++itr)
	map0[*itr] = *itr;
      for(VarSet::iterator itr = rnv0.begin(); itr != rnv0.end(); ++itr)
	map0[*itr] = newVar(*itr); //pM->addVar(string("x") + char('a' + (++i)) );
      
      TQBRef V0 = dynamic_cast<TQBObj*>(&*(V[0]))->renameQuantifiedVars(map0);
      TQBRef V1 = dynamic_cast<TQBObj*>(&*(V[i]))->renameQuantifiedVars(map1);
      TAndRef C = new TAndObj();
      C->AND(V0->getFormulaPart());
      C->AND(V1->getFormulaPart());
      V0->formulaPart = C;
      for(int i = 0; i < V1->numBlocks(); i++)
	V0->add(V1->blockVars(i), V1->blockType(i));    
      V[0] = V0;
    }
    res = V[0];
  }
  void action(TOrObj* p)
  {
    vector<TQBRef> V;
    for(TOrObj::disjunct_iterator itr = p->disjuncts.begin(); itr != p->disjuncts.end(); ++itr)
    {
      (*this)(*itr);
      V.push_back(res);
    }
    
    for(unsigned int i = 1; i < V.size(); i++)
    {
      VarSet qv0 = V[0]->getFullQuantifiedVarSet();
      VarSet qv1 = V[i]->getFullQuantifiedVarSet();
      VarSet fv0 = V[0]->getVars() - qv0;
      VarSet fv1 = V[i]->getVars() - qv1;
      VarSet rnv1 = qv1 & (qv0 + fv0); // variables in V[1] that need renaming
      VarSet rnv0 = qv0 & fv1; // variables in V[0] that need renaming
      VarSet vold = qv0 + fv0 + qv1 + fv1; 
    
      VarKeyedMap<VarSet> map1;
      for(VarSet::iterator itr = vold.begin(); itr != vold.end(); ++itr)
	map1[*itr] = *itr;
      for(VarSet::iterator itr = rnv1.begin(); itr != rnv1.end(); ++itr)
	map1[*itr] = newVar(*itr); //pM->addVar(string("x") + char('a' + (++i)) );

      VarKeyedMap<VarSet> map0;
      for(VarSet::iterator itr = vold.begin(); itr != vold.end(); ++itr)
	map0[*itr] = *itr;
      for(VarSet::iterator itr = rnv0.begin(); itr != rnv0.end(); ++itr)
	map0[*itr] = newVar(*itr); //pM->addVar(string("x") + char('a' + (++i)) );
      
      TQBRef V0 = dynamic_cast<TQBObj*>(&*(V[0]))->renameQuantifiedVars(map0);
      TQBRef V1 = dynamic_cast<TQBObj*>(&*(V[i]))->renameQuantifiedVars(map1);
      TOrRef C = new TOrObj();
      C->OR(V0->getFormulaPart());
      C->OR(V1->getFormulaPart());
      V0->formulaPart = C;
      for(int i = 0; i < V1->numBlocks(); i++)
	V0->add(V1->blockVars(i), V1->blockType(i));    
      V[0] = V0;
    }
    res = V[0];
  }

  void action(TQBObj* p) 
  {
    (*this)(p->getFormulaPart());
    VarSet Vinner = res->getFullQuantifiedVarSet();
    for(int i = 0; i < p->numBlocks(); i++)
      res->add(p->blockVars(i) - Vinner, p->blockType(i));
  }
};

TFormRef makePrenex(TFormRef F)
{
  MakePrenexHelper H(F->getPolyManagerPtr());
  H(F);
  return H.getResult();
}

  TFormRef exclose(TFormRef T)
  {
    VarSet FVars = getFreeVars(T);
    TFormRef res;
    if (FVars.none()) 
      res = T;
    else {
      TQBRef Tp = dynamic_cast<TQBObj*>(&*T);
      if (Tp.is_null())
	res = new TQBObj(FVars, EXIST, T, T->getPolyManagerPtr());
      else
      {
	Tp->add(FVars,EXIST);
	res = Tp;
      }
    }
    return res;
  }

TFormRef exclose(TFormRef T, const vector<string> &holdout)
{
  VarSet HVars = 0;
  PolyManager *p = T->getPolyManagerPtr();
  for(int i = 0; i < holdout.size(); i++)
    HVars = HVars + p->getVar(holdout[i]);
  VarSet FVars = getFreeVars(T);
  VarSet EVars = FVars - HVars;
  TFormRef res;
  if (EVars.none()) 
    res = T;
  else {
    TQBRef Tp = dynamic_cast<TQBObj*>(&*T);
    if (Tp.is_null())
      res = new TQBObj(EVars, EXIST, T, T->getPolyManagerPtr());
    else
    {
      Tp->add(EVars,EXIST);
      res = Tp;
    }
  }
  return res;
}


TFormRef splitAtom(TAtomRef A)
{
  FactRef f = A->getFactors();
  int relop = A->getRelop();
  switch(relop)
  {
  case LEOP: case GEOP:
    return mkOR(makeAtom(f,EQOP) , makeAtom(f,relop & NEOP));
    break;
  default:
    return A;
    break;
  }
}

 TFormRef splitNonStrict(TFormRef F)
 {
   TFormRef F1 = level1(F);
   switch (F1->getTFType()){
   case TF_CONST: return F1; break;
   case TF_ATOM:
     return splitAtom(F1);
     break;
   case TF_AND: {     
     TAndRef res = new TAndObj(), C = dynamic_cast<TAndObj*>(&*F1);
     for(auto itr = C->begin(); itr != C->end(); ++itr) {
       TFormRef tmp = splitAtom(*itr);
       res->AND(tmp); }
     return res;
   } break;
   default: throw TarskiException("splitNonStrict requires constants, atoms or conjunctions.");
    }
   return NULL;
 }

class FactorCollector : public TFPolyFun
{
  PolyManager *pM;
  std::set<IntPolyRef>* Wp; 
public:
  FactorCollector(PolyManager &M, set<IntPolyRef> &W) { pM = &M; Wp = &W; }
  void action(TAtomObj* p) {
    FactRef f = p->getFactors();
    for(auto itr = f->factorBegin(); itr != f->factorEnd(); ++itr)
      Wp->insert(itr->first);
  }
  void action(TConstObj* p) { ; }
  void action(TAndObj* p) {
    for(auto itr = p->begin(); itr != p->end(); ++itr)
      actOn(*itr);
  }
};

void getFactors(TFormRef F, std::set<IntPolyRef> &W)
{
  FactorCollector FC(*F->getPolyManagerPtr(),W);
  FC.actOn(F);
  return;
}

}//end namespace tarski
