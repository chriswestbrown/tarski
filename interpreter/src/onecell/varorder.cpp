#include "varorder.h"
#include "../realroots/realroots.h"

using namespace std;
namespace tarski {
  
static int nextToken(istream &in, string &tok) // 0 means variable, 1 means <, 2 means ; or EOF
{
  int state = 0;
  tok = "";
  int res = 2;
  char c;
  while(state < 3 && in && (c = in.get()))
  {
    switch(state) {
    case 0: 
      if (isspace(c)) { continue; } 
      else if (c == '<') { tok = "<"; res = 1; state = 3; }
      else if (c == ';') { tok = ";"; res = 2; state = 3; }
      else { state = 1; res = 0; tok.push_back(c); }
      break;
    case 1:
      if (isspace(c)) { state = 3; } 
      else if (c == '<' || c == ';') { in.putback(c); state = 3; }
      else tok.push_back(c);
      break;
    }
  }
  return res; 
}


void VarOrderObj::read(istream &in)
{
  string tok;
  bool finished = false;
  while(!finished)
  {
    int r = nextToken(in,tok);
    if (r == 0) { push_back(tok); } 
    else { cerr << "Unexpected token '" << tok << "' in variable order!" << endl; exit(1); }
    r = nextToken(in,tok);
    switch(r) {
    case 0: 
      cerr << "Unexpected token '" << tok << "' in variable order!" << endl; exit(1);
      break;
    case 2:
      finished = true;
      break;
    }
  }
}

void VarOrderObj::write() 
{ 
  if (V.size() == 0) 
  { SWRITE("0"); } 
  else 
  { 
    SWRITE(ptrPM->getName(V[1])); 
    for(int i = 2; i < V.size(); ++i) { SWRITE(" < "); SWRITE(ptrPM->getName(V[i])); }
  } 
}

int VarOrderObj::level(IntPolyRef p) 
{ 
  VarSet W = p->getVars(); 
  int i = V.size()-1; 
  while(i > 0 && (V[i] & W).none()) --i; 
  return i;
}

/*****************************************************************
 * E V A L U A T I O N
 *****************************************************************/

VarKeyedMap<GCWord> VarOrderObj::makeValueMap(Word alpha, int r)
{
  VarKeyedMap<GCWord> value(NIL);
  int i = 1;
  for(Word P = alpha; P != NIL && i <= r; P = RED(P), ++i)
    value[get(i)] = FIRST(P);
  return value;
}

IntPolyRef VarOrderObj::partialEval(IntPolyRef p, Word L, int r)
{
  VarKeyedMap<GCWord> values = makeValueMap(L,r);
  GCWord content = 0;
  IntPolyRef q = ptrPM->evalAtRationalPointMakePrim(p,values,content);
  return RNSIGN(content) >= 0 ? q : getPolyManager()->neg(q);
}

bool VarOrderObj::partialEvalRoot(IntPolyRef p, Word L, int r, int& n_less, int& n_equal, int& n_greater)
{
  VarKeyedMap<GCWord> values = makeValueMap(L,r-1);
  Word alpha_z = LELTI(L,r);
  RealAlgNumRef alpha_z_ran = rationalToRealAlgNum(alpha_z);
  
  GCWord content;
  IntPolyRef A = ptrPM->evalAtRationalPointMakePrim(p,values,content);
  if (A->isZero()) { return false; }
  vector<RealRootIUPRef> roots =  RealRootIsolateRobust(A);
  for(int i = 0; i < roots.size(); i++)
  {
    int c = roots[i]->compareToRobust(alpha_z_ran);
    if (c == -1) n_less++;
    else if (c == 0) n_equal++;
    else n_greater++;
  }
  return true;
}

// returns truth value of TExtAtom B evaluated at (X,alpha), i.e. TRUE or FALSE
// P2sign contains signs of poly's at (X,alpha).  If signs are already there, evalAtomAtPoint uses them.  Otherwise,
// it comptues the sign and adds a record to P2sign.
int evalExtAtomAtPoint(VarOrderRef X, GCWord alpha, TExtAtomRef B, map<IntPolyObj*,int > &P2sign)
{
  int n_less = 0, n_equal = 0, n_greater = 0;
  bool nonnullified = true;
  for(map<IntPolyRef,int>::iterator itr = B->factorsBegin(); itr != B->factorsEnd(); ++itr)
  {
    IntPolyObj* p = &(*(itr->first));
    int s = X->partialEval(p,alpha,X->level(p))->signIfConstant();      
    P2sign[p] = s;
    if (nonnullified)
      nonnullified = X->partialEvalRoot(p, alpha, X->level(p), n_less, n_equal, n_greater);
  }
  return nonnullified && B->detTruth(n_less,n_equal,n_greater) ? TRUE : FALSE;
}

// returns truth value of A evaluated at (X,alpha), i.e. TRUE or FALSE
// P2sign contains signs of poly's at (X,alpha).  If signs are already there, evalAtomAtPoint uses them.  Otherwise,
// it comptues the sign and adds a record to P2sign.
int evalAtomAtPoint(VarOrderRef X, GCWord alpha, TAtomRef A, map<IntPolyObj*,int > &P2sign)
{
  int s = A->F->signOfContent();
  for(map<IntPolyRef,int>::iterator itr = A->F->MultiplicityMap.begin(); itr != A->F->MultiplicityMap.end(); ++itr)
  {
    int sf = UNDET;
    IntPolyObj* p = &(*(itr->first)); 
    map<IntPolyObj*,int >::iterator p2itr = P2sign.find(p);
    if (p2itr == P2sign.end())
      sf = P2sign[p] = X->partialEval(p,alpha,X->level(p))->signIfConstant();      
    else
      sf = p2itr->second;
    if (itr->second % 2 == 0) sf = sf*sf;
    s = sf*s;
  }
  return signSatSigma(s,A->relop);
}

int evalStrictConjunctionAtPoint(VarOrderRef X, GCWord alpha, TAndRef C)
{
  map<IntPolyObj*,int > P2sign;
  int sat = TRUE;
  for(TAndObj::conjunct_iterator itr = C->conjuncts.begin(); itr != C->conjuncts.end() && sat == TRUE; ++itr)
  {
    TAtomRef A = asa<TAtomObj>(*itr);
    TExtAtomRef B = asa<TExtAtomObj>(*itr);
    if (A.is_null() && B.is_null()) { throw TarskiException("evalStrictConjunctionAtPoint requires conjunction of atomic/extended-atomic formulas."); }
    if (!A.is_null())
      sat = evalAtomAtPoint(X,alpha,A,P2sign);
    else
      sat = evalExtAtomAtPoint(X,alpha,B,P2sign);
  }
  return sat;
}

int countUNSATStrictConjunctionAtPoint(VarOrderRef X, GCWord alpha, TAndRef C)
{
  int count = 0;
  map<IntPolyObj*,int > P2sign;
  for(TAndObj::conjunct_iterator itr = C->conjuncts.begin(); itr != C->conjuncts.end(); ++itr)
  {
    TAtomRef A = asa<TAtomObj>(*itr);
    TExtAtomRef B = asa<TExtAtomObj>(*itr);
    if (A.is_null() && B.is_null()) { throw TarskiException("evalStrictConjunctionAtPoint requires conjunction of atomic formulas."); }
    if (!A.is_null() && evalAtomAtPoint(X,alpha,A,P2sign) == FALSE || !B.is_null() && evalExtAtomAtPoint(X,alpha,B,P2sign) == FALSE)
      ++count;
  }
  return count;
}

}//end namespace tarski
