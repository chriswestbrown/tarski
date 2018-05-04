#include "builder.h"

//#define _OC_DEBUG_

using namespace std;

namespace tarski {

extern Word CSSP(Word a, Word b);

bool BuilderObj::add(FactRef f)
{
  if (inErrorState()) { return false; }
  PolyManager &PM = *(D->getPolyManager());
  VarOrderRef X = D->getVarOrder();
  int k = 0;
  for(map<IntPolyRef,int>::iterator itr = f->MultiplicityMap.begin(); itr != f->MultiplicityMap.end(); ++itr)
  {
    IntPolyRef next = PM.getCannonicalCopy(itr->first);
    if (pfset.find(next) == pfset.end()) 
    { 
      int lev = X->level(next);
      queue<IntPolyRef> &Q = Unprocessed[lev];
      Q.push(next);

      //      Unprocessed[X->level(next)].push(next); 
      pfset.insert(next); 
      ++k; 
    }
  }
  return k > 0;
}

bool BuilderObj::oneStep(int n)
{
  if (inErrorState()) { return false; }
  PolyManager &PM = *(D->getPolyManager());
  VarOrderRef X = D->getVarOrder();

  // look for p in lowest level non-empty queue.  h is that level
  int h = 1; while(h <= n && Unprocessed[h].size() == 0 && Processed[h].size() == 0) ++h;
    
  // if nothing is found, return true
  if (h > n) { return true; }

  // if no already processed polynomials at level h, then take next from the Unprocessed queue of level h
  if (Processed[h].size() == 0) 
  {
    IntPolyRef p = Unprocessed[h].front(); Unprocessed[h].pop(); 
    if (X->partialEval(p,D->getAlpha(),X->level(p))->isZero()) { setError(p); return false; }
    
    if (h == 1) { Processed[h].push(p); return false; }
    FactRef f = new FactObj(PM);
    if (p->degree((*X)[h]) > 1) 
    { 
      FactRef discFactors = PM.discriminantFactors(p,(*X)[h]);
      //cout << "FACTORS: "; discFactors->write(); cout << endl;
      f->addFactors(discFactors,1);
    }

    FactRef ldcfFactors = PM.ldcfFactors(p,(*X)[h]);
    //cout << "LDCFFACTORS: "; ldcfFactors->write(); cout << endl;
    f->addFactors(ldcfFactors,1);
    
    if (D->get(h)->a->compareToNegInfty() != 0)
    {
      FactRef reslowerFactors = PM.resultantFactors(p,D->get(h)->l,(*X)[h]);
      f->addFactors(reslowerFactors,1);
    }
    if (D->get(h)->b->compareToPosInfty() != 0)
    {
      FactRef resupperFactors = PM.resultantFactors(p,D->get(h)->u,(*X)[h]);
      f->addFactors(resupperFactors,1);	  
    }
    add(f);
    if (inErrorState()) { return false; }
    Processed[h].push(p);
    return false;
  }
  
  // Grab p, the next polynomial to refine the cell by
  IntPolyRef p = Processed[h].front(); Processed[h].pop();
  // cout << "adding ... "; p->write(PM); cout << endl;
  
  // Evaluate at rational point (partially)
  int k = X->level(p);
  IntPolyRef q = X->partialEval(p,D->getAlpha(),k-1);
  if (q->isZero()) { setError(p); return false; }
  if (q->isConstant()) { return false; }
  
  // Create new cell bound
  CellBoundRef cb1 = new CellBoundObj(LELTI(D->getAlpha(),X->level(q)),q->getSaclibPoly(),p);
  // cout << "new bound to merge: " << cb1->toStr(&PM) << endl;
  
  // merge!
  D->get(k) = merge(D->get(k),cb1);
  return false;
}

BuilderRef BuilderObj::buildOne(IntPolyRef p, VarOrderRef X, Word PT, int dim)
{
  dim = X->level(p);
  OpenCellRef D = new OpenCellObj(X,PT,X->size());
  PolyManager &PM = *(X->getPolyManager());
  // cerr << "START buildOne: "; p->write(PM); cout << endl;
  FactRef facts = PM.discriminantFactors(p,(*X)[dim]);
  FactRef ldcfFactors = PM.ldcfFactors(p,(*X)[dim]);
  facts->addFactors(ldcfFactors,1);
  BuilderRef B = new BuilderObj(D);
  if (B->inErrorState()) { return B; }
  
  if (dim <= 2) 
  { B->add(facts); B->close(); }
  else
  {
    B->add(facts);
    if (X->partialEval(p,PT,dim)->isZero()) 
    {  
      cout << "Error! poly "; p->write(PM); cout << " is zero at evaluation point." << endl;; 
      exit(1);
    }
  }
  IntPolyRef A = dim == 1 ? p : X->partialEval(p,PT,dim-1);  
  CellBoundRef CB;
  if (A->isConstant())
    CB = new CellBoundObj(LELTI(PT,dim));
  else
    CB = new CellBoundObj(LELTI(PT,dim), A->getSaclibPoly(), p);
  D->get(dim) = CB;
  B->pfset.insert(p);
  // cerr << "END buildOne: " << endl; B->D->debugPrint(cerr);
  return B;
}


BuilderRef BuilderObj::buildOneRec(IntPolyRef p, VarOrderRef X, Word PT, int dim)
{
  dim = X->level(p);
  OpenCellRef D = new OpenCellObj(X,PT,X->size());
  PolyManager &PM = *(X->getPolyManager());
  // cerr << "START buildOneRec: "; p->write(PM); cout << endl;
  BuilderRef B = new BuilderObj(D);
  
  if (X->partialEval(p,PT,dim)->isZero()) {  B->setError(p); return B; }
  
  if (dim < 2) 
  { /* Nothing to do, right? B->add(facts); B->close(); */ }
  else
  {
    FactRef facts = PM.discriminantFactors(p,(*X)[dim]);
    FactRef ldcfFactors = PM.ldcfFactors(p,(*X)[dim]);
    facts->addFactors(ldcfFactors,1);
    B->addRec(facts,dim);
    if (B->inErrorState()) return B;
  }
  IntPolyRef A = dim == 1 ? p : X->partialEval(p,PT,dim-1);  
  CellBoundRef CB;
  if (A->isConstant())
    CB = new CellBoundObj(LELTI(PT,dim));
  else
    CB = new CellBoundObj(LELTI(PT,dim), A->getSaclibPoly(), p);
  D->get(dim) = CB;
  if (!p->isConstant()) { B->pfset.insert(p); }
  // cerr << "END buildOneRec: " << endl; B->D->debugPrint(cerr);
  return B;
}


bool BuilderObj::addRec(FactRef f, int k)
{
  if (inErrorState()) { return false; }
  VarOrderRef X = D->getVarOrder();
  PolyManager &PM = *(D->getPolyManager());
  Word PT = D->getAlpha();
  for(map<IntPolyRef,int>::iterator itr = f->MultiplicityMap.begin(); itr != f->MultiplicityMap.end(); ++itr)
  {
    IntPolyRef p = itr->first;
    if (pfset.find(p) != pfset.end()) { continue; }
    //    if (k == -1) k = X->level(p);
    if (k == -1) k = X->size();
    BuilderRef B1 =  buildOneRec(p,X,PT,k);
    if (B1->inErrorState()) { setError(B1->getErrorPoly()); return false; }
    mergeBintoA(this,B1,k);
  }
  return true;
}

string BuilderObj::projFacSummary()
{
  ostringstream sout;
  int n = D->getVarOrder()->size();
  vector<int> stats(n+1);
  for(set<IntPolyRef,PolyManager::PolyLT>::iterator itr = pfset.begin(); itr != pfset.end(); ++itr)
  {
    stats[D->getVarOrder()->level(*itr)]++;
  }
  sout << stats[1];
  for(int i = 2; i <= n; ++i)
    sout << ", " << stats[i];
  return sout.str();  
}

string BuilderObj::projFacInfo()
{
  ostringstream out;
  int n = D->getVarOrder()->size();
  vector<ostringstream*> V(n+1); for(int i = 0; i < n+1; ++i) V[i] = new ostringstream();
  for(set<IntPolyRef,PolyManager::PolyLT>::iterator itr = pfset.begin(); itr != pfset.end(); ++itr)
  {
    PushOutputContext(*(V[D->getVarOrder()->level(*itr)]));
    (*itr)->write(*(D->getPolyManager()));
    SWRITE("\n");
    PopOutputContext();
  }
  
  for(int i = 1; i <= n; ++i)
  { out << V[i]->str() << endl; delete V[i]; }
  return out.str();
}

vector< set<IntPolyRef,PolyManager::PolyLT> >  BuilderObj::projFacByLevel()
{
  int n = D->getVarOrder()->size();
  vector< set<IntPolyRef,PolyManager::PolyLT> > PF(n+1);
  for(set<IntPolyRef,PolyManager::PolyLT>::iterator itr = pfset.begin(); itr != pfset.end(); ++itr)
    PF[D->getVarOrder()->level(*itr)].insert(*itr);
  return PF;
}

void BuilderObj::mergeBintoA(BuilderRef A, BuilderRef B, int dim) // dim <= dimension B, dimension A
{
  if (A->inErrorState()) { return; }
  if (B->inErrorState()) { A->setError(B->getErrorPoly()); return; }
  OpenCellRef cA = A->D;
  OpenCellRef cB = B->D;
  int ds = std::min(cA->dimension(),cB->dimension());
  if (ds < dim) 
  { 
    mergeBintoA(A,B,ds);  
    return;
  }
  if (dim == 1)
  {
    cA->mergeBound(1,cB->get(1));
  }
  else
  {      
    mergeBintoA(A,B,dim-1);
    PolyManager &PM = *(cA->getPolyManager());
    VarOrderRef X = cA->getVarOrder();
    FactRef f = new FactObj(PM);
    if (!cA->get(dim)->l.is_null() && !cB->get(dim)->l.is_null())
    {
      FactRef reslowerFactors = PM.resultantFactors(cA->get(dim)->l,cB->get(dim)->l,(*X)[dim]);
      f->addFactors(reslowerFactors,1);
    }
    
    if (!cA->get(dim)->u.is_null() && !cB->get(dim)->u.is_null())
    {
      FactRef resupperFactors = PM.resultantFactors(cA->get(dim)->u,cB->get(dim)->u,(*X)[dim]);
      f->addFactors(resupperFactors,1);
    }
    
    int lowOrHigh;
    CellBoundRef newA = mergeWithError(cA->get(dim),cB->get(dim),lowOrHigh);
    if (newA.is_null()) {
      FactRef f = lowOrHigh < 0 ?
	PM.resultantFactors(cA->get(dim)->l,cB->get(dim)->l,(*X)[dim]) :
	PM.resultantFactors(cA->get(dim)->u,cB->get(dim)->u,(*X)[dim]);
      
      // find factor of f that vanishes and set it as error polynomial
      // BUG FIX HERE
      for(map<IntPolyRef,int>::iterator itr = f->MultiplicityMap.begin(); itr != f->MultiplicityMap.end(); ++itr)
      {
	IntPolyRef fs = itr->first;
	Word alpha = cA->getAlpha();
	if(X->partialEval(fs,alpha,dim-1)->isZero()) { A->setError(fs); return; }
      }
      cerr << "Error in BuilderObj::mergeBIntoA!" << endl;
      exit(1);
    }
    // if (!(newA->l.identical(cA->get(dim)->l) &&  newA->u.identical(cA->get(dim)->u)) && 
    // 	!(newA->l.identical(cB->get(dim)->l) &&  newA->u.identical(cB->get(dim)->u)))
    if (!( newA->identicalLower(cA->get(dim)) &&  newA->identicalUpper(cA->get(dim)) ) && 
	!( newA->identicalLower(cB->get(dim)) &&  newA->identicalUpper(cB->get(dim)) ) )
    {
      if (!newA->l.is_null() && !newA->u.is_null())
      {
	FactRef resCross = PM.resultantFactors(newA->l,newA->u,(*X)[dim]);
	f->addFactors(resCross,1);
      }
    }
    //    A->add(f);
    //    A->close();
    A->addRec(f);  if (A->inErrorState()) { return; }
    cA->get(dim) = newA;
  }

  VarOrderRef X = cA->getVarOrder();
  for(set<IntPolyRef,PolyManager::PolyLT>::iterator itr = B->pfset.begin(); itr != B->pfset.end(); ++itr)
    if (X->level(*itr) == dim)
      A->pfset.insert(*itr);
}

void BuilderObj::splitAtLevelK(BuilderRef A, BuilderRef Ap, int dim, int k, vector<BuilderRef> &V)
{
  if (A->inErrorState() || Ap->inErrorState() || k < 1 || k > dim) { return; }
  OpenCellRef cA = A->D;
  OpenCellRef cAp = Ap->D;
  int level = k;
  VarOrderRef X = A->getCell()->getVarOrder();
  
  //-- NOTE!!!! I should modify this so that the newly created or "split off" cell gets the splitting polynomial
  //--          added to its known sign-invariant set in certain circumstances, namely if splitting section is
  //--          the 1st root of the polynomial, or if the lower boundary of the new cell is the lone-lower section
  //--          of the same polynomial as the splitting section
  int splitPolyTruthInvariantIn[] = { false, false };

  RealAlgNumRef lowerBoundA = cA->get(level)->a;
  RealAlgNumRef lowerBoundAp = cAp->get(level)->a;
  int c2 = lowerBoundA->compareToRobust(lowerBoundAp);
  if (c2 > 0) { throw TarskiException("Unexpected comparison result in splitAtLevelK!"); }
  else if (c2 < 0)
  {
    Word x_low = rationalPointInInterval(lowerBoundA,lowerBoundAp);
    
    Word PT_low_rev = NIL, T_alpha = cAp->getAlpha();
    for(int i = 0; i < level-1; ++i,T_alpha = RED(T_alpha)) { PT_low_rev = COMP(FIRST(T_alpha),PT_low_rev); }
    PT_low_rev = COMP(x_low,PT_low_rev);
    Word PT_low = CINV(PT_low_rev);
    Word PT = cA->choosePointCompletion(PT_low);
    OpenCellRef cAkL = new OpenCellObj(cA->getVarOrder(),PT,dim);
    
    //-- bounds from level < k come from Ap
    for(int i = 1; i < k; ++i)
      cAkL->setBound(i,cAp->get(i)->clone());
    
    //-- bound for level = k comes from A's lower bound and Ap's lower bound (with direction switched)
    IntPolyRef t_l = cA->get(k)->l;
    Word j_l = t_l.is_null() ? 0 : asa<RealRootIUPObj>(cA->get(k)->a)->rootIndex();
    IntPolyRef t_u = cAp->get(k)->l;
    Word j_u = t_u.is_null() ? 0 : asa<RealRootIUPObj>(cAp->get(k)->a)->rootIndex();
    CellBoundRef boundk = new CellBoundObj(X,PT,k,t_l,j_l,t_u,j_u);
    cAkL->setBound(k,boundk);
    splitPolyTruthInvariantIn[0] =  (j_u == 1 || (!t_l.is_null()) && t_l->equal(t_u) && j_u == j_l + 1);
    
    //-- bounds for level > k come from A, but the algebraic coords a & b have to be recomputed with PT
    for(int i = k+1; i <= dim; ++i)
    {
      IntPolyRef t_l = cA->get(i)->l;
      Word j_l = t_l.is_null() ? 0 : asa<RealRootIUPObj>(cA->get(i)->a)->rootIndex();
      IntPolyRef t_u = cA->get(i)->u;
      Word j_u = t_u.is_null() ? 0 : asa<RealRootIUPObj>(cA->get(i)->b)->rootIndex();
      CellBoundRef boundi = new CellBoundObj(X,PT,i,t_l,j_l,t_u,j_u);
      cAkL->setBound(i,boundi);
    }
    
    // Create the new cell and give it the right sign-invariant factors set
    BuilderRef B = new BuilderObj(cAkL);
    for(set<IntPolyRef,PolyManager::PolyLT>::iterator itr = Ap->pfset.begin(); itr != Ap->pfset.end(); ++itr)
      if (Ap->D->getVarOrder()->level(*itr) < k) //note
	B->pfset.insert(*itr);
    for(set<IntPolyRef,PolyManager::PolyLT>::iterator itr = A->pfset.begin(); itr != A->pfset.end(); ++itr)
      if (A->D->getVarOrder()->level(*itr) >= k) //note
	B->pfset.insert(*itr);
    if (splitPolyTruthInvariantIn[0]) 
      B->pfset.insert(t_u);

    
    V.push_back(B);
  }
  else { V.push_back(NULL); }
  
  RealAlgNumRef upperBoundA = cA->get(level)->b;
  RealAlgNumRef upperBoundAp = cAp->get(level)->b;
  
  int c1 = upperBoundAp->compareToRobust(upperBoundA);
  
  if (c1 > 0) { throw TarskiException("Unexpected comparison result in splitAtLevelK!"); }
  else if (c1 < 0)
  {
    Word x_high = rationalPointInInterval(upperBoundAp,upperBoundA);
    
    Word PT_high_rev = NIL, T_alpha = cAp->getAlpha();
    for(int i = 0; i < level-1; ++i,T_alpha = RED(T_alpha)) { PT_high_rev = COMP(FIRST(T_alpha),PT_high_rev); }
    PT_high_rev = COMP(x_high,PT_high_rev);
    Word PT_high = CINV(PT_high_rev);
    Word PT = cA->choosePointCompletion(PT_high);
    OpenCellRef cAkU = new OpenCellObj(cA->getVarOrder(),PT,dim);
    
    //-- bounds from level < k come from Ap
    for(int i = 1; i < k; ++i)
      cAkU->setBound(i,cAp->get(i)->clone());
    
    //-- bound for level = k comes from Ap's upper bound (with direction switched) and A's upper bound 
    IntPolyRef t_l = cAp->get(k)->u;
    Word j_l = t_l.is_null() ? 0 : asa<RealRootIUPObj>(cAp->get(k)->b)->rootIndex();
    IntPolyRef t_u = cA->get(k)->u;
    Word j_u = t_u.is_null() ? 0 : asa<RealRootIUPObj>(cA->get(k)->b)->rootIndex();
    CellBoundRef boundk = new CellBoundObj(X,PT,k,t_l,j_l,t_u,j_u);
    cAkU->setBound(k,boundk);
    int numRootsOft_l = asa<RealRootIUPObj>(cAp->get(k)->b)->numberOfRoots();
    splitPolyTruthInvariantIn[1] =  (j_l == numRootsOft_l || (!t_u.is_null()) && t_l->equal(t_u) && j_u == j_l + 1);
    
    //-- bounds for level > k come from A, but the algebraic coords a & b have to be recomputed with PT
    for(int i = k+1; i <= dim; ++i)
    {
      IntPolyRef t_l = cA->get(i)->l;
      Word j_l = t_l.is_null() ? 0 : asa<RealRootIUPObj>(cA->get(i)->a)->rootIndex();
      IntPolyRef t_u = cA->get(i)->u;
      Word j_u = t_u.is_null() ? 0 : asa<RealRootIUPObj>(cA->get(i)->b)->rootIndex();
      CellBoundRef boundi = new CellBoundObj(X,PT,i,t_l,j_l,t_u,j_u);
      cAkU->setBound(i,boundi);
    }
    
    // Create the new cell and give it the right sign-invariant factors set
    BuilderRef B = new BuilderObj(cAkU);
    for(set<IntPolyRef,PolyManager::PolyLT>::iterator itr = Ap->pfset.begin(); itr != Ap->pfset.end(); ++itr)
      if (Ap->D->getVarOrder()->level(*itr) < k)
	B->pfset.insert(*itr);
    for(set<IntPolyRef,PolyManager::PolyLT>::iterator itr = A->pfset.begin(); itr != A->pfset.end(); ++itr)
      if (A->D->getVarOrder()->level(*itr) >= k)
	B->pfset.insert(*itr);
    if (splitPolyTruthInvariantIn[1]) 
      B->pfset.insert(t_l);

    V.push_back(B);
    
  }
  else { V.push_back(NULL); }

  // TODO: Add one more optimization - if there are both a lower and an upper cell generated by 
  // this split, the poly defining the boundary between the lower cell and Ap might be deducible
  // as sign-invariant in the upper cell.  Similarly, the poly definint the boundary between the
  // upper cell and Ap might be deducible as sign-invariant in the lower cell.
}



void BuilderObj::mkNuCAD1(VarOrderRef X, IntPolyRef p, int dim, Word alpha, vector<BuilderRef> &V, vector<string> &L)
{
  cerr << "no longer supported!" << endl;
  exit(0);
  PolyManager *ptrPM = X->getPolyManager();
  
  BuilderRef root = new BuilderObj(new OpenCellObj(X,alpha,dim));
  V.push_back(root);
  L.push_back("C");
  int front = 0;

  while(front < V.size())
  {
    cerr << "stack size = " << V.size() << endl;
    BuilderRef D = V[front];
    string label = L[front];
    ++front;

    if (label[label.size()-1] == 'X') continue;

    BuilderRef DX;
    do {
      DX = buildOneRec(p,X,D->getCell()->getAlpha(),dim);
      if (!DX->inErrorState() && (mergeBintoA(DX,D,dim), !DX->inErrorState())) { break; }
      cerr << "Error!" << endl;
      IntPolyRef f = DX->getErrorPoly();
      f->write(*ptrPM);
      SWRITE("\n");
      OWRITE(DX->getCell()->getAlpha());
      SWRITE("\n");

      int i = X->level(f);
      vector<IntPolyRef> F;
      F.push_back(f);
      do {
	bool nullf = false; 
	for(int j = 0; j < F.size() && !nullf; ++j) 
	{ nullf = nullf ||  X->isNullified(F[j],D->getCell()->getAlpha(),i); }
	if (!nullf) break;
	vector<IntPolyRef> G;
	for(int j = 0; j < F.size(); ++j)
	{
	  FactRef ld = ptrPM->ldcfFactors(F[j],(*X)[i]);
	  for(map<IntPolyRef,int>::iterator itr = ld->MultiplicityMap.begin(); itr != ld->MultiplicityMap.end(); ++itr)
	    G.push_back(itr->first);
	}
	swap(F,G);
	--i;
      } while(i > 1);
      
      RealAlgNumRef alpha_i = rationalToRealAlgNum(LELTI(D->getCell()->getAlpha(),i));
      RealAlgNumRef zeta = D->getCell()->get(i)->a;
      for(int j = 0; j < F.size(); ++j)
      {
	IntPolyRef g = X->partialEval(F[j],D->getCell()->getAlpha(),i-1);
	Word Q, R;
	IPQR(1,g->sP,asa<RealRootIUPObj>(alpha_i)->saclibPoly(),&Q,&R);
	if (R == 0) {
	  g = new IntPolyObj(1,Q,g->getVars());
	}
	vector<RealRootIUPRef> Z;
	if (!g->isConstant()) { Z = RealRootIsolateSquarefree(g); }
	int m = Z.size()-1; while(m >= 0 && alpha_i->compareToRobust(Z[m]) <= 0) { --m; }
	if (m >= 0) { zeta = max(zeta,Z[m]); }
      }
      Word alpha_i_new = rationalPointInInterval(zeta,alpha_i);

      // Make new alpha
      Word alpha_new = NIL;
      Word t = D->getCell()->getAlpha();
      for(int j = 0; j < i-1; ++j, t = RED(t))
	alpha_new = COMP(FIRST(t),alpha_new);
      alpha_new = CINV(COMP(alpha_i_new,alpha_new));
      Word alpha_new_final = D->getCell()->moveSamplePoint(alpha_new);
    } while(true);

    V.push_back(DX);
    L.push_back(label+"X");

    for(int k = 1; k <= dim; ++k)
    {
      const char* t[] = {"0","1","1","3","4","5","6","7","8","9"}; 
      vector<BuilderRef> W;
      splitAtLevelK(D,DX,dim,k,W);
      if (!W[0].is_null()) { 
	V.push_back(W[0]);
	L.push_back((label+t[k])+"L"); }
      if (!W[1].is_null()) { 
	V.push_back(W[1]);
	L.push_back((label+t[k])+"U"); }
    }
  }
}
}//end namespace tarski


