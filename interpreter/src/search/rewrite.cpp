#include "../formula/linearSubs.h"
#include "../formula/formmanip.h"
#include "../formula/writeForQE.h"
#include "../formula/formula.h"
#include "rewrite.h"
#include "qfr.h"
#include "degreereduce.h"

using namespace std;

namespace tarski {
  
TFormRef BasicRewrite::refine(QAndRef target, TFQueueRef Q)
{
  if (target->expanded)
    return NULL; // I thought things were set up so this wouldn't happen!
  target->expanded = true;

  // Clever little gambit to encase F in an AND if it happens to be an atom
  TAndRef Fs = new TAndObj();
  Fs->AND(target->F);
  FindEquations E;
  E(Fs);
  set<TAtomRef>::iterator beg_eq = E.res.begin(), end_eq = E.res.end();

  /*** Basic linear subs and linSubsXpowk that we always do ***/
  for(auto i_eq = beg_eq; i_eq != end_eq; ++i_eq)
  {
    if ((*i_eq)->F->numFactors() == 1)
    {
      TFormRef res = linSubs(target,*i_eq,Q,Fs);
      if (asa<TConstObj>(res)) return res;
      res = linSubsXpowK(target,*i_eq,Q,Fs);
      if (asa<TConstObj>(res)) return res;
    }
    else if (Fs->size() > 1)
    {
      TFormRef res = splitOnAtom(target,*i_eq,Q);
      if (asa<TConstObj>(res)) return res;      
    }
  }

  /*** General degree reduce. ***/
  if(true) {
    GeneralDegreeReduce GDR;
    TFormRef res1 = GDR.exec(target);
    if (!res1.identical(target->F)) {
      if (constValue(res1) == TRUE) return res1;
      ParentageRef PT1= GDR.makeParentage(target);
      vector<QAndRef> TV = { new QAndObj(res1,target->QVars,target->UVars,PT1) };
      globalQM->enqueueOR(TV,Q);
    }
  }

  if (!(M_leaveOut & M_linearSpolys))
  {
  // try rewriting by considering pairs of equations and trying to reduce
  // to linear by an S-polynomial-like construction
  for(set<TAtomRef>::iterator i_eq = beg_eq; i_eq != end_eq; ++i_eq)
  { 
    if ((*i_eq)->F->numFactors() != 1) continue;
    for(set<TAtomRef>::iterator j_eq = beg_eq; j_eq != end_eq; ++j_eq)
    {
      if (i_eq == j_eq) continue;
      if ((*j_eq)->F->numFactors() != 1) continue;
      TFormRef res = linearSpolys(target,*i_eq,*j_eq,Q,Fs);
      if (asa<TConstObj>(res)) return res; 
    }
  }
  }

  if (!(M_leaveOut & M_uniqueAtomWithVar))
  {
    TFormRef res = uniqueAtomWithVar(target,Fs,Q);
    if (asa<TConstObj>(res)) return res;
  }

  if (!(M_leaveOut & M_onlyNotEQ))
  {
    // if we have a conjunction of atoms of the form A1 /\ ... /\ Ak /\ B1 /\ ... /\ Bl s.t.
    // 1. the Ai contain quantified variables and the Bi do not, and 
    // 2. the Ai are all /= atoms
    // then we can rewrite as /\_i=1..k ~[ai,1 = 0 /\ ... /\ ai,r = 0] /\ B1 /\ ... /\ Bl
    // where the ai,j are the coefficients (as polys in the unquantified variables) of Ai as
    // a polynomial in the quantified variables.
    // In this rewriting, we make this simplification if and only if we can simplify it to a
    // conjunction.

    VarSet Qv = target->QVars;
    TAndRef  A = new TAndObj();
    TAndRef  B = new TAndObj();
    bool formflag = true;
    for(TAndObj::conjunct_iterator itr = Fs->conjuncts.begin(); itr != Fs->conjuncts.end(); ++itr)
    {
      TAtomRef a = asa<TAtomObj>(*itr);
      if (a.is_null()) { formflag = false; break; }
      if ((Qv & a->getVars()).none()) { B->AND(a); }
      else if (a->relop == NEOP && !a->F->isZero()) { A->AND(a); }
      else { formflag = false; break; }	
    }
    if (formflag && A->size() > 0)
    {
      if ((A->getVars() ^ (A->getVars() & Qv)).none()) // A contains only quantified variables
      { 
	Q->enqueue(new QAndObj(B,A->getVars() ^ A->getVars(),B->getVars(),new PAllInequationsObj(target,Qv)));
      }
      else
      {
	//-- for each a /= 0 in A, check that the coefficients of a as a poly in Qv cannot all vanish simultaneously
	for(TAndObj::conjunct_iterator itr = A->conjuncts.begin(); itr != A->conjuncts.end(); ++itr)
	{
	  TAtomRef a = asa<TAtomObj>(*itr);
	  vector<IntPolyRef> VV;
	  target->getPolyManagerPtr()->nonZeroCoefficients(a->F, Qv, VV);
	}
      }
    }
  }

  return NULL;
}

//  REQUIRE: A is f=0 & B is g=0, f,g irreducible.
//  If for quantified variable x & a,b \in Z-{0}, deg_x(f), deg_x(g) > 1,
//  but deg_x(a*f + b*g) <= 1, produce rewrite with A replaced with a*f+b*g=0
//  and rewrite with B replaced by a*f+b*g=0.
//  The idea is that this kind of rewriting allows us to proceed with linear
//  substitution, which doesn't systematically introduce "extraneous" polynomials
//  the way quadratic substitution does.
TFormRef BasicRewrite::linearSpolys(QAndRef target, TAtomRef A, TAtomRef B, 
				    TFQueueRef Q, TFormRef Fs)
{
  PolyManager &M = *A->getPolyManagerPtr();
  IntPolyRef f = A->F->MultiplicityMap.begin()->first;
  IntPolyRef g = B->F->MultiplicityMap.begin()->first;
  VarSet V = f->getVars() & g->getVars() & target->QVars;
  for(VarSet::iterator vitr = V.begin(); vitr != V.end(); ++vitr)
  {
    Variable x = *vitr;
    int df = f->degree(x);
    int dg = g->degree(x);
    if (df != dg || df <= 1) continue;
    vector<IntPolyRef> res = M.standardSpoly(f,g,x);
    IntPolyRef h = res[0], a = res[1], b = res[2];
    if (h->degree(x) != 1) continue; // only interested in reducing to linear (for now)

    if (verbose)
    {
      cout << "Success!(" << M.getName(x) << "): ";
      h->write(M); 
      cout << " from ";
      f->write(M); cout << " and "; g->write(M);
      cout << endl;
    }

    TAtomRef newConstraint = makeAtom(M,h,EQOP);
    if (b->isConstant()) {
      TAndRef F1 = new TAndObj;
      F1->AND(copyAndRemove(Fs,A));
      F1->AND(newConstraint);
      TFormRef res1 = defaultNormalizer->normalize(F1);
      if (constValue(res1) == TRUE) return res1;
      ParentageRef PT1= new PSPolyObj(target,B,A,newConstraint,VarSet(x));
      vector<QAndRef> TV = { new QAndObj(res1,target->QVars,target->UVars,PT1) };
      globalQM->enqueueOR(TV,Q);
    }
    else {
      TAndRef F1 = new TAndObj;
      F1->AND(copyAndRemove(Fs,A));
      F1->AND(newConstraint);
      F1->AND(makeAtom(M,b,NEOP));
      TAndRef F2 = new TAndObj;
      F2->AND(Fs);
      TAtomRef deq = makeAtom(M,b,EQOP);
      F2->AND(deq);
      TFormRef res1 = defaultNormalizer->normalize(F1);
      if (constValue(res1) == TRUE) return res1;
      TFormRef res2 = defaultNormalizer->normalize(F2);
      if (constValue(res2) == TRUE) return res2;
      ParentageRef PT1= new PSPolyObj(target,B,A,newConstraint,x);
      ParentageRef PT2= new PSPolyDegenerateObj(target,B,A,deq,x);
      vector<QAndRef> TV = { new QAndObj(res1,target->QVars,target->UVars,PT1),
			     new QAndObj(res2,target->QVars,target->UVars,PT2) };
      globalQM->enqueueOR(TV,Q);
    }

    if (a->isConstant()) {
      TAndRef F2 = new TAndObj;
      F2->AND(copyAndRemove(Fs,B));
      F2->AND(newConstraint);
      TFormRef res2 = defaultNormalizer->normalize(F2);
      if (constValue(res2) == -1) return res2;
      ParentageRef PT2= new PSPolyObj(target,A,B,newConstraint,VarSet(x));
      vector<QAndRef> TV = { new QAndObj(res2,target->QVars,target->UVars,PT2) };
      globalQM->enqueueOR(TV,Q);
    }
    else {
      TAndRef F1 = new TAndObj;
      F1->AND(copyAndRemove(Fs,A));
      F1->AND(newConstraint);
      F1->AND(makeAtom(M,a,NEOP));
      TAndRef F2 = new TAndObj;
      F2->AND(Fs);
      TAtomRef deq = makeAtom(M,a,EQOP);
      F2->AND(deq);
      TFormRef res1 = defaultNormalizer->normalize(F1);
      if (constValue(res1) == TRUE) return res1;
      TFormRef res2 = defaultNormalizer->normalize(F2);
      if (constValue(res2) == TRUE) return res2;
      ParentageRef PT1= new PSPolyObj(target,A,B,newConstraint,x);
      ParentageRef PT2= new PSPolyDegenerateObj(target,A,B,deq,x);
      vector<QAndRef> TV = { new QAndObj(res1,target->QVars,target->UVars,PT1),
			     new QAndObj(res2,target->QVars,target->UVars,PT2) };
      globalQM->enqueueOR(TV,Q);
    }
  }   
  return NULL;
}

TFormRef BasicRewrite::linSubs(QAndRef target, TAtomRef A, TFQueueRef Q, TFormRef Fs)
{
  if (Fs.vpval() == A.vpval()) return NULL;

  IntPolyRef P = A->F->MultiplicityMap.begin()->first;
  if ((P->getVars() & target->UVars).any()) return NULL;

  VarSet V = P->linearlyOccurringVariables();
  for(VarSet::iterator itr = V.begin(); itr != V.end(); ++itr)
  {
    Variable x = *itr;

    /* Don't keep, but I'm experimenting with disallowing substitutions
       for free variables from an equation containing quantified variables. */
    if ((P->getVars() & target->QVars).any() && (target->QVars & x).none())  continue;

    vector<QAndRef> TV;

    // Generic Case
    ParentageRef GCP = new PSubsObj(target,A,x,'G');
    TFormRef GC = linearSubstGenericCase(*(A->F->M),Fs,P,x,A,(target->QVars & x).none()); ++numGenericSubs;
    GC = defaultNormalizer->normalize(GC);
    if (constValue(GC) == TRUE) return GC;
    TV.push_back(new QAndObj(GC,target->QVars,target->UVars + x,GCP));

    // Degenerate Case
    if (!fullyGeneric) // <- might we want a version that ignores exploring degenerate cases?
    {
      ParentageRef DCP = new PSubsObj(target,A,x,'D');
      TFormRef DC = linearSubstDegenerateCase(*(A->F->M),Fs,P,x,A); ++numDegenerateSubs;
      DC = defaultNormalizer->normalize(DC);
      if (constValue(DC) == TRUE) return DC;
      TV.push_back(new QAndObj(DC,target->QVars,target->UVars,DCP));
    }

    // Add result to queue
    globalQM->enqueueOR(TV,Q);
    if (!Q->constValue.is_null()) return Q->constValue;
  }
  return NULL;
}

TFormRef BasicRewrite::splitOnAtom(QAndRef target, TAtomRef A, TFQueueRef Q)
{    
  // Split equation A to form a separate conjunction for each factor
  TFormRef Fs = copyAndRemove(target->F,A);
  vector<QAndRef> disjuncts;
  for(map<IntPolyRef,int>::iterator itr = A->factorsBegin();itr != A->factorsEnd(); ++itr)
  {
    TAndRef D = new TAndObj;
    D->AND(Fs);
    D->AND(makeAtom(*(A->F->M),itr->first,EQOP));
    TFormRef Dsimple = defaultNormalizer->normalize(D);
    if (constValue(Dsimple) == TRUE) return new TConstObj(TRUE);
    disjuncts.push_back(new QAndObj(Dsimple,target->QVars,target->UVars,new PSplitObj(target,A,itr->first)));
  }
  globalQM->enqueueOR(disjuncts,Q);
  
  return NULL;
}

// I'd like to add a new ``operator'' that looks to cancel out
// higher-order terms in order to leave a polynomial that's
// linear in one of the quantified variables.
/*
Let's do a simple, cheap cop-out version now.  We need to look
for polys p and q and variable x s.t. p and q are equational
contraints and x
 */

// This function does "linear substitution" not on x, but on x^k
// A: atom P = 0, where P is irreducible
TFormRef BasicRewrite::linSubsXpowK(QAndRef target, TAtomRef A, TFQueueRef Q, TFormRef Fs)
{
  IntPolyRef P = A->F->MultiplicityMap.begin()->first;
  if ((P->getVars() & target->UVars).any()) return NULL;

  VarSet V = P->variableUniqueDegrees();
  for(VarSet::iterator itr = V.begin(); itr != V.end(); ++itr)
  {
    Variable x = *itr;
    int dx = P->degree(x);
    if (dx == 1) continue;

    // Split polynomials appearing in Fs into B = "bad" and G = "good", removing A; i.e. Fs = A /\ B /\ G
    set<IntPolyRef> B, G;
    if (asa<TAndObj>(Fs) != NULL)
    {
      TAndRef FsA = asa<TAndObj>(Fs);
      for(TAndObj::conjunct_iterator citr = FsA->conjuncts.begin(); citr != FsA->conjuncts.end(); ++citr)
      {
	TAtomRef nextA = asa<TAtomObj>(*citr);
	if (A.identical(nextA)) continue;
	for(map<IntPolyRef,int>::iterator fitr = nextA->factorsBegin(); fitr != nextA->factorsEnd(); ++fitr)
	{
	  int g = fitr->first->gcdOfVariableExponents(x);
	  if (g % dx == 0)
	    G.insert(fitr->first);
	  else
	    B.insert(fitr->first);
	}
      }
    }

    // Still need to add the case in which there *are* bad guys
    

    // The no bad guys case!
    if (B.size() == 0)
    {
      IntPolyRef Ps = P->reduceDegreeInX(x,dx);
      TAtomRef As = makeAtom(*A->getPolyManagerPtr(),Ps,EQOP);
      TAndRef Gs = new TAndObj;
      if (dx % 2 == 0) { TAtomRef tA = makeAtom(*A->getPolyManagerPtr(),new IntPolyObj(x),GEOP); Gs->AND(tA); }

      if (asa<TAndObj>(Fs) != NULL)
      {
	TAndRef FsA = asa<TAndObj>(Fs);
	for(TAndObj::conjunct_iterator citr = FsA->conjuncts.begin(); citr != FsA->conjuncts.end(); ++citr)
	{
	  TAtomRef nextA = asa<TAtomObj>(*citr);
	  if (A.identical(nextA)) continue;
	  FactRef K = new FactObj(*A->getPolyManagerPtr());
	  for(map<IntPolyRef,int>::iterator fitr = nextA->factorsBegin(); fitr != nextA->factorsEnd(); ++fitr)
	    K->addFactor(fitr->first->reduceDegreeInX(x,dx),fitr->second);
	  Gs->AND(new TAtomObj(K,nextA->relop));
	}
      }
      
      vector<QAndRef> TV;
      // Generic Case
      ParentageRef GCP = new PSubsObj(target,A,x,'G',dx);
      TFormRef GC = linearSubstGenericCase(*(As->F->M),Gs,Ps,x,As,false); ++numGenericSubs;
      if ((x & target->QVars).none() && constValue(GC) != FALSE) 
      { 
	TAndRef tmp = new TAndObj; tmp->AND(A); tmp->AND(GC); GC = tmp;
      }
      GC = defaultNormalizer->normalize(GC);
      if (constValue(GC) == TRUE) return GC;
      TV.push_back(new QAndObj(GC,target->QVars,target->UVars + x,GCP));
      
      // Degenerate Case
      ParentageRef DCP = new PSubsObj(target,A,x,'D',dx);
      TFormRef DC = linearSubstDegenerateCase(*(A->F->M),Fs,P,x,A); ++numDegenerateSubs;
      DC = defaultNormalizer->normalize(DC);
      if (constValue(DC) == TRUE) return DC;
      TV.push_back(new QAndObj(DC,target->QVars,target->UVars,DCP));
      
      // Add result to queue
      globalQM->enqueueOR(TV,Q);
      if (!Q->constValue.is_null()) return Q->constValue;      
    }
  }
  return NULL;
}

int getSignInfo(TAndRef Fs, IntPolyRef c1, PolyManager *PM)
{
  //  cout << "##################" << endl;  Fs->getPolyManagerPtr()->printContents();
  TAtomRef A = makeAtom(*PM,c1,LTOP);
  TFormRef c1L = defaultNormalizer->normalize(A && Fs);
  TFormRef c1E = defaultNormalizer->normalize((A->relop = EQOP, A) && Fs);
  TFormRef c1G = defaultNormalizer->normalize((A->relop = GTOP, A) && Fs);
  int sigma1 = ALOP;
  if (constValue(c1L) == FALSE) sigma1 ^= LTOP;
  if (constValue(c1E) == FALSE) sigma1 ^= EQOP;
  if (constValue(c1G) == FALSE) sigma1 ^= GTOP;
  return sigma1;
}

TFormRef BasicRewrite::uniqueAtomWithVar(QAndRef target, TAndRef Fs, TFQueueRef Q)
{
  map<TFormRef,VarSet> M = atomsContainingUniqueVar(Fs, target->QVars);  
  for(map<TFormRef,VarSet>::iterator itr = M.begin(); itr != M.end(); ++itr)
  {
    // Pull out Atom a and poly P
    TAtomRef a = asa<TAtomObj>(itr->first);              if (a->F->MultiplicityMap.size() != 1) continue;
    IntPolyRef P = a->F->MultiplicityMap.begin()->first; if (a->F->MultiplicityMap.begin()->second != 1) continue;

    // I really need to remove each atom from Fs before I ask for ths sign info.
    // Otherwise it can end up that the atom I'm going to eliminate carries the info that
    // causes the deduction ... which is circular.  That's why I'm building Fsp.
    TAndRef Fsp = new TAndObj;
    Fsp->AND(copyAndRemove(Fs,a));

    // Loop over each variable looking for a substitution
    for(VarSet::iterator vitr = itr->second.begin(); vitr != itr->second.end(); ++vitr)
    {
      VarSet x = *vitr;
      if (P->degree(x) != 1) continue;
      IntPolyRef c1 = target->getPolyManagerPtr()->coef(P,x,1);
      IntPolyRef c0 = target->getPolyManagerPtr()->coef(P,x,0);
      int sigma1 = getSignInfo(Fsp,c1,target->getPolyManagerPtr());
      int sigma0 = getSignInfo(Fsp,c0,target->getPolyManagerPtr());
      
      vector<QAndRef> TV;
      if (consistentWith(sigma0,a->relop)) 
      { /* Ex x [ a ] is always sat at x = 0. */ 
	ParentageRef pr = new PUniqueAtomObj(target,x,a,0);
	TFormRef GC = defaultNormalizer->normalize(Fsp);
	if (constValue(GC) == TRUE) return GC;
	TV.push_back(new QAndObj(GC,target->QVars,target->UVars + x,pr));
	
      }
      else if ((sigma1 & EQOP) == 0)      
      { /* Ex x [ a ] is always sat */ 
	ParentageRef pr = new PUniqueAtomObj(target,x,a,1);
	TFormRef GC = defaultNormalizer->normalize(Fsp);
	if (constValue(GC) == TRUE) return GC;
	TV.push_back(new QAndObj(GC,target->QVars,target->UVars + x,pr));
      }
      else 
      { /* break into cases. */ 

	// CASE not b relop 0, i.e. we need a nonzero ldcoef and x.
	ParentageRef pr1 = new PUniqueAtomObj(target,x,a,2);
	TAtomRef A = makeAtom(*Fs->getPolyManagerPtr(),c1,NEOP);
	TAtomRef B = makeAtom(*Fs->getPolyManagerPtr(),c0,negateRelop(a->relop));
	TFormRef GC = defaultNormalizer->normalize(Fsp && A && B);
	if (constValue(GC) == TRUE) return GC;
	TV.push_back(new QAndObj(GC,target->QVars,target->UVars + x,pr1));
	
	// CASE x = 0, i.e. b relop 0
	ParentageRef pr0 = new PUniqueAtomObj(target,x,a,3);
	TAtomRef Bp = makeAtom(*Fs->getPolyManagerPtr(),c0,a->relop);
	TFormRef DC = defaultNormalizer->normalize(Fsp && Bp);
	if (constValue(DC) == TRUE) return DC;
	TV.push_back(new QAndObj(DC,target->QVars,target->UVars + x,pr0));
      }
      globalQM->enqueueOR(TV,Q);      
    }
  }
  return NULL;
}


void printDerivation(TFormRef F)
{
  TOrRef tor = new TOrObj();
  tor->OR(F);
  for(set<TFormRef>::iterator itr = tor->disjuncts.begin(); itr != tor->disjuncts.end(); ++itr)
  {
    QAndRef sa = asa<QAndObj>(*itr);
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

void printDerivationIfAnd(TFormRef F)
{
  QAndRef sa = asa<QAndObj>(F);
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

}//end namespace tarski
