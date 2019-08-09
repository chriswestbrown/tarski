#include "opennucad.h"
#include "../smtsolver/minhitset/naive/hitset.h"

//#define _OC_DEBUG_
#ifdef _OC_DEBUG_
#define _OC_DEBUG2_
#endif

using namespace std;

namespace tarski {
  


void ONuCADObj::mkNuCADConjunction(VarOrderRef X, TAndRef F, int dim, Word alpha)
{
  // Initialize V/L as an empty "queue" of labeled cells, then enque the cell R^dim
  PolyManager *ptrPM = X->getPolyManager();
  BuilderRef rootBuilder = new BuilderObj(new OpenCellObj(X,alpha,dim));
  int front = 0;
  nodeQueue->push(root = new NodeObj(NULL,rootBuilder,UNDET,0,'C'));
  fullRefine();
}


  void ONuCADObj::fullRefine()
  {
    while(!nodeQueue->stopSearch())
    {
      NodeRef node = nodeQueue->front(); nodeQueue->pop();
      if (node->getTruthValue() == UNDET)
	NuCADSplit(getVarOrder(),chooser,getDim(),node);
    }  
  }
  
/*
  Given label, does the "next" refinement step in NuCAD conjunction
  by splitting the cell defined by the label.
 */
  void ONuCADObj::refineNuCADConjunction(const std::string &label)
  {
    NodeRef node = getNode(label);
    if (node->getTruthValue() != UNDET) { return; }
    NuCADSplit(getVarOrder(),chooser,getDim(),node);
}

/*
  Given label, does the "next" refinement step in NuCAD conjunction
  by splitting the cell defined by the label.
 */
void ONuCADObj::refineSubtreeNuCADConjunction(NodeRef node)
{
  SearchQueueRef toRestore = nodeQueue;
  nodeQueue = new SearchQueueObj();
  nodeQueue->push(node);
  fullRefine();
  nodeQueue = toRestore;
}
void ONuCADObj::refineSubtreeNuCADConjunction(const string &label)
{
  refineSubtreeNuCADConjunction(getNode(label));
}

/*
  Given label, does the "next" refinement step in NuCAD conjunction
  by splitting the cell defined by the label.
 */
void ONuCADObj::revertCell(const string &label)
{
  NodeRef node = getNode(label);
  node->revert();
}

  
/*
  Input:
    Builder "D" with label "label" and variable order X, dimension dim and conjunction C.
  Output:
    Q : set of polynomials to add
    tvAtAlpha : the truth value of conjunction C at alpha (D's sample point)
    targetTruthValue : when D is refined by elts of Q, this is the truth value of the refined cell
  NOTE: if Q is empty then targetTruthValue = tvAtAlpha != UNDET
 */
void chooseSplitPoly(VarOrderRef X, BuilderRef D, TAndRef C, int dim,
		     set<IntPolyRef> &Q, int &tvAtAlpha, int &targetTruthValue)
{
  tvAtAlpha = targetTruthValue = UNDET;
  // partition polynomials into those known to be sign-invariant in D, and those whose status is unknown
  // Also, fill map P2A that takes a polynomial and maps it to a vector containing all the atoms in F that
  // have that polynomial as a LHS.
  set<IntPolyRef> knownOrderInvariant, unknown;
  map<IntPolyObj*,vector<TAtomRef> > P2A;
  map<IntPolyObj*,int > P2sign;
  set<IntPolyRef> causeFalse;
  for(TAndObj::conjunct_iterator itr = C->conjuncts.begin(); itr != C->conjuncts.end(); ++itr)
  {
    TAtomRef A = asa<TAtomObj>(*itr);
    if (A.is_null()) { cerr << "NuCADSplitConjunction requires conjunction of atomic formulas." << endl; exit(1); }
    if (A->F->numFactors() != 1) { cerr << "NuCADSplitConjunction requires atoms with irreducible non-constant LHS's." << endl; exit(1); }
    IntPolyRef p = A->F->MultiplicityMap.begin()->first;
    if (A->F->MultiplicityMap[p] != 1) { cerr << "NuCADSplitConjunction requires atoms with irreducible non-constant LHS's." << endl; exit(1); }
    if (D->inPFSet(p))
      knownOrderInvariant.insert(p);
    else
      unknown.insert(p);
    P2A[&(*p)].push_back(A);
    int s = X->partialEval(p,D->getCell()->getAlpha(),X->level(p))->signIfConstant();
    P2sign[&(*p)] = s;
    if (consistentWith(signToSigma(A->F->signOfContent()*s),A->relop) == FALSE) // i.e. "if A is not satisfied at alpha ..."
      causeFalse.insert(p);
  }
  
#ifdef _OC_DEBUG_
cerr << "DEBUG2" << endl;
cerr << "knownOrderInvariant.size() = " << knownOrderInvariant.size() << endl;
cerr << "unknown.size() = " << unknown.size() << endl;
cerr << "causeFalse.size() = " << causeFalse.size() << endl;
#endif

 if (false)
 {
   // Determine "Q", the set of polynomials whose sign-invariance will imply truth-invariance around alpha 
   tvAtAlpha = (causeFalse.size() == 0 ? TRUE : FALSE);
   if (tvAtAlpha == TRUE)
   { // at this point the conjunction is true at alpha, and all polynomials from F must be sign-invariant    
     for(set<IntPolyRef>::iterator itr = unknown.begin(); itr != unknown.end(); ++itr)
       Q.insert(*itr);
     targetTruthValue = TRUE;
   }
   else
   { // at this point the conjunction is false at alpha and as long as one poly from causeFalse is sign-invariant,
     // we have truth-invariance. Check whether any of the causeFalse polys is also known sign-Invariant in D
     bool tinvfalse = false;
     for(set<IntPolyRef>::iterator itr = causeFalse.begin(); !tinvfalse && itr != causeFalse.end(); ++itr)
     {
       tinvfalse = D->inPFSet(*itr);
     }
     if (!tinvfalse)
       Q.insert(*causeFalse.begin()); // HACK!  I SHOULD DO A WHOLE LOT BETTER THAN JUST TAKING THE FIRST!      
     targetTruthValue = FALSE;
   }
 }

 if (true)
 {
   // Determine "Q", the set of polynomials whose sign-invariance will imply truth-invariance around alpha 
   tvAtAlpha = (causeFalse.size() == 0 ? TRUE : FALSE);
   if (tvAtAlpha == TRUE)
   { 
     if (unknown.size() == 0) { targetTruthValue = TRUE; }
     else 
     {  
       Q.insert(*unknown.begin()); // HACK!  I SHOULD DO A WHOLE LOT BETTER THAN JUST TAKING THE FIRST!    
       targetTruthValue = (unknown.size() > 1 ? UNDET : TRUE);
     }
   }
   else
   { // at this point the conjunction is false at alpha and as long as one poly from causeFalse 
     // is sign-invariant, we have truth-invariance    
     // Check whether any of the causeFalse polys is also known sign-Invariant in D
     bool tinvfalse = false;
     for(set<IntPolyRef>::iterator itr = causeFalse.begin(); !tinvfalse && itr != causeFalse.end(); ++itr)
     {
       tinvfalse = D->inPFSet(*itr);
     }
     if (!tinvfalse)
     {
       Q.insert(*causeFalse.begin()); // HACK!  I SHOULD DO A WHOLE LOT BETTER THAN JUST TAKING THE FIRST!
     }
     else
     {
       targetTruthValue = FALSE; // one of the causeFalse polynomials is already known to be sign-invariant in cell
     }
   }
 }

}


void ONuCADObj::NuCADSplit(VarOrderRef X, SplitSetChooserRef chooser, int dim, NodeRef node)
{
  BuilderRef D = node->data;
  PolyManager &PM = *(D->getCell()->getPolyManager());

#ifdef _OC_DEBUG_
cerr << "DEBUG1" << endl;
 cerr << node->getLabel() << " " << D->getCell()->definingFormula() << endl;
PushOutputContext(cerr); OWRITE(D->getCell()->getAlpha()); SWRITE("\n"); PopOutputContext();
cerr << D->projFacInfo() << endl;
#endif

/* DEBUG */
 // if (node->getLabel() == "C4U2U2U3U3U1U4L3L3U4U1L2L")
 // {
 //   cerr << "In split call of parent that causes trouble!" << endl;
 //   cerr << D->projFacInfo() << endl;
 // }

 // Determine "Q", the set of polynomials whose sign-invariance will imply truth-invariance around alpha
 // and "tvAtAlpha", the truth value of formula C at alpha.
 set<IntPolyRef> Q;
 int tvAtAlpha = UNDET, targetTruthValue = UNDET;
 chooser->chooseSplit(X,node,dim,Q,tvAtAlpha,targetTruthValue);
 if (Q.size() == 0)
 {
   node->truthValue = targetTruthValue;
   if (node->splitLabel == 'Y') node->splitLabel = 'X';
   return;
 }

#ifdef _OC_DEBUG_
cerr << "DEBUG3" << endl;
#endif

  // Merge Q into D - I'm not finding a terribly clever way to do this I fear!  
  BuilderRef DX = new BuilderObj(new OpenCellObj(X,D->getCell()->getAlpha(),dim));
  BuilderObj::mergeBintoA(DX,D,dim);
  for(set<IntPolyRef>::iterator itr = Q.begin(); !DX->inErrorState() && itr != Q.end(); ++itr)
  {
#ifdef _OC_DEBUG_
    cerr << "Adding "; PushOutputContext(cerr); (*itr)->write(PM); PopOutputContext(); cerr << endl;
#endif
    FactRef f = makeFactor(PM,*itr);
    DX->addRec(f);
  } 

  if (DX->inErrorState()) 
  { 
    dealWithFail(X,D,DX,dim);
    NuCADSplit(X, chooser, dim, node);
    return;
  }

#ifdef _OC_DEBUG_
  cerr << "DEBUG4" << endl;
#endif

  int maxk = 0;
  for(int k = 1; k <= dim; ++k)
  {
    vector<BuilderRef> W;
    BuilderObj::splitAtLevelK(D,DX,dim,k,W);
    if (!W[0].is_null()) { 
      NodeRef tmp = new NodeObj(&*node,W[0],UNDET,k,'L');
      node->Lchild.push_back(tmp);
      nodeQueue->push(tmp);
      maxk = k;
    }
    if (!W[1].is_null()) { 
      NodeRef tmp = new NodeObj(&*node,W[1],UNDET,k,'U');
      node->Uchild.push_back(tmp);
      nodeQueue->push(tmp);
      maxk = k;
    }
  }
  
  // If we have actually split, add DX as child and push, otherwise just replace existing cell in node
  if (maxk == 0)
  {
    node->data = DX;
    if (targetTruthValue == UNDET)
      nodeQueue->push(node);
    else
    {
      node->truthValue = targetTruthValue;
      if (node->splitLabel == 'Y') node->splitLabel = 'X';
    }
  }
  else
    nodeQueue->push(node->XYchild = new NodeObj(&*node,DX,targetTruthValue,maxk,(targetTruthValue == UNDET ? 'Y' : 'X')));

#ifdef _OC_DEBUG_
  cerr << "DEBUG5" << endl;  
#endif
}

/*
extern void newChooseSplitPoly(VarOrderRef X, BuilderRef D, TAndRef C, int dim,
			       set<IntPolyRef> &Q, int &tvAtAlpha, int &targetTruthValue);

void ONuCADObj::NuCADSplitConjunction(VarOrderRef X, TAndRef C, int dim, NodeRef node)
{
  BuilderRef D = node->data;
  PolyManager &PM = *(D->getCell()->getPolyManager());

#ifdef _OC_DEBUG_
cerr << "DEBUG1" << endl;
 cerr << node->getLabel() << " " << D->getCell()->definingFormula() << endl;
PushOutputContext(cerr); OWRITE(D->getCell()->getAlpha()); SWRITE("\n"); PopOutputContext();
cerr << D->projFacInfo() << endl;
#endif

 // Determine "Q", the set of polynomials whose sign-invariance will imply truth-invariance around alpha
 // and "tvAtAlpha", the truth value of formula C at alpha.
 set<IntPolyRef> Q;
 int tvAtAlpha = UNDET, targetTruthValue = UNDET;
 // chooseSplitPoly(X,D,C,dim,Q,tvAtAlpha,targetTruthValue);
 newChooseSplitPoly(X,D,C,dim,Q,tvAtAlpha,targetTruthValue);
 if (Q.size() == 0)
 {
   //nodeQueue.push(node->XYchild = new NodeObj(&*node,D,tvAtAlpha,dim,'X'));
   node->truthValue = targetTruthValue;
   if (node->splitLabel == 'Y') node->splitLabel = 'X';
   return;
 }

#ifdef _OC_DEBUG_
cerr << "DEBUG3" << endl;
#endif

  // Merge Q into D - I'm not finding a terribly clever way to do this I fear!  
  BuilderRef DX = new BuilderObj(new OpenCellObj(X,D->getCell()->getAlpha(),dim));
  BuilderObj::mergeBintoA(DX,D,dim);
  for(set<IntPolyRef>::iterator itr = Q.begin(); !DX->inErrorState() && itr != Q.end(); ++itr)
  {
#ifdef _OC_DEBUG_
    cerr << "Adding "; PushOutputContext(cerr); (*itr)->write(PM); PopOutputContext(); cerr << endl;
#endif
    FactRef f = makeFactor(PM,*itr);
    DX->addRec(f);
  } 

  if (DX->inErrorState()) 
  { 
    dealWithFail(X,D,DX,dim);
    NuCADSplitConjunction(X, C, dim, node);
    return;
  }

#ifdef _OC_DEBUG_
  cerr << "DEBUG4" << endl;
#endif

  int maxk = 0;
  for(int k = 1; k <= dim; ++k)
  {
    vector<BuilderRef> W;
    BuilderObj::splitAtLevelK(D,DX,dim,k,W);
    if (!W[0].is_null()) { 
      NodeRef tmp = new NodeObj(&*node,W[0],UNDET,k,'L');
      node->Lchild.push_back(tmp);
      nodeQueue->push(tmp);
      maxk = k;
    }
    if (!W[1].is_null()) { 
      NodeRef tmp = new NodeObj(&*node,W[1],UNDET,k,'U');
      node->Uchild.push_back(tmp);
      nodeQueue->push(tmp);
      maxk = k;
    }
  }
  
  // If we have actually split, add DX as child and push, otherwise just replace existing cell in node
  if (maxk == 0)
  {
    node->data = DX;
    if (targetTruthValue == UNDET)
      nodeQueue->push(node);
    else
    {
      node->truthValue = targetTruthValue;
      if (node->splitLabel == 'Y') node->splitLabel = 'X';
    }
  }
  else
    nodeQueue->push(node->XYchild = new NodeObj(&*node,DX,targetTruthValue,maxk,(targetTruthValue == UNDET ? 'Y' : 'X')));

#ifdef _OC_DEBUG_
  cerr << "DEBUG5" << endl;  
#endif
}
*/

/********************************************************************
 * Inputs:
 *   D  : the cell we are trying to merge into DX (must not be in FAIL state)
 *   DX : the cell in FAIL state from trying to merge D into it
 *        call its fail polynomial f and the level of f i_f
 * Side Effects:
 *   The sample point of D is moved to point alpha' (still in D, of course) 
 *   s.t. for some i, 0 < i <= i_f and f(alpha_1,...,_alpha_i,x_{i+1},...,x_{i_f}) 
 *   is the zero polynomial, we have:  (actually, i is guaranteed to be minimal)
 *   1. the first i-1 coordinates of alpha and alpha' are identical, 
 *   2. alpha'_i < alpha_i
 *   2. f(alpha'_1,...,_alpha'_i,x_{i+1},...,x_{i_f}) is not the zero polynomial,
 ********************************************************************/
void ONuCADObj::dealWithFail(VarOrderRef X, BuilderRef D, BuilderRef DX, int dim)
{
  PolyManager &PM = *(D->getCell()->getPolyManager());

  // Get polynomial f and level i that cause the fail
  IntPolyRef f = DX->getErrorPoly();
  int i_f = X->level(f);
#ifdef _OC_DEBUG_
  cerr << "Error!" << endl;
  cout << "Fail caused by ";
  f->write(PM);
  SWRITE("\n");
  X->write();
  cout << " = ";
  DX->getCell()->writeAlpha();
  SWRITE("\n");
#endif
    
  // Perturb!
  // STEP 1: Find level i <= i_f and set F of i-level polys s.t. if gamma \in R^i such that
  //         none of the elements of F are zero at gamma then f is not identically zero when
  //         (partially) evaluated at gamma.
  int i = i_f;
  vector<IntPolyRef> F;
  F.push_back(f);
  do {
#ifdef _OC_DEBUG_
    cerr << "level i = " << i << endl;
#endif
    bool nullf = false; 
    for(int j = 0; j < F.size() && !nullf; ++j) 
    { nullf = nullf ||  X->isNullified(F[j],D->getCell()->getAlpha(),i); }
    if (!nullf) break;
    int N_F = F.size();
    for(int j = 0; j < N_F; ++j)
    {
      if (X->level(F[j]) == i)
      {
#ifdef _OC_DEBUG_
	cerr << "processing ... "; F[j]->write(PM); cout << endl;
#endif
	FactRef ld = PM.ldcfFactors(F[j],(*X)[i]);
	for(map<IntPolyRef,int>::iterator itr = ld->MultiplicityMap.begin(); itr != ld->MultiplicityMap.end(); ++itr)
	  F.push_back(itr->first);
      }
    }
    --i;
  } while(i > 1);
      
#ifdef _OC_DEBUG_
  cerr << "Decided on i = " << i << ", perturbing based on ... " << endl;
  for(int j = 0; j < F.size(); ++j)
    if (X->level(F[j]) == i) { F[j]->write(PM); cout << endl; }
#endif

  // STEP 2: Find zeta s.t. no element of F is zero at (gamma_1,...,gamma_{i-1},rho) for any rho \in (zeta,gamma_i).
  RealAlgNumRef alpha_i = rationalToRealAlgNum(LELTI(D->getCell()->getAlpha(),i));
  RealAlgNumRef zeta = D->getCell()->get(i)->a;
  for(int j = 0; j < F.size(); ++j)
  {
    if (X->level(F[j]) != i) continue;
    IntPolyRef g = X->partialEval(F[j],D->getCell()->getAlpha(),i-1);
    vector<RealRootIUPRef> Z = RealRootIsolateRobust(g);
    int m = Z.size()-1; while(m >= 0 && alpha_i->compareToRobust(Z[m]) <= 0) { --m; }
    if (m >= 0) { zeta = max(zeta,Z[m]); }
  }

  // STEP 3: choose new alpha_i from interval (seta,alpha_i <-- old)
  Word alpha_i_new = rationalPointInInterval(zeta,alpha_i);
  
  // STEP 4: complete (alpha_1,...,alpha_i) to get new alpha and make it D's model point.
  //         The point of this new alpha is that there are infinitely many choices of it for 
  //         which f is not zero
  Word alpha_new = NIL;
  Word t = D->getCell()->getAlpha();
  for(int j = 0; j < i-1; ++j, t = RED(t))
    alpha_new = COMP(FIRST(t),alpha_new);
  alpha_new = CINV(COMP(alpha_i_new,alpha_new));
  Word alpha_new_final = D->getCell()->moveSamplePoint(alpha_new);
  
#ifdef _OC_DEBUG_
  cout << "Fail caused by ";
  f->write(PM);
  SWRITE(" --- ");
  X->write();
  cout << " = ";
  DX->getCell()->writeAlpha();
  SWRITE(" -- resolved to -- ");
  D->getCell()->writeAlpha();
  SWRITE("\n");
#endif
}

string ONuCADObj::toString(int mask)
{
  nx = nt = nf = 0;
  ostringstream sout;
  PushOutputContext(sout);
  
  // Print cells
  stack<NodeRef> S;
  S.push(root);
  while(!S.empty())
  {
    NodeRef n = S.top(); S.pop();
    if (!n->hasChildren()) { nx++; if (n->truthValue == TRUE) nt++; if (n->truthValue == FALSE) nf++; }
    if ((mask & m_acells) || ((mask & m_tcells) && n->truthValue == TRUE) || ((mask & m_fcells) && n->truthValue == FALSE))
    {
      sout << "Cell " << n->getLabel() 
	   << " " << (n->truthValue == TRUE ? "TRUE" : (n->truthValue == FALSE ? "FALSE" : "UNDET")) << " ";
      n->data->getCell()->writeAlpha();
      sout << " " << n->data->getCell()->definingFormula();
      sout << endl;
    }    
    for(int i = 0; i < n->Lchild.size(); i++)
      S.push(n->Lchild[i]);
    if (!n->XYchild.is_null()) S.push(n->XYchild);
    for(int i = 0; i < n->Uchild.size(); i++)
      S.push(n->Uchild[i]);
  }

  /*  
  // Calculate total proj-fac information
  vector< set<IntPolyRef,PolyManager::PolyLT> > pftotal(dim + 1);
  vector<int> maxsizes(dim+1);
  vector<int> maxsizesIndx(dim+1);
  for(int i = 0; i < V.size(); i++)
  {
    for(set<IntPolyRef,PolyManager::PolyLT>::iterator itr = V[i]->pfbegin(); itr != V[i]->pfend(); itr++)
    {
      pftotal[X->level(*itr)].insert(*itr);
    }
    vector< set<IntPolyRef,PolyManager::PolyLT> > pfbl = V[i]->projFacByLevel();
    for(int ll = 1; ll <= dim; ll++)
    {
      int x = pfbl[ll].size();
      if (x > maxsizes[ll]) { maxsizes[ll] = x; maxsizesIndx[ll] = i; }
    }
  }
  
  if (mask & m_pfsumm)
  {
    for(int i = 1; i < dim+1; i++)
    {
      sout << "Num level " << i << " factors: " << pftotal[i].size() << endl;
    }
    for(int l = 1; l <= dim; l++)
      sout << "max single cell level " << l << ": " << maxsizes[l] << " - came from cell " << maxsizesIndx[l] << endl;
    
    // compute sum of print lengths
    unsigned long long LN = 0;
    for(int i = 1; i <= dim; ++i)
      for(set<IntPolyRef,PolyManager::PolyLT>::iterator itr = pftotal[i].begin(); itr != pftotal[i].end(); ++itr)
	LN += X->getPolyManager()->polyToStr(*itr).length();
    sout << "sum of print lengths =" << LN << endl;
  }
  
  if  (mask & m_nxcells) { sout << "Num X cells = " << nx << endl; }
  if  (mask & m_ntcells) { sout << "Num TRUE X cells = " << nt << endl; }
  if  (mask & m_nfcells) { sout << "Num FALSE X cells = " << nf << endl; }
  */
    PopOutputContext();
    return sout.str();
}

NodeRef ONuCADObj::getNode(const string &label)
{
  int i = 0, state = 0; // 0 <- start, 1 <- ready to read X/Y, or <index<(L|U), 2 <-- one or more digits read
  int index = -1;
  NodeRef curr = root;
  while(i < label.length())
  {
    if (state == 0) { if (label[i] == 'C') { state = 1; } else { throw TarskiException("Invalid label!"); } }
    else if (state == 1)
    {
      if (label[i] == 'X' || label[i] == 'Y') { curr = curr->XYchild; }
      else if (isdigit(label[i])) { index = label[i] - '0'; state = 2; }
      else { throw TarskiException(string("Unexpected character '") + label[i] + "' in label"); }
    }
    else if (state == 2)
    {
      if (isdigit(label[i])) { index = 10*index + (label[i] - '0'); }
      if (label[i] == 'L' || label[i] == 'U') { 
	state = 1;
	vector<NodeRef> &childList = label[i] == 'L' ? curr->Lchild : curr->Uchild;
	int i = 0;
	for(i = 0; i < childList.size() && childList[i]->splitLevel != index; i++);
	if (i == childList.size()) throw TarskiException("Label '" + label + "' not found!");
	else { curr = childList[i]; }
      }
    }
    i++;
  }
  return curr;
}

void ONuCADObj::writeDot(const string &label, std::ostream& out)
{
  NodeRef start = getNode(label);
  writeDot(start,out);
}

void ONuCADObj::writeDot(NodeRef start, std::ostream& out)
{
  queue<NodeRef> Q;
  Q.push(start);

  out << "digraph G {" << endl;
  
  while(!Q.empty())
  {
    NodeRef n = Q.front(); Q.pop();
    string nlab = n->getLabel();
    out << nlab << " [label=\"" << n->splitLevel << n->splitLabel << 
      (n->truthValue == TRUE ? " T" : (n->truthValue == FALSE ? " F" : "")) <<
      "\"];" << endl;
    for(int i = 0; i < n->Lchild.size(); i++)
    { 
      NodeRef t = n->Lchild[i];
      out << nlab << " -> " << t->getLabel() << ";" << endl;
      Q.push(t);
    }
    if (n->hasChildren()) 
    { 
      out << nlab << " -> " << n->XYchild->getLabel() << ";" << endl; Q.push(n->XYchild); 
    }    
    for(int i = 0; i < n->Uchild.size(); i++)
    { 
      NodeRef t = n->Uchild[i];
      out << nlab << " -> " << t->getLabel() << ";" << endl;
      Q.push(t);
    }
  }

  out << "}" << endl;
}


// This only exists for debugging purposes.  It checks that the OpenNuCAD
// data-structure subtree rooted at P satisfies the requirement that whenever
// a Node has children, it has both an XYchild and at least one L/U child,
// and the splitLevel of the XYchild is the same as the maximum splitLevel of
// any L/U child.
void checkIt(NodeRef P)
{
  if (P->hasChildren())
  {
    // recursive check
    checkIt(P->XYchild);
    for(int i = 0; i < P->Lchild.size(); i++)
      checkIt(P->Lchild[i]);
    for(int i = 0; i < P->Uchild.size(); i++)
      checkIt(P->Uchild[i]);

    // check P itself
    int testX = P->XYchild->splitLevel;
    int testC = std::max(P->Lchild.empty() ? -1 : P->Lchild.back()->splitLevel,
			 P->Uchild.empty() ? -1 : P->Uchild.back()->splitLevel);
    if (testX != testC)
    {
      throw TarskiException("XYchild splitLevel is wrong! [ checking cell " + P->getLabel() + " ]");
    }
  }
}


void ONuCADObj::mergeLow(NodeRef P, int k, int truthValue)
{
  // setup
  NodeRef X = P->XYchild;
  if (X.is_null() || X->splitLevel != k) { throw TarskiException("In mergeLow: Invalid input!"); }
  int nL = P->Lchild.size(), nU = P->Uchild.size();
  NodeRef L; if (nL > 0) L = P->Lchild.back();
  NodeRef U; if (nU > 0) U = P->Uchild.back();
  if (L->splitLevel > k) { throw TarskiException("In mergeLow: Invalid input! L-child level too high."); }
  if (L.is_null() || L->splitLevel < k) return;
  if (!U.is_null() && U->splitLevel > k) { throw TarskiException("In mergeLow: Invalid input! U-child level too high."); }

  // compute the new set of sign-invariant projection factors, which is the intersection of X and L pfsets
  IntPolyRef sbp = X->getData()->getCell()->get(k)->getBoundDefPoly(-1);
  set<IntPolyRef,PolyManager::PolyLT> newpfset;
  set<IntPolyRef,PolyManager::PolyLT>::iterator iX = X->getData()->pfbegin();
  set<IntPolyRef,PolyManager::PolyLT>::iterator iL = L->getData()->pfbegin();
  std::set_intersection(X->getData()->pfbegin(),X->getData()->pfend(),L->getData()->pfbegin(),L->getData()->pfend(),
			std::inserter(newpfset, newpfset.end()));
  newpfset.erase(sbp);
 
  // if the cells being merged are the only children, P w/o children is the new node
  if (nL == 1 && nU == 0) 
  {
    P->Lchild.pop_back(); P->XYchild = NULL; P->truthValue = truthValue; P->getData()->swapPFSet(newpfset); return; 
  }
    
  // check that X and L have sample points that are identical in coordinate 1,...,K-1
  int i = 1;
  {
    Word alphaX = X->getData()->getCell()->getAlpha();
    Word alphaL = L->getData()->getCell()->getAlpha();
    while(i < k && EQUAL(FIRST(alphaX),FIRST(alphaL))) { i++; alphaX = RED(alphaX); alphaL = RED(alphaL); }
  }
  if (i != k)
  {
    // cerr << "Moving sample point in mergeLow" << endl;
    Word alphaX = X->getData()->getCell()->getAlpha();
    // In this case, we have to move one of the sample points so that they agree
    Word ppx = NIL, tmp = alphaX; for(int i = 1; i < k; i++) { ppx = COMP(FIRST(tmp),ppx); tmp = RED(tmp); } ppx = CINV(ppx);
    L->getData()->getCell()->moveSamplePoint(ppx);
  }

  // Modify X to become the new cell
  X->getData()->getCell()->get(k)->expand(L->getData()->getCell()->get(k),-1);
  X->truthValue = truthValue;
  X->getData()->swapPFSet(newpfset);
  P->Lchild.pop_back();
  if (U.is_null() || U->splitLevel < k)
  {
    int j = std::max( U.is_null() ? 0 : U->splitLevel , P->Lchild.empty() ? 0 : P->Lchild.back()->splitLevel );
    X->splitLevel = j;
  }

#ifdef _OC_DEBUG2_
  checkIt(P); // should leave P in a correct state
#endif
}

void ONuCADObj::mergeHigh(NodeRef P, int k, int truthValue)
{
  // setup
  NodeRef X = P->XYchild;
  if (X.is_null() || X->splitLevel != k) { throw TarskiException("In mergeHigh: Invalid input!"); }
  int nL = P->Lchild.size(), nU = P->Uchild.size();
  NodeRef L; if (nL > 0) L = P->Lchild.back();
  NodeRef U; if (nU > 0) U = P->Uchild.back();
  if (U->splitLevel > k) { throw TarskiException("In mergeHigh: Invalid input! U-child level too high."); }
  if (U.is_null() || U->splitLevel < k) return;
  if (!L.is_null() && L->splitLevel > k) { throw TarskiException("In mergeHigh: Invalid input! L-child level too high."); }
  
  // compute the new set of sign-invariant projection factors, which is 
  // the intersection of X and U pfsets, subtracing out the boundary poly they share
  IntPolyRef sbp = X->getData()->getCell()->get(k)->getBoundDefPoly(+1);
  set<IntPolyRef,PolyManager::PolyLT> newpfset;
  set<IntPolyRef,PolyManager::PolyLT>::iterator iX = X->getData()->pfbegin();
  set<IntPolyRef,PolyManager::PolyLT>::iterator iU = U->getData()->pfbegin();
  std::set_intersection(X->getData()->pfbegin(),X->getData()->pfend(),U->getData()->pfbegin(),U->getData()->pfend(),
			std::inserter(newpfset, newpfset.end()));
  newpfset.erase(sbp);

  // if the cells being merged are the only children, P w/o children is the new node
  if (nU == 1 && nL == 0) 
  { 
    P->Uchild.pop_back(); P->XYchild = NULL; P->truthValue = truthValue; P->getData()->swapPFSet(newpfset); return; 
  }
    
  // check that X and U have sample points that are identical in coordinate 1,...,K-1
  int i = 1;
  {
    Word alphaX = X->getData()->getCell()->getAlpha();
    Word alphaU = U->getData()->getCell()->getAlpha();
    while(i < k && EQUAL(FIRST(alphaX),FIRST(alphaU))) { i++; alphaX = RED(alphaX); alphaU = RED(alphaU); }
  }
  if (i != k)
  {
    //cerr << "Moving sample point in mergeHigh" << endl;
    Word alphaX = X->getData()->getCell()->getAlpha();
    // In this case, we have to move one of the sample points so that they agree
    Word ppx = NIL, tmp = alphaX; for(int i = 1; i < k; i++) { ppx = COMP(FIRST(tmp),ppx); tmp = RED(tmp); } ppx = CINV(ppx);
    U->getData()->getCell()->moveSamplePoint(ppx);
  }
  
  // Modify X to become the new cell
  X->getData()->getCell()->get(k)->expand(U->getData()->getCell()->get(k),+1);
  X->truthValue = truthValue;
  X->getData()->swapPFSet(newpfset);
  P->Uchild.pop_back();
  if (L.is_null() || L->splitLevel < k)
  {
    int j = std::max( L.is_null() ? 0 : L->splitLevel , P->Uchild.empty() ? 0 : P->Uchild.back()->splitLevel );
    X->splitLevel = j;
  }

#ifdef _OC_DEBUG2_
  checkIt(P); // should leave P in a correct state
#endif
}

// returns true iff the split levels of every descendent of P is less than m
bool splitLevelsOfAllDescendentsLessThan(int m, NodeRef P)
{
  if (!P->hasChildren()) return true;
  if (P->XYchild->splitLevel >= m || 
      (P->Lchild.size() > 0 && P->Lchild.back()->splitLevel >= m) ||
      (P->Uchild.size() > 0 && P->Uchild.back()->splitLevel >= m ))
    return false;
  if (!splitLevelsOfAllDescendentsLessThan(m,P->XYchild)) return false;
  for(int i = 0; i < P->Lchild.size(); i++)
    if (!splitLevelsOfAllDescendentsLessThan(m,P->Lchild[i])) return false;
  for(int i = 0; i < P->Uchild.size(); i++)
    if (!splitLevelsOfAllDescendentsLessThan(m,P->Uchild[i])) return false;
  return true;
}

const int NOEXIST = (TRUE | FALSE | UNDET) << 1; // hopefully this gives me something different from TRUE, FALSE and UNDET!

// Sets the m-level CellBound for parent and all descendents to b
void setCellBoundForCellAndAllDescendents(NodeRef parent, int m, CellBoundRef b)
{
  stack<NodeRef> S;
  S.push(parent);
  while(!S.empty())
  {
    NodeRef n = S.top(); S.pop();
    n->data->getCell()->setBound(m,b);
    if (!n->hasChildren()) continue;
    S.push(n->XYchild);
    for(int i = 0; i < n->Lchild.size(); i++) S.push(n->Lchild[i]);
    for(int i = 0; i < n->Uchild.size(); i++) S.push(n->Uchild[i]);
  }    
}

// pushXYChildDownOneLevel(parent,child,m)
// This function replaces the decomposition of level >= m of the parent by an enlargement of
// The decomposition defined by child.  This makes sense because of the requirements on
// parent/child/m.
// m is the current splitLevel of child
// child is a child of parent, with the requirements:
// 1. no descendent of child has split-level >= m
// 2. parent has at least one L/U child of level < m
void pushXYChildDownOneLevel(NodeRef parent, NodeRef child, int m)
{
  // make child XYchild of parent, and remove from parent all L/U children of level>= m
  parent->XYchild = child;
  while(!parent->Lchild.empty() && parent->Lchild.back()->splitLevel >= m)
    parent->Lchild.pop_back();
  while(!parent->Uchild.empty() && parent->Uchild.back()->splitLevel >= m)
    parent->Uchild.pop_back();

  // Determine proper split level for new parent->XYchild.
  int newm = std::max(parent->Lchild.empty() ? -1 : parent->Lchild.back()->splitLevel,
		 parent->Uchild.empty() ? -1 : parent->Uchild.back()->splitLevel);
  child->splitLevel = newm;
  child->splitLabel = child->hasChildren() ? 'Y' : 'X';

  // set the CB[m] of all descendents of parent to the parent's CB[m] because we
  // are enlarging child to cover the entirety of what XY/L/U covered before
  OpenCellRef c = parent->data->getCell();
  CellBoundRef b = c->get(m);
  setCellBoundForCellAndAllDescendents(child,m,b);
}

// pushDown(parent,child,m)
// Part of existenial propagation.
// basically replaces the parent node with the child node.
// care needs to be taken to only do this when it will 
// produce correct results.  NOTE: parent takes on truthValue of child!
// TODO: put in proper specs!
void pushDown(NodeRef parent, NodeRef child, int m)
{
  NodeObj* pp = child->parent;
  parent->XYchild = child->XYchild; 
  swap(parent->Lchild,child->Lchild); 
  swap(parent->Uchild,child->Uchild); 
  parent->truthValue = child->truthValue;
  
  // If the parent it its new configuration has any children, their "parent" pointer needs to be set to parent
  if (parent->hasChildren())
  {
    parent->XYchild->parent = pp;
    for(int i = 0; i < parent->Lchild.size(); i++)
      parent->Lchild[i]->parent = pp;
    for(int i = 0; i < parent->Uchild.size(); i++)
      parent->Uchild[i]->parent = pp;
  }

  // set the CB[m] of all descendents of parent to the parent's CB[m]
  OpenCellRef c = parent->data->getCell();
  CellBoundRef b = c->get(m);
  setCellBoundForCellAndAllDescendents(parent,m,b);

#ifdef _OC_DEBUG2_
  checkIt(parent); // check that this modified "parent" is in a correct state
#endif
}

void ONuCADObj::expropDownTo(NodeRef P, int k, bool strictFlag)
{
#ifdef _OC_DEBUG2_
  checkIt(P); // check that P is in a correct state before we start
#endif

  /* DEBUG */
  // if (P->getLabel() == "CY2U2U3U3U1UY3L3U")
  //   cerr << "base break!" << endl;

  if (!P->hasChildren()) return;
  expropDownTo(P->XYchild,k,strictFlag);
  int m = P->XYchild->splitLevel;
  int iL = P->Lchild.size() - 1;
  int iU = P->Uchild.size() - 1;

  // Also, I should test the merging code to see how it works when a child isn't a leaf node.
  int tX = P->XYchild->truthValue;
  for(;m > 0;m--)
  {
    bool inProjectionCase = (m > k);
    if (!P->hasChildren()) return;
    if (P->XYchild->splitLevel < m) continue;

    int tL = NOEXIST;
    NodeRef L; if (iL >= 0) L = P->Lchild[iL];
    if (!L.is_null() && L->splitLevel == m) { expropDownTo(L,k,strictFlag); tL = L->truthValue; iL--; } else { L = NULL; }

    int tU = NOEXIST;
    NodeRef U; if (iU >= 0) U = P->Uchild[iU];
    if (!U.is_null() && U->splitLevel == m) { expropDownTo(U,k,strictFlag); tU = U->truthValue; iU--; } else { U = NULL; }

    if (tL == NOEXIST && tU == NOEXIST) continue;
    
    if (!inProjectionCase)
    {
      if (tX != UNDET && !strictFlag)
      {
	int newtX = tX;
	if (!L.is_null()) { if (tX == tL) { mergeLow(P,m,tX); } else { newtX = UNDET; } }  checkIt(P);
	if (!U.is_null()) { if (tX == tU) { mergeHigh(P,m,tX); } else { newtX = UNDET; } }  checkIt(P);
	tX = newtX;
      }
    }
    else if (tL == TRUE || tX == TRUE || tU == TRUE)
    {
      NodeRef theOne = tL == TRUE ? L : (tU == TRUE ? U : P->XYchild);
      if (iL < 0 && iU < 0)
      {
	pushDown(P,theOne,m);  // This means "theOne" basically expands to become P
	return;
      }
      else
	pushXYChildDownOneLevel(P,theOne,m);
      tX = TRUE;
    }
#ifdef  ISSACPAPERPLOT
    else if (tL == UNDET && tX == FALSE && tU == UNDET)
    {
      CellBoundRef bU = P->Uchild[iU+1]->getData()->getCell()->get(m);
      CellBoundRef bX = P->XYchild->getData()->getCell()->get(m);
      CellBoundRef bN = bX->clone();
      bN->expand(bU,+1);
      setCellBoundForCellAndAllDescendents(P->Uchild[iU+1],m,bN);
      P->XYchild = P->Uchild[iU+1];
      P->XYchild->splitLabel = 'X';
      P->Uchild.pop_back();
      return;
    }
#endif
    else
    {
      if (tL == FALSE && tX == FALSE)
	mergeLow(P,m,FALSE);
      if (tX == FALSE && tU == FALSE)
	mergeHigh(P,m,FALSE);    
      if (tL != UNDET && tX != UNDET && tU != UNDET)
	tX = FALSE; // in this case the above merged all three into a single FALSE cell
      else if (tL == UNDET && tX != UNDET && tU != UNDET && splitLevelsOfAllDescendentsLessThan(m,L))
      {
	NodeRef XY = P->XYchild;
	if (iL < 0 && iU < 0 && XY->splitLevel == m) // means P only has children mL,mX,mU
	{
	  pushDown(P,L,m);  
	  checkIt(P); 
	  return; 
	}
	else
	{
	  // L becomes the XYchild, and moves down one or more levels (since it's now the ONLY child at level m)
	  // NOTE: the fact that tX is not UNDET means there are no higher-level L or U children
	  pushXYChildDownOneLevel(P,L,m);
	  tX = UNDET;
	}
      }
      else if (tL != UNDET && tX == UNDET && tU != UNDET && splitLevelsOfAllDescendentsLessThan(m,P->XYchild))
      {
	NodeRef XY = P->XYchild;
	if (iL < 0 && iU < 0 && XY->splitLevel == m) // means P only has children mL,mX,mU
	{ pushDown(P,XY,m); return; }
	else if (XY->splitLevel == m) // means P only has children at level m and lower
	{
	  pushXYChildDownOneLevel(P,XY,m); // NOTE: tX is already UNDET
	}
	else
	  ; // in this case there may be higher-level L & U children, so we can't do anything.
      }
      else if (tL != UNDET && tX != UNDET && tU == UNDET && splitLevelsOfAllDescendentsLessThan(m,U))
      {
	NodeRef XY = P->XYchild;
	if (iL < 0 && iU < 0 && XY->splitLevel == m) // means P only has children mL,mX,mU
	{ pushDown(P,U,m); return; }
	else 
	{
	  // U becomes the XYchild, and moves down one level (since it's now the ONLY child at level m)
	  // NOTE: the fact that tX is not UNDET means there are no higher-level L or U children
	  pushXYChildDownOneLevel(P,U,m);
	  tX = UNDET;
	}
      }
      else
	tX = UNDET;
    }
  }
}


// truth value TRUE, FALSE or UNDET - UNDET means not truth-invariant
// number of cells in sub-tree
pair<int,int> tissHelper(NodeRef n)
{  
  if (!n->hasChildren()) { return pair<int,int>(n->getTruthValue(),1); }
  int nt = 0, nf = 0, nu = 0, count = 0;
  vector< pair<int,int> > V;
  vector< NodeRef > W;
  NodeObj::ChildIterator itr = n->childBegin();
  while(itr.hasNext())
  {
    NodeRef c = itr.next();
    V.push_back(tissHelper(c));
    W.push_back(c);
    int t = V.back().first;
    if (t == TRUE) ++nt; else if (t == FALSE) ++nf; else ++nu;
    count += V.back().second;
  }
  if (nu > 0 || (nt > 0 && nf > 0))
  {
    for(int i = 0; i < V.size(); i++)
      if (V[i].first != UNDET && V[i].second > 1)
	cout << W[i]->getLabel() << " " << (V[i].first == TRUE ? "TRUE" : "FALSE") << " " << V[i].second << endl;
    return pair<int,int>(UNDET,count);
  }
  else
  {
    return pair<int,int>(nt > 0 ? TRUE : FALSE,count);
  }
}

void ONuCADObj::truthInvariantSubtreeStats()
{
  tissHelper(root);
}

void ONuCADObj::negateTruthValues()
{
  LeafIterator itr = this->iterator(); 
  while(itr.hasNext())
  {
    NodeRef n = itr.next();
    if (n->truthValue == TRUE) n->truthValue = FALSE;
    else if (n->truthValue == FALSE) n->truthValue = TRUE;
  }
}


IntPolyRef SplitSetChooserObj::chooseNextPoly(set<IntPolyRef> &S, VarOrderRef X, NodeRef node)
{
  
  // cout << endl << "Order is "; X->write(); cout << " choosing between ... ";
  // for(auto itr = S.begin(); itr != S.end(); ++itr) {
  //   (*itr)->write(*X->getPolyManager()); cout << ", ";
  // }
  // cout << endl;
  // node->getData()->getCell()->debugPrint(cout);
  int N = S.size();
  node->numSplitOptions = N;
    
  if (N == 0) throw TarskiException("Set S empty in chooseNextPoly!");
  IntPolyRef choice;
  if (N == 1)
    choice = *S.begin();
  else {  
    const int CHOOSE_FIRST = 0, CHOOSE_BPC_MIN = 1, CHOOSE_RANDOM = 2;
    switch(CHOOSE_BPC_MIN)
    {
    case CHOOSE_FIRST: { choice = *(S.begin()); } break;
    case CHOOSE_BPC_MIN: { // This uses the BasicPolyCompare lt method to make the choice
      BasicPolyCompare BPC(X);
      IntPolyRef smallest;
      for(set<IntPolyRef>::iterator itr = S.begin(); itr != S.end(); ++itr)
	if (smallest.is_null() || BPC.lt(*itr,smallest))
	  smallest = *itr;
      choice = smallest;
    }break;
    case CHOOSE_RANDOM: {
      int n = S.size();
      int k = rand()%S.size();
      set<IntPolyRef>::iterator itr = S.begin();
      for(int i = 0; i < k; ++itr, ++i);
      choice = *itr;
    } break;
    default: { throw TarskiException("In chooseNextPoly: unkown criterion!"); } break;
    }
  }
  
  return choice;
}

/*
  NEW!  new choose split polynomial
  This version allows for multi-factor atoms, and it tries to make a moderately intelligent choice
  Input:
    Builder "D" with label "label" and variable order X, dimension dim and conjunction C.
  Output:
    Q : set of polynomials to add
    tvAtAlpha : the truth value of conjunction C at alpha (D's sample point)
    targetTruthValue : when D is refined by elts of Q, this is the truth value of the refined cell
  NOTE: if Q is empty then targetTruthValue = tvAtAlpha != UNDET
 */
void SplitSetChooserConjunction::chooseSplit(VarOrderRef X, NodeRef node, int dim,
		     set<IntPolyRef> &Q, int &tvAtAlpha, int &targetTruthValue)
{
  BuilderRef D = node->getData();
  GCWord alpha = D->getCell()->getAlpha();

  // collect all factors appearing in C and determine their signs & whether or not they are sign invariant
  map<IntPolyObj*,int > P2sign; 
  map<IntPolyObj*,bool > P2signInvariant; 
  for(TAndObj::conjunct_iterator itr = C->conjuncts.begin(); itr != C->conjuncts.end(); ++itr)
  {
    TAtomRef A = asa<TAtomObj>(*itr);
    TExtAtomRef B = asa<TExtAtomObj>(*itr);
    if (A.is_null() && B.is_null()) { throw TarskiException("NuCADSplitConjunction requires conjunction of atomic formulas."); }
    map<IntPolyRef,int>::iterator itrf = (A.is_null() ? B->factorsBegin() : A->factorsBegin());
    map<IntPolyRef,int>::iterator itr_end = (A.is_null() ? B->factorsEnd() : A->factorsEnd());
    for( ; itrf != itr_end; ++itrf)
    {
      IntPolyObj* p = &(*(itrf->first));
      if (P2sign.find(p) == P2sign.end())
      {
	int s = X->partialEval(p,alpha,X->level(p))->signIfConstant();      
	int sinv = D->inPFSet(p);
	P2sign[p] = s;
	P2signInvariant[p] = sinv;
      }
    }
  }


  // go through each atom A in conjunction C and categorize as tiT (known truth-invariant true), tiF (known truth-invariant false)
  // alphaT (true at alpha, not known truth-invariant) or alphaF (false at alpha, not known truth-invariant)
  int tiT = 0, tiF = 0, alphaT = 0, alphaF = 0;
  set<IntPolyRef> polyT, polyF, polyIndividuallySufficientF;
  for(TAndObj::conjunct_iterator itr = C->conjuncts.begin(); itr != C->conjuncts.end(); ++itr)
  {
    int sat;
    vector<IntPolyObj*> nonti;
    TAtomRef A = asa<TAtomObj>(*itr);
    TExtAtomRef B = asa<TExtAtomObj>(*itr);

    if (!A.is_null()) //-- REGULAR TARSKI ATOM
    {      
      int s = A->F->signOfContent();
      for(map<IntPolyRef,int>::iterator itr = A->factorsBegin(); itr != A->factorsEnd(); ++itr)
      {
	IntPolyObj* p = &(*(itr->first));
	if (!P2signInvariant[p]) nonti.push_back(p);
	int sf = P2sign[p]; if (itr->second % 2 == 0) sf = sf*sf;
	s = s*sf;
      }
      sat = signSatSigma(s,A->relop);
    }
    else //-- EXTENDED TARSKI ATOM
    {
      int n_less = 0, n_equal = 0, n_greater = 0;
      bool nonnullified = true;
      for(map<IntPolyRef,int>::iterator itr = B->factorsBegin(); itr != B->factorsEnd(); ++itr)
      {
	IntPolyObj* p = &(*(itr->first));
	if (!P2signInvariant[p]) nonti.push_back(p);
	if (nonnullified)
	  nonnullified = X->partialEvalRoot(p, alpha, X->level(p), n_less, n_equal, n_greater);
      }
      sat = nonnullified && B->detTruth(n_less,n_equal,n_greater);
    }

    if (sat)
      if (nonti.empty()) tiT++;
      else { alphaT++; for(int i = 0; i < nonti.size(); ++i) polyT.insert(nonti[i]); }
    else 
      if (nonti.empty()) tiF++;
      else 
      { 
	alphaF++; 
	for(int i = 0; i < nonti.size(); ++i) 
	  polyF.insert(nonti[i]); 
	if (nonti.size() == 1)
	  polyIndividuallySufficientF.insert(nonti[0]);
      }
  }

  // make a choice!
  if (tiF != 0) 
  { tvAtAlpha = FALSE; targetTruthValue = FALSE; /* Q = emptyset */ return; }
  
  if (tiF == 0 && alphaT == 0 && alphaF == 0) 
  { tvAtAlpha = TRUE; targetTruthValue = TRUE; /* Q = emptyset */ return; }

  if (alphaF != 0)
  {
    IntPolyRef p = chooseNextPoly(polyF,X,node);
    tvAtAlpha = FALSE;
    targetTruthValue = polyIndividuallySufficientF.find(p) == polyIndividuallySufficientF.end() ? UNDET : FALSE;
    Q.insert(p);
    return;
  }

  if (alphaT != 0)
  {
    IntPolyRef p = chooseNextPoly(polyT,X,node);
    tvAtAlpha = TRUE;
    targetTruthValue = polyT.size() == 1 ? TRUE : UNDET;
    Q.insert(p);
    return;
  }
  return; 
}


/*************************************************
 * d n f
 ************************************************/
void chooseSplitDNF(VarOrderRef X, vector<TAndRef> &C, int dim, NodeRef node,
		    set<IntPolyRef> &Q, int &tvAtAlpha, int &targetTruthValue);

void ONuCADObj::mkNuCADDNF(VarOrderRef X, TFormRef F, int dim, Word alpha)
{

  // Initialize V/L as an empty "queue" of labeled cells, then enque the cell R^dim
  PolyManager *ptrPM = X->getPolyManager();
  BuilderRef rootBuilder = new BuilderObj(new OpenCellObj(X,alpha,dim));
  int front = 0;
  nodeQueue->push(root = new NodeObj(NULL,rootBuilder,UNDET,0,'C'));

  SplitSetChooserRef chooser = new SplitSetChooserDNF(F);

  while(!nodeQueue->stopSearch())
  {
    // Dequeue cell D with label label
#ifdef _OC_DEBUG_
    cerr << "queue size = " << nodeQueue->size() << endl;
#endif
    if (verbose) { cerr << "queue size = " << nodeQueue->size() << endl; }
    NodeRef node = nodeQueue->front(); nodeQueue->pop();
    if (node->getTruthValue() != UNDET) { continue; }
    NuCADSplit(X,chooser,dim,node);
  }  
}


class CAResObj;
typedef GC_Hand<CAResObj> CAResRef;


class CAResObj : public GC_Obj
{
private:
public:
  enum TruthCat { tiF = 0, tiT = 1, alphaF = 2, alphaT = 3 };
  TruthCat category;
  set<IntPolyRef> polyT, polyF, polyIndividuallySufficientF;
public:
  CAResObj() { }
};

/*
  NuCADAnalyzeDNF(X,C,dim,node) analyzes conjunction C in cell node->data, returning a CAResObj
  with the results of the analuysis.
  TODO: make P2sign and P2signInvariant arguments, so that info is not recomputed!
 */
CAResRef NuCADAnalyzeDNF(VarOrderRef X, TAndRef C, int dim, NodeRef node)
{
  GCWord alpha = node->getData()->getCell()->getAlpha();
  CAResRef res = new CAResObj();

  // collect all factors appearing in C and determine their signs & whether or not they are sign invariant
  map<IntPolyObj*,int > P2sign; 
  map<IntPolyObj*,bool > P2signInvariant; 
  for(TAndObj::conjunct_iterator itr = C->conjuncts.begin(); itr != C->conjuncts.end(); ++itr)
  {
    TAtomRef A = asa<TAtomObj>(*itr);
    TExtAtomRef B = asa<TExtAtomObj>(*itr);
    if (A.is_null() && B.is_null()) { throw TarskiException("NuCADSplitConjunction requires conjunction of atomic formulas."); }
    map<IntPolyRef,int>::iterator itrf = (A.is_null() ? B->factorsBegin() : A->factorsBegin());
    map<IntPolyRef,int>::iterator itr_end = (A.is_null() ? B->factorsEnd() : A->factorsEnd());
    for( ; itrf != itr_end; ++itrf)
    {
      IntPolyObj* p = &(*(itrf->first));
      if (P2sign.find(p) == P2sign.end())
      {
	int s = X->partialEval(p,alpha,X->level(p))->signIfConstant();      
	int sinv = node->getData()->inPFSet(p);
	P2sign[p] = s;
	P2signInvariant[p] = sinv;
      }
    }
  }

  // go through each atom A in conjunction C and categorize as tiT (known truth-invariant true), tiF (known truth-invariant false)
  // alphaT (true at alpha, not known truth-invariant) or alphaF (false at alpha, not known truth-invariant)
  int tiT = 0, tiF = 0, alphaT = 0, alphaF = 0;
  //  set<IntPolyRef> polyT, polyF, polyIndividuallySufficientF;
  for(TAndObj::conjunct_iterator itr = C->conjuncts.begin(); itr != C->conjuncts.end(); ++itr)
  {
    int sat;
    vector<IntPolyObj*> nonti;
    TAtomRef A = asa<TAtomObj>(*itr);
    TExtAtomRef B = asa<TExtAtomObj>(*itr);

    if (!A.is_null()) //-- REGULAR TARSKI ATOM
    {      
      int s = A->F->signOfContent();
      for(map<IntPolyRef,int>::iterator itr = A->factorsBegin(); itr != A->factorsEnd(); ++itr)
      {
	IntPolyObj* p = &(*(itr->first));
	if (!P2signInvariant[p]) nonti.push_back(p);
	int sf = P2sign[p]; if (itr->second % 2 == 0) sf = sf*sf;
	s = s*sf;
      }
      sat = signSatSigma(s,A->relop);
    }
    else //-- EXTENDED TARSKI ATOM
    {
      int n_less = 0, n_equal = 0, n_greater = 0;
      bool nonnullified = true;
      for(map<IntPolyRef,int>::iterator itr = B->factorsBegin(); itr != B->factorsEnd(); ++itr)
      {
	IntPolyObj* p = &(*(itr->first));
	if (!P2signInvariant[p]) nonti.push_back(p);
	if (nonnullified)
	  nonnullified = X->partialEvalRoot(p, alpha, X->level(p), n_less, n_equal, n_greater);
      }
      sat = nonnullified && B->detTruth(n_less,n_equal,n_greater);
    }

    if (sat)
      if (nonti.empty()) tiT++;
      else { alphaT++; for(int i = 0; i < nonti.size(); ++i) res->polyT.insert(nonti[i]); }
    else 
      if (nonti.empty()) tiF++;
      else 
      { 
	alphaF++; 
	for(int i = 0; i < nonti.size(); ++i) 
	  res->polyF.insert(nonti[i]); 
	if (nonti.size() == 1)
	  res->polyIndividuallySufficientF.insert(nonti[0]);
      }
  }

  if (tiF != 0)  { res->category = CAResObj::tiF; }
  else if (tiF == 0 && alphaT == 0 && alphaF == 0) { res->category = CAResObj::tiT; }
  else if (alphaF != 0) { res->category = CAResObj::alphaF; }
  else if (alphaT != 0) { res->category = CAResObj::alphaT; }
  return res; 
}

void SplitSetChooserDNF::chooseSplit(VarOrderRef X, NodeRef node, int dim,
		   set<IntPolyRef> &Q, int &tvAtAlpha, int &targetTruthValue)
{
  int n_tiF = 0, n_tiT = 0, n_alphaF = 0, n_alphaT = 0;
  vector<CAResRef> results;
  for(int i = 0; i < C.size(); ++i)
  {
    CAResRef r = NuCADAnalyzeDNF(X,C[i],dim,node);
    results.push_back(r);
    if (r->category == CAResObj::tiF) n_tiF++;
    else if (r->category == CAResObj::tiT) n_tiT++;
    else if (r->category == CAResObj::alphaF) n_alphaF++;
    else n_alphaT++;    
  }

  // determine case and act
  if (n_tiT > 0) //-- at least one disjunct is known truth-invariant true, so F is truth-invariant true
  {
    tvAtAlpha = TRUE; targetTruthValue = TRUE; /* Q = emptyset */ return;
  }
  else if (n_tiF == C.size()) //-- all disjuncts are known truth-invariant false , so F is truth-invariant false
  {
    tvAtAlpha = FALSE; targetTruthValue = FALSE; /* Q = emptyset */ return;
  }
  else if (n_tiF + n_alphaF == C.size()) //-- all disjuncts are false at alpha, at least one is not known truth-invariant false
  {
    // collect disjuncts that are alphaF
    vector<CAResRef> T;
    for(int i = 0; i < results.size(); ++i)
      if (results[i]->category == CAResObj::alphaF)
	T.push_back(results[i]);

    // choose polynomial p appearing in at least one disjunct's polyF set [NOTE: The way I'm doing this now is lame!]
    BasicPolyCompare CMP(X);
    set<IntPolyRef> S;
    for(int i = 0; i < T.size(); ++i)
      for(set<IntPolyRef>::iterator itr = T[i]->polyF.begin(); itr != T[i]->polyF.end(); ++itr)
	S.insert(*itr);
    set<IntPolyRef>::iterator itr = S.begin();
    IntPolyRef p = *itr; ++itr;
    for( ; itr != S.end() ; ++itr)
      if (CMP.lt(*itr,p))
	p = *itr;
    // set targetTruthValue ... can only say "UNDET" unless p appears as sufficient in every alphaF conjunct, 
    // in which case we can say FALSE
    int check = true;
    for(int i = 0; check && i < T.size(); ++i)
      check = (T[i]->polyIndividuallySufficientF.find(p) != T[i]->polyIndividuallySufficientF.end());

    tvAtAlpha = FALSE; targetTruthValue = (check ? FALSE : UNDET); Q.insert(p); return;
  }
  else //-- no known truth-invariant true disjuncts, but at least one is true at alphaT
  {
    // choose one of the alphaT disjuncts & choose polynomial p from its polyT set
    int m = -1;
    for(int i = 0; i < results.size(); ++i)
      if (results[i]->category == CAResObj::alphaT && (m == -1 || results[i]->polyT.size() < results[m]->polyT.size()))
	m = i;

    BasicPolyCompare CMP(X);
    set<IntPolyRef>::iterator itr = results[m]->polyT.begin();
    IntPolyRef p = *itr; ++itr;
    for( ; itr != results[m]->polyT.end() ; ++itr)
      if (CMP.lt(*itr,p))
	p = *itr;

    // set targetTruthValue ... can only say "UNDET" unless p came from a polyT set of size one, in which case we can say TRUE
    tvAtAlpha = TRUE; targetTruthValue = (results[m]->polyT.size() == 1 ? TRUE : UNDET); Q.insert(p); return;
  }
}


/*
res <- locate(n,C,beta)
Input:
  n    : a positive integer
  C    : a Node representing a cell in an OpenNuCAD of n-space
  beta : a point contained within C
Output:
  res  : (N,k) if k == 0, N is the Node containing beta, otherwise
         if sgn(k) == -1, beta is a zero of the |k|th level lower
         boundary of N, else of the |k|th level upper boundary. 
 */
pair<NodeRef,int> ONuCADObj::locate(NodeRef C, Word beta)
{
  if (!C->hasChildren()) { return make_pair(C,0); }
  VarOrderRef X = C->getData()->getCell()->getVarOrder();
  bool boundaryError = false;
  int N = std::min(C->XYchild->getSplitLevel(),LENGTH(beta));
  int il = 0, iu = 0, nextK;
  int nextkL = C->Lchild.size() > il ? C->Lchild[il]->getSplitLevel() : N+1;
  int nextkU = C->Uchild.size() > iu ? C->Uchild[iu]->getSplitLevel() : N+1;
  while((nextK = std::min(nextkL,nextkU)) && nextK <= N)
  {
    RealAlgNumRef Q = rationalToRealAlgNum(LELTI(beta,nextK));
    if (nextK == nextkL) 
    {
      // compare alpha_nextK with the upper bound of the current lower child
      int s = C->Lchild[il]->getData()->getCell()->get(nextK)->compareWithBound(+1,nextK,beta,Q,X);
      if (s < 0) { return locate(C->Lchild[il],beta); }
      else if (s == 0) { return make_pair(C->Lchild[il],nextK); }
      else { il++; nextkL = (C->Lchild.size() > il ? C->Lchild[il]->getSplitLevel() : N+1); }	
    } 
    if (nextK == nextkU) 
    {
      // compare alpha_nextK with the upper bound of the current upper child
      int s = C->Uchild[iu]->getData()->getCell()->get(nextK)->compareWithBound(-1,nextK,beta,Q,X);
      if (s > 0) { return locate(C->Uchild[iu],beta); }
      else if (s == 0) { return make_pair(C->Uchild[iu],-nextK); }
      else { iu++; nextkU = (C->Uchild.size() > iu ? C->Uchild[iu]->getSplitLevel() : N+1); }	
    }
  }
  return locate(C->XYchild,beta);
}

vector<NodeRef> ONuCADObj::locate(Word A)
{
  vector<GCWord> P; P.push_back(NIL);
  for(Word Ap = A; Ap != NIL; Ap = RED(Ap))
    P.push_back(FIRST(Ap));
  VarOrderRef X = root->getData()->getCell()->getVarOrder();
  vector<NodeRef> V;
  stack<NodeRef> S;
  S.push(root);
  while(!S.empty())
  {
    NodeRef n = S.top(); S.pop();
    if (!n->hasChildren()) { V.push_back(n); break; }

    NodeRef found = NULL;
    bool boundaryError = false;
    int N = std::min(n->XYchild->getSplitLevel(),LENGTH(A));
    int il = 0, iu = 0, nextK;
    int nextkL = n->Lchild.size() > 0 ? n->Lchild[il]->getSplitLevel() : N+1;
    int nextkU = n->Uchild.size() > 0 ? n->Uchild[iu]->getSplitLevel() : N+1;
    while((nextK = std::min(nextkL,nextkU)) && nextK <= N)
    {
      RealAlgNumRef Q = rationalToRealAlgNum(LELTI(A,nextK));
      if (nextK == nextkL) 
      {
	// compare alpha_nextK with the upper bound of the current lower child
	int s = n->Lchild[il]->getData()->getCell()->get(nextK)->compareWithBound(+1,nextK,A,Q,X);
	if (s < 0) { found = n->Lchild[il]; break; }
	else if (s == 0) { boundaryError = true; break; }
	else { il++; nextkL = (n->Lchild.size() > il ? n->Lchild[il]->getSplitLevel() : N+1); }	
      }
      if (nextK == nextkU) 
      {
	// compare alpha_nextK with the upper bound of the current upper child
	int s = n->Uchild[iu]->getData()->getCell()->get(nextK)->compareWithBound(-1,nextK,A,Q,X);
	if (s > 0) { found = n->Uchild[iu]; break; }
	else if (s == 0) { boundaryError = true; break; }
	else { iu++; nextkU = (n->Uchild.size() > iu ? n->Uchild[iu]->getSplitLevel() : N+1); }	
      }
    }

    if (boundaryError)
      throw TarskiException("Error in locate!  Input point lies on cell boundary!");

    if (!found.is_null())
      S.push(found);
    else
    {
      // push the XYchild and all remaining L&U children
      S.push(n->XYchild);
      while(il < n->Lchild.size())
	S.push(n->Lchild[il++]);
      while(iu < n->Uchild.size())
	S.push(n->Uchild[iu++]);
    }
  }
  return V;
}


/*

projectionResolve(NodeRef C1, NodeRef C2, int levelk, int dim)
C1 : a node with subtree level <= levelk
C2 : a node with subtree level <= levelk
levelk : < dim
dim : C1 & C2 cells in NuCAD(s) of R^dim
 */
static int tmpCtr = 0;
void ONuCADObj::projectionResolve(NodeRef C1, NodeRef C2, int levelk, int dim)
{
  cerr << ++tmpCtr << endl;
  // new queue, initially containing all true leaf cells in the subtree of C2
  queue<NodeRef> Q;
  stack<NodeObj::ChildIterator> ciS; ciS.push(C2->childBegin());
  while(!ciS.empty())
  {
    NodeObj::ChildIterator &itr = ciS.top();
    if (itr.hasNext())
    {
      NodeRef n = itr.next();
      if (n->hasChildren())
	ciS.push(n->childBegin());
      else if (n->getTruthValue() == TRUE)
      {
	Q.push(n);
      }
    }
    else
      ciS.pop();
  }

  int debugLoopCount = 0;
  while(!Q.empty())
  {
    NodeRef c = Q.front(); Q.pop();
    Word alpha = c->getData()->getCell()->getAlpha();
    Word alphaTrimmed = PREFIXLIST(alpha,levelk);

    pair<NodeRef,int> res = locate(C1,alphaTrimmed);
    NodeRef d = res.first;
    while (res.second != 0) // POINT ON A CELL BOUNDARY IN C1
    { 
      // we have to perturb alphaTrimmed a bit so that it is still in c, but (eventually)
      // no longer on a boundary in C1
      int errk = abs(res.second), dir = res.second < 0 ? -1 : +1;
      CellBoundRef B_d = res.first->getData()->getCell()->get(errk);
      CellBoundRef B_c = c->getData()->getCell()->get(errk);
      VarOrderRef X = this->getVarOrder();
      if (dir < 0) // means alphaTrummed fell on the left boundary of d
      {
	// find coord a_new s.t. alpha_errk < a_new < d's upper boundary over (alpha_1,...,alpha_{errk-1})
	RealAlgNumRef ub_c = B_c->getBoundAbovePoint(+1,errk,alphaTrimmed,X);
	RealAlgNumRef ub_d = B_d->getBoundAbovePoint(+1,errk,alphaTrimmed,X);
	RealAlgNumRef lb_d = B_d->getBoundAbovePoint(-1,errk,alphaTrimmed,X); // aka "alpha_errk"
	Word coord_new = rationalPointInInterval(lb_d,min(ub_c,ub_d));
	Word newPreAlpha = CONC(PREFIXLIST(alphaTrimmed,errk-1),LIST1(coord_new));
	Word newAlpha = c->getData()->getCell()->moveSamplePoint(newPreAlpha);
	if (newAlpha == NIL)
	  throw TarskiException("Unexpected failure to move sample point in projectionResove!");
	alpha = newAlpha;
	alphaTrimmed = PREFIXLIST(alpha,levelk);
	res = locate(C1,alphaTrimmed);
	d = res.first;
      }
      else // means alphaTrimmed fell on the right boundary of d
      {
	// find coord a_new s.t. d's lower boundary over (alpha_1,...,alpha_{errk-1}) < a_new < alpha_errk
	RealAlgNumRef lb_c = B_c->getBoundAbovePoint(-1,errk,alphaTrimmed,X);
	RealAlgNumRef lb_d = B_d->getBoundAbovePoint(-1,errk,alphaTrimmed,X);
	RealAlgNumRef ub_d = B_d->getBoundAbovePoint(+1,errk,alphaTrimmed,X); // aka "alpha_errk"
	Word coord_new = rationalPointInInterval(max(lb_c,lb_d),ub_d);
	Word newPreAlpha = CONC(PREFIXLIST(alphaTrimmed,errk-1),LIST1(coord_new));
	Word newAlpha = c->getData()->getCell()->moveSamplePoint(newPreAlpha);
	if (newAlpha == NIL)
	  throw TarskiException("Unexpected failure to move sample point in projectionResove!");
	alpha = newAlpha;
	alphaTrimmed = PREFIXLIST(alpha,levelk);
	res = locate(C1,alphaTrimmed);
	d = res.first;
      }
    }

    // This basically makes cx a clone of d.  NOTE: It's important to move d's sample point so
    // that it agrees up to level levelk with c's sample point.  The merge operation depends on it!
    d->getData()->getCell()->moveSamplePoint(alphaTrimmed);
    BuilderRef cx = new BuilderObj(new OpenCellObj(X,LCOPY(d->getData()->getCell()->getAlpha()),dim));
    BuilderObj::mergeBintoA(cx,d->getData(),dim); 

    // "split c by merging with d", i.e. cx is the "split by c" version of d
    BuilderObj::mergeBintoA(cx,c->getData(),levelk);
    if (cx->inErrorState()) 
    { 
      dealWithFail(X,c->getData(),cx,dim);
      Q.push(c);
      continue;
    }

    // enqueue onto Q all "split off" subcells of c (except cx itself)
    for(int k = 1; k <= levelk; ++k)
    {
      vector<BuilderRef> W;
      BuilderObj::splitAtLevelK(c->getData(),cx,dim,k,W);
      if (!W[0].is_null()) { 
	NodeRef tmp = new NodeObj(&*c,W[0],UNDET,k,'L');
	Q.push(tmp);
      }
      if (!W[1].is_null()) { 
	NodeRef tmp = new NodeObj(&*c,W[1],UNDET,k,'U');
	Q.push(tmp);
      }
    }

    // if d marked false then split d by merging with e, marking the intersection 
    // cell (which should be exactly equivalent to e) as true and the remaining new cells false
    if (d->getTruthValue() == FALSE)
    {
      BuilderRef D = d->data;
      int maxk = 0;
      for(int k = 1; k <= levelk; ++k)
      {
	vector<BuilderRef> W;
	BuilderObj::splitAtLevelK(D,cx,dim,k,W);
	if (!W[0].is_null()) { 
	  NodeRef tmp = new NodeObj(&*d,W[0],FALSE,k,'L');
	  d->Lchild.push_back(tmp);
	  maxk = k;
	}
	if (!W[1].is_null()) { 
	  NodeRef tmp = new NodeObj(&*d,W[1],FALSE,k,'U');
	  d->Uchild.push_back(tmp);
	  maxk = k;
	}
      }
      
      // If we have actually split, add cx as (TRUE) child, otherwise just replace existing cell in node
      if (maxk != 0)
      {
	d->XYchild = new NodeObj(&*d,cx,TRUE,maxk,'X');
	d->truthValue = UNDET;
      }
      else
      {
	d->data = cx;
	d->truthValue = TRUE;
      }      
    }
  }
}

int NodeObj::getSubtreeLevel(bool initFlag)
{
  int maxLevel = initFlag ? 0 : getSplitLevel();
  if (!hasChildren()) { return maxLevel; }
  for(NodeObj::ChildIterator itr = childBegin(); itr.hasNext(); maxLevel = std::max(maxLevel,itr.next()->getSubtreeLevel(false)));
  return maxLevel;
}


NodeRef NodeObj::lowerMostNodeWithSubtreeLevelExceedingK(int k)
{
  if (!hasChildren()) return NULL;
  NodeRef recXY = XYchild->lowerMostNodeWithSubtreeLevelExceedingK(k);
  if (!recXY.is_null()) return recXY;
  int m = XYchild->getSplitLevel();

  int iL = Lchild.size();
  NodeRef recL = (iL == 0 || Lchild[iL-1]->getSplitLevel() < m 
		  ? NULL : Lchild[--iL]->lowerMostNodeWithSubtreeLevelExceedingK(k));
  if (!recL.is_null()) return recL;
  
  int iU = Uchild.size();
  NodeRef recU = (iU == 0 || Uchild[iU-1]->getSplitLevel() < m 
		  ? NULL : Uchild[--iU]->lowerMostNodeWithSubtreeLevelExceedingK(k));
  if (!recU.is_null()) return recU;

  if (XYchild->getSplitLevel() > k)
    return this;

  while(iL > 0)
  {
    NodeRef recL = Lchild[--iL]->lowerMostNodeWithSubtreeLevelExceedingK(k);
    if (!recL.is_null()) return recL;
  }

  while(iU > 0)
  {
    NodeRef recU = Uchild[--iU]->lowerMostNodeWithSubtreeLevelExceedingK(k);
    if (!recU.is_null()) return recU;
  }
  return NULL;
}


NodeRef ONuCADObj::getNode(const char* p) { string s(p); return getNode(s); }

/******************** OpenNuCADSATSolver **************************/

//-- Input : atom A and map knownRelop that maps poly p to the relop we know it satisfies in some given cell D
//-- Output: TRUE, FALSE, or UNDET, the truth value A can be deduced to have in the cell based on knownRelop.
int evalTruth(TAtomRef A, map<IntPolyObj*,int > &knownRelop)
{
  int targetRelop = A->getRelop();
  int evalRelop = signToSigma(A->F->signOfContent());
  for(map<IntPolyRef,int>::iterator itr = A->F->MultiplicityMap.begin(); itr != A->F->MultiplicityMap.end(); ++itr)
  {
    IntPolyObj* p = &(*(itr->first));
    int relop_p = knownRelop[p];
    evalRelop = T_prod[evalRelop][itr->second % 2 == 0 ? T_square[relop_p] : relop_p];
  }
  if (sameOrStronger(evalRelop,targetRelop))
    return TRUE;
  else if (sameOrStronger(evalRelop,negateRelop(targetRelop)))
    return FALSE;
  else
    return UNDET;
}


TAndRef OpenNuCADSATSolverObj::getUNSATCore()
{
  return nucad->getUNSATCore();
  /*
  const int n = C->size();  //-- C is the conjunction that came as orig input
  std::vector<TFormRef> atoms(n);
  {  int i = 0;
    for(auto citr = C->begin(); citr != C->end(); ++citr)
      atoms[i++] = *citr;
  }
  set<IntPolyObj*> S;
  for(int i = 0; i < n; i++)
  {
    TAtomRef atom = asa<TAtomObj>(atoms[i]);
    if (atom.is_null()) {
      throw TarskiException("OpenNuCADSATSolverObj::getUNSATCore currently conjunction of atoms"); }
    FactRef F = atom->getFactors();
    for(auto fitr = F->factorBegin(); fitr != F->factorEnd(); ++fitr)
      S.insert(&(*(fitr->first)));
  }
  
  //-- set up hitting set problem
  std::vector< std::vector<int> > hitSetIn;
  auto litr = nucad->iterator(); //-- itrates of the leaves
  for(int i = 0; litr.hasNext(); ++i)
  {
    //-- pull out D, the next cell to consider (and associated info)
    NodeRef node = litr.next();
    BuilderRef b = node->getData();
    OpenCellRef D = b->getCell();
    Word alpha = D->getAlpha();

    
    //-- b->inPFSet(p), where p is an IntPolyRef, returns true iff p is known
    //-- to be sign invariant in cell D.

    //-- for IntPolyObj* p, knownRelop[p] is > if it is known positive throught cell D, < if
    //-- it is known negative throught, = if it is known zero throughout, and ALOP otherwise.
    map<IntPolyObj*,int > knownRelop;
    for(auto pitr = S.begin(); pitr != S.end(); ++pitr)
    {
      if (b->inPFSet(*pitr))
      {
	IntPolyRef p = V->partialEval(*pitr,alpha,LENGTH(alpha));
	if (!p->isConstant()) { throw TarskiException("Error!  Evaluation did not produce a constant!"); }
	int sigma = signToSigma(p->signIfConstant());
	knownRelop[*pitr] = sigma;
      }
      else
	knownRelop[*pitr] = ALOP;
    }

    //-- add to hist set problem any atom known to be false throught cell D
    hitSetIn.push_back(vector<int>());
    vector<int> &nextset = hitSetIn.back();
    for(int j = 0; j < n; j++)
    {
      if (evalTruth(atoms[j],knownRelop) == FALSE)
	nextset.push_back(j+1);
    }
    nextset.push_back(0);
  }
  
  //-- solve hitting set problem and translate into unsat core
  HitProb HP;
  HP.fill(n,hitSetIn.size(),hitSetIn);
  vector<int> H;
  naiveSolve(HP,H);
  
  TAndRef unsatCore = new TAndObj();
  for(int k = 0; k < H.size(); ++k)
    unsatCore->AND(atoms[H[k] - 1]);
  return unsatCore;
  */
}


TAndRef ONuCADObj::getUNSATCore()
{
  const int n = this->C->size();  //-- C is the conjunction that came as orig input
  std::vector<TFormRef> atoms(n);
  {  int i = 0;
    for(auto citr = C->begin(); citr != C->end(); ++citr)
      atoms[i++] = *citr;
  }
  set<IntPolyObj*> S;
  for(int i = 0; i < n; i++)
  {
    TAtomRef atom = asa<TAtomObj>(atoms[i]);
    if (atom.is_null()) {
      throw TarskiException("OpenNuCADSATSolverObj::getUNSATCore currently conjunction of atoms"); }
    FactRef F = atom->getFactors();
    for(auto fitr = F->factorBegin(); fitr != F->factorEnd(); ++fitr)
      S.insert(&(*(fitr->first)));
  }
  
  //-- set up hitting set problem
  std::vector< std::vector<int> > hitSetIn;
  auto litr = this->iterator(); //-- itrates of the leaves
  for(int i = 0; litr.hasNext(); ++i)
  {
    //-- pull out D, the next cell to consider (and associated info)
    NodeRef node = litr.next();
    BuilderRef b = node->getData();
    OpenCellRef D = b->getCell();
    Word alpha = D->getAlpha();

    
    //-- b->inPFSet(p), where p is an IntPolyRef, returns true iff p is known
    //-- to be sign invariant in cell D.

    //-- for IntPolyObj* p, knownRelop[p] is > if it is known positive throught cell D, < if
    //-- it is known negative throught, = if it is known zero throughout, and ALOP otherwise.
    map<IntPolyObj*,int > knownRelop;
    for(auto pitr = S.begin(); pitr != S.end(); ++pitr)
    {
      if (b->inPFSet(*pitr))
      {
	IntPolyRef p = this->X->partialEval(*pitr,alpha,LENGTH(alpha));
	if (!p->isConstant()) { throw TarskiException("Error!  Evaluation did not produce a constant!"); }
	int sigma = signToSigma(p->signIfConstant());
	knownRelop[*pitr] = sigma;
      }
      else
	knownRelop[*pitr] = ALOP;
    }

    //-- add to hist set problem any atom known to be false throught cell D
    hitSetIn.push_back(vector<int>());
    vector<int> &nextset = hitSetIn.back();
    for(int j = 0; j < n; j++)
    {
      if (evalTruth(atoms[j],knownRelop) == FALSE)
	nextset.push_back(j+1);
    }
    nextset.push_back(0);
  }
  
  //-- solve hitting set problem and translate into unsat core
  HitProb HP;
  HP.fill(n,hitSetIn.size(),hitSetIn);
  vector<int> H;
  naiveSolve(HP,H);
  
  TAndRef unsatCore = new TAndObj();
  for(int k = 0; k < H.size(); ++k)
    unsatCore->AND(atoms[H[k] - 1]);
  return unsatCore;
}


/********* HPC LEARNING STUFF *************/
/*
  Get node, revert it, run each choice along with the subtree size it produced.  Output.
 */

class NothingLeftToChoose : public TarskiException
{
public:
  NothingLeftToChoose() : TarskiException("Nothing Left To Choose.") { }
};

class TrialChooserObj; typedef GC_Hand<TrialChooserObj> TrialChooserRef;
class TrialChooserObj : public SplitSetChooserConjunction
{
private:
  SplitSetChooserRef chooser;
  set<IntPolyRef> chosen;
  IntPolyRef lastChoice;
  NodeRef splitNode;
  BasicPolyCompare BPC;
public:
  TrialChooserObj(SplitSetChooserRef chooser, NodeRef node, TAndRef C) : SplitSetChooserConjunction(C),
									 BPC(node->getData()->getCell()->getVarOrder())
  {
    this->chooser = chooser; splitNode = node;
  }

  bool operator()(const pair<int,IntPolyRef> &p1, const pair<int,IntPolyRef> &p2)
  {
    return BPC.lt(p1.second,p2.second);
  } 
  
  IntPolyRef chooseNextPoly(set<IntPolyRef> &S, VarOrderRef X, NodeRef node)
  {
    if (this->splitNode.identical(node) && lastChoice.is_null())
    {
      /*
      if (chosen.size() == 0) {
	for(auto itr = S.begin(); itr != S.end(); ++itr) {
	  cout << ">>>> "; (*itr)->write(*(X->getPolyManager())); cout << endl; }
      }
      cout << "S.size() = " << S.size() << "   " << node->getLabel() << endl;
      */
      auto itr = S.begin();
      while(itr != S.end() && chosen.find(*itr) != chosen.end())
	++itr;
      if (itr == S.end()) throw NothingLeftToChoose();
      chosen.insert(lastChoice = *itr);
      return lastChoice;
    }
    else
      return chooser->chooseNextPoly(S,X,node);
  }
  void reset() { chosen.clear(); lastChoice = NULL; } 
  int numChosen() { return chosen.size(); }
  IntPolyRef getLastChoice() { auto x = lastChoice; lastChoice = NULL; return x; }
};

vector<float> generateFeatures(NodeRef node, IntPolyRef p1, IntPolyRef p2);
pair<IntPolyRef,IntPolyRef> removeTerms(IntPolyRef p, IntPolyRef q);

void ONuCADObj::trial(NodeRef node, vector<vector<float>> &X, vector<float> &y)
{  
  SplitSetChooserRef toRevert = chooser;
  TrialChooserRef tchooser = new TrialChooserObj(toRevert,node,this->C);
  chooser = tchooser;

  /*
    Populate 'results' with (int,poly) pairs in which 'poly' is
    the split polynomial chosen, and 'int' is the number of nodes in
    the subtree that resulted from taking that choice.
   */
  vector<pair<int,IntPolyRef>> results;
  Word alpha = node->getData()->getCell()->getAlpha();
  bool done = false;
  int minSize = -1;
  while(!done)
  {
    node->revert(); //revertCell(label);
    try {
      refineSubtreeNuCADConjunction(node);
      Word alphap = node->getData()->getCell()->getAlpha();
      if (!EQUAL(alpha,alphap)) {
	tchooser->reset();
	results.clear();
	alpha = alphap;
	minSize = -1;
	cout << " sample point is now "; node->getData()->getCell()->writeAlpha(); cout << endl;
      }
      else {
	int count = 0;
	for(auto itr = iterator(node); itr.hasNext(); itr.next(), ++count);
	cerr << "count = " << count << endl;
	results.push_back(pair<int,IntPolyRef>(count,tchooser->getLastChoice()));
	if (minSize == -1 || count < minSize) { minSize = count; }
      }
    }
    catch(NothingLeftToChoose &e) { done = true; }
    cerr << "minSize = " << minSize << endl;
  }
  
  //node->getData()->getCell()->debugPrint();

  /*
    For each pair (int_i,poly_i) , (int_j,poly_j) we get a data row,
    features(poly_i,poly_j), int_i - int_j.
   */
  for(int i = 0; i < results.size(); ++i) {
    for(int j = 0; j < results.size(); ++j) {
      int delta = results[i].first - results[j].first;
      if (delta == 0) continue;

      X.push_back(generateFeatures(node,results[i].second,results[j].second));
      y.push_back(float(delta)/minSize);

      /*
      vector<float> &V = X.back();
      cout << delta << "(" << float(delta)/minSize << ") : ";
      results[i].second->write(*(node->getData()->getPolyManager()));
      cout << ", ";
      results[j].second->write(*(node->getData()->getPolyManager()));
      cout << " ";
      cout << "[ " << V[0];
      for(int k = 1; k < V.size(); ++k)
	cout << ", " << V[k];
      cout << " ]";
      cout << " --- ";
      pair<IntPolyRef,IntPolyRef> p = removeTerms(results[i].second,results[j].second);
      p.first->write(*(node->getData()->getPolyManager()));
      cout << " , ";
      p.second->write(*(node->getData()->getPolyManager()));
      cout << endl;
      */
    }
  }
  chooser = toRevert;
}



Word REMOVETERMS(Word r, Word A, Word B)
{
  if (B == 0 || B == NIL)
    return A;
  if (A == 0 || A == NIL || r == 0)
    return 0;
  Word dA, cA, dB, cB, Ap, Bp;
  FIRST2(A,&dA,&cA);
  FIRST2(B,&dB,&cB);
  Bp = RED2(B); Bp = Bp == NIL ? 0 : Bp;
  Ap = RED2(A); Ap = Ap == NIL ? 0 : Ap;  
  if (dA < dB)
    return REMOVETERMS(r,A,Bp);
  else if (dA > dB)
  {
    Word As = REMOVETERMS(r,Ap,B);    
    return COMP2(dA,cA,(As == 0 ? NIL : As));
  }
  else // dA = dB
  {
    Word cAp = REMOVETERMS(r-1,cA,cB);
    Word As = REMOVETERMS(r,Ap,Bp);
    return cAp == 0 ? As : COMP2(dA,cAp,(As == 0 ? NIL : As));
  }
}
  
/*
  Return a poly that is equal to A with all terms removed that correspond to
  power-products that appear in B.
 */
pair<IntPolyRef,IntPolyRef> removeTerms(IntPolyRef p, IntPolyRef q)
{
  VarSet V = p->getVars() + q->getVars();
  Word r = V.numElements();
  Word A = p->expand(V), B = q->expand(V);
  return pair<IntPolyRef,IntPolyRef>(new IntPolyObj(r,REMOVETERMS(r,A,B),V),new IntPolyObj(r,REMOVETERMS(r,B,A),V));
}
  
void polyPairFeatures(NodeRef node, IntPolyRef p1, IntPolyRef p2, vector<float> &V);

/*** currently 1 + 5 + 5 = 11 features ***/
vector<float> generateFeatures(NodeRef node, IntPolyRef p1, IntPolyRef p2)
{
  vector<float> V;
  int td1 = p1->totalDegree(), td2 = p2->totalDegree();
  V.push_back(td1 == 1 && td2 > 1 ? -1.0 : (td2 == 1 && td1 > 1 ? 1.0 : 0.0));
  polyPairFeatures(node,p1,p2,V);
  pair<IntPolyRef,IntPolyRef> w = removeTerms(p1,p2);
  polyPairFeatures(node,w.first,w.second,V);
  
  // comparative measures - construct q1 the poly consisting of all terms of p1 that are not in p2 (and q2 vice versa)
  // VarKeyedMap<int> M(ALOP);
  // FernPolyIter f1(p1,M), f2(p2,M);
  return V;
}

void polyPairFeatures(NodeRef node, IntPolyRef p1, IntPolyRef p2, vector<float> &V)
{
  VarOrderRef X = node->getData()->getCell()->getVarOrder();
  int n = node->getData()->getCell()->dimension(), lev1 = X->level(p1), lev2 = X->level(p2);
  int td1 = p1->totalDegree(), td2 = p2->totalDegree();
  int sps1 = BasicPolyCompare::SACPOLYSIZE(p1), sps2 = BasicPolyCompare::SACPOLYSIZE(p2); 
  
  // Individual measures
  double s_level = lev1 - lev2 == 0 ? 0.0 : (lev1 - lev2)/double(n - 1);
  double s_tdeg = td1 - td2 == 0 ? 0.0 : (td1 - td2)/5.0;
  double s_sps = sps1 - sps2 == 0 ? 0.0 : (sps1 - sps2)/10.0;
  V.push_back(s_level);
  V.push_back(s_tdeg);
  V.push_back(s_sps);
  V.push_back(p1->degree(X->get(lev1)) - 1.0); // degree in main variable
  V.push_back(p2->degree(X->get(lev2)) - 1.0); // degree in main variable
}

// candidate[i] = (k,node) where node has k leaves.
int ONuCADObj::getCandidateNodes(NodeRef node, std::vector<pair<int,NodeRef>> &candidates,
				 int leafThreshold, int choicesThreshold)
{
  if (!node->hasChildren()) return 1;
  int numLeaves = 0;
  for(auto itr = node->childBegin(); itr.hasNext(); )
    numLeaves += getCandidateNodes(itr.next(),candidates,leafThreshold,choicesThreshold);
  if (numLeaves >= leafThreshold && node->numSplitOptions >= choicesThreshold)
    candidates.push_back(pair<int,NodeRef>(numLeaves,node));
  return numLeaves;
}

IntPolyRef FeatureChooser::chooseNextPoly(set<IntPolyRef> &S, VarOrderRef X, NodeRef node)
{
  int N = S.size();
  if (N == 0) throw TarskiException("Set S empty in chooseNextPoly!");
  IntPolyRef choice;
  if (N == 1)
    choice = *S.begin();
  else {  
    auto itr = S.begin();
    choice = *itr;
    while(++itr != S.end())
    {
      IntPolyRef next = *itr;
      vector<float> F = generateFeatures(node,next,choice);
      float val = this->comp->eval(F);
      if (val < 0.0)
	choice = next;
    }
  }
  return choice;
}


}//end namespace tarski
