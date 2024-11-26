#include "bexpr.h"

namespace tarski {

BExpRef mkexp(const BExpRef &x, int k) {
  if (!ISATOM(k)) { throw BExpException("Exponent too large in mkexp!"); }
  if (k < 0) throw BExpException("Only positive exponents in mkexp!");
  else if (k > 1) return mkfun(opexp,x,mknum(k));
  return k == 1 ? x : (BExpRef)mknum(1);
}

std::string pp(const BExpRef &E);
  
// context is an opcode - note this is for *terms* 
std::string ppp(const BExpRef &E, int context) {
  // pretty print parens - decide whether to wrap parens around
  // something depending on the outside context.
  bool addparens = true;
  if (context == opa)
      addparens = false;
  else if (context == opm && E->op() != opa) 
    addparens = false;
  else if (E->op() == opvar || E->op() == opnum && E->isint()) 
    addparens = false;
  return addparens ? "(" + pp(E) + ")" : pp(E);
}

// context is an opcode - note this is for predicates / logical ops
std::string pppP(const BExpRef &E, int context) {
  // pretty print parens - decide whether to wrap parens around
  // something depending on the outside context.
  bool addparens = true;
  if (E->op() == UNIVER || E->op() == EXIST)
    return pp(E);
  
  if (E->op() == NOTOP && context == NOTOP)
    addparens = false;
  if (context == OROP)
    addparens = false;
  else if (context == ANDOP && (E->op() == ANDOP || E->op() == NOTOP || E->op() < 8 /* relop */))
    addparens = false;
  else if (E->op() == opfalse || E->op() == optrue || E->op() == opundet) 
    addparens = false;

  return addparens ? "[" + pppP(E,OROP) + "]" : pp(E);
}

  
std::string pp(const BExpRef &E) {
  int op = E->op();
  switch(op) {
  case opnum: case opvar:
    return E->str();
  case opn:
    return "-" + ppp(E->arg(0),opn);
  case opinv:
    return "1/" + ppp(E->arg(0),opinv);
  case opexp:
    return ppp(E->arg(0),opexp) + "^" + ppp(E->arg(1),opexp);
  case opm: case opa: {
    std:string res = ppp(E->arg(0),op);
    for(int i = 1; i < E->nargs(); i++)
      res += (op == opm ? "*" : "+") + ppp(E->arg(i),op);
    return res; }
  case opdiv:
    return ppp(E->arg(0),opinv) + "/" + ppp(E->arg(1),opinv);
  case NOOP: case LTOP: case EQOP: case LEOP: case GTOP: case NEOP: case GEOP:
    return ppp(E->arg(0),opa) + " " + opstr(op) + " " + ppp(E->arg(1),opa);
  case NOTOP:
    return "~" + pppP(E->arg(0),op);
  case ANDOP: case OROP: case RIGHTOP: case LEFTOP: case EQUIOP:
    return pppP(E->arg(0),op) + " " + opstr(op) + " " + pppP(E->arg(1),op);
  case UNIVER: case EXIST: {
    std::string res = "[" + opstr(op) + " " + E->arg(0)->str();
    for(int i = 1; i < E->nargs() - 1; i++)
      res += ("," + E->arg(i)->str());
    res += (pppP(E->arg(E->nargs()-1),-1) + "]");
    return res;
  }
  default:
    throw BExpException("Unknown op code " + to_string(op) + " in pp!");
  }
}

// this is meant to be top-level, so I am forcing outer parens!
std::string ppstr(const BExpRef &E) {
  if (E->op() >= 30) return ppp(E,-1);
  else return pppP(E,-1);
}
  
BExpRef expexpand(BExpRef E, int k) {
  // exponential expand.  Expandds E^k
  // expand(E,k) requires that the arguments of E are
  // already exp-expanded and flattened! Also no k <= 0
  // NOTE: This makes sense, since flatten also expexpands
  //       as it goes.
  if (!ISATOM(k)) { throw BExpException("Exponent too large in expexpand!"); }
  if (k <= 0)
    throw BExpException("expexpand requires positive exponent");
  int op = E->op();  
  switch(op) {
  case opnum:
    return E->num()->pow(k);
  case opvar:
    return mkexp(E,k);
  case opn:
    return k % 2 == 0 ? expexpand(E->arg(0),k) : (BExpRef)mkfun(opn,expexpand(E->arg(0),k));
  case opexp: {
    if (!E->arg(1)->isint()) throw BExpException("'^' 2nd argument must be an int");
    integer j = IPROD(E->arg(1)->num()->val(),k); // OK: this checks for int
    return mkexp(E->arg(0),j); // if this is not a BDigit, mkexp will throw exception
  }
  case opm: {
    std::vector<BExpRef> X(E->nargs());
    for(int i = 0; i < E->nargs(); i++)
      X[i] = expexpand(E->arg(i),k);
    return mkfun(opm,X);
  }
  case opa: {
    //std::cerr << "In opa for E = " << pp(E) << std::endl;
    int deg = k;
    int nterms = E->nargs();
    std::vector< std::vector<integer> > evecs;
    pgen(deg,nterms,evecs);
    /*
    std::cerr << "deg = " << deg << " nterms = " << nterms << std::endl;
    for(int i = 0; i < evecs.size(); i++) {
      for(int j = 0; j < nterms; j++) {
	std::cerr << evecs[i][j] << " ";
      }
      std::cerr << std::endl;
    }
    */
    vector<BExpRef> terms;
    for(int i = 0; i < evecs.size(); i++) {
      integer cf = multinomial_coefficient(deg,evecs[i]);      
      vector<BExpRef> nexttermfactors;
      nexttermfactors.push_back(mknum(cf));
      for(int j = 0; j < nterms; j++) {
	if (evecs[i][j] != 0) {
	  BExpRef t = expexpand(E->arg(j),evecs[i][j]);
	  nexttermfactors.push_back(t);
	}
      }
      terms.push_back(mkfun(opm,nexttermfactors));
      //std::cerr << "TERM = " << pp(mkfun(opm,nexttermfactors)) << std::endl;
    }
    return mkfun(opa,terms);
  }
  case opinv:
  case opdiv:
    return mkexp(E,k);
  default:
    throw BExpException("Unknown op type in expexpand");
  }
}

BExpRef negative(BExpRef E) {
  switch(E->op()) {
  case opn:
    return E->arg(0);
  case opnum:
    return E->num()->negative();
  case opdiv:
    return mkfun(opdiv,negative(E->arg(0)),E->arg(1));
  case opa: {
    int N = E->nargs();
    std::vector<BExpRef> X(N);
    for(int i = 0; i < E->nargs(); i++)
      X[i] = negative(E->arg(i));
    return mkfun(opa,X); }
  case opm: {
    int i_neg = -1, i_num = -1, N = E->nargs();
    std::vector<BExpRef> X(N);
    for(int i = 0; i < N; i++) {
      X[i] = E->arg(i);
      if (i_neg == -1 && X[i]->op() == opn) { i_neg = i; }
      else if (i_num == -1 && X[i]->op() == opnum) { i_num = i; }
    }
    if (i_neg != -1)      { X[i_neg] = negative(X[i_neg]); }
    else if (i_num != -1) { X[i_num] = negative(X[i_num]); } 
    else                  { X.insert(X.begin(),mknum(-1)); }
    return mkfun(opm,X);
  }
  default:
    return mkfun(opn,E);
  }
}

  bool hasVar(BExpRef E) {
    switch(E->op()) {
    case opvar: return true;
    case opnum: return false;
    default:
      for(int i = 0; i < E->nargs(); i++)
	if (hasVar(E->arg(i)))
	  return true;
    }
    return false;
  }
  
  bool hasNonConstDenom(BExpRef E) {
    switch(E->op()) {
    case opvar: case opnum: return false;
    case opdiv: return hasVar(E->arg(1));
    case opinv: return hasVar(E->arg(0));
    default:
      for(int i = 0; i < E->nargs(); i++)
	if (hasNonConstDenom(E->arg(i)))
	  return true;
    }
    return false;
  }
  
// must guarantee that flatten ensures that if an "inv" is within
// an exponential, the exp is an immediate parent unless it is
// outside of the inv's inv-ancestor
BExpRef flatten(BExpRef E) {
  if (E->op() == opvar || E->op() == opnum)
    return E;
  const int N = E->nargs();
  std::vector<BExpRef> args(N);
  for(int i = 0; i < N; i++)
    args[i] = flatten(E->arg(i));

  BExpRef R;
  int op = E->op();
  switch(op) {
  case opn:
    R = negative(flatten(args[0]));
    break;
  case opa: {
    std::vector<BExpRef> X;
    BNumRef c = mknum(0);
    for(int i = 0; i < args.size(); i++) {
      if (args[i]->op() == opa) {
	for(int j = 0; j < args[i]->nargs(); j++) {
	  BExpRef z = args[i]->arg(j);
	  if (z->isnum())
	    c = BNumObj::sum(c,z->num());
	  else 
	    X.push_back(z);
	}
      }
      else
	if (args[i]->isnum()) c = BNumObj::sum(c,args[i]->num()); else  X.push_back(args[i]);
    }
    if (X.size() == 0)
      R = c;
    else {
      X.push_back(c);
      R = mkfun(opa,X);
    }
  } break;
  case opm: {
    // go through all integer factors and pull them out
    std::vector<BExpRef> targs;
    BNumRef c = mknum(1);
    for(int i = 0; i < args.size(); i++)
      if (args[i]->isnum())
	c = BNumObj::mul(c,args[i]->num());
      else
	targs.push_back(args[i]);
    
    if (c->isint() && c->num()->val() == 0 && !hasNonConstDenom(E))
      R = c;
    else {    
      bool fsflag = false;
      std::vector<BExpRef> X;
      if (c->isnum() && c->num()->val() != 1)
	X.push_back(c);
      for(int i = 0; i < targs.size(); i++) {
	if (targs[i]->op() == opm) {
	  for(int j = 0; j < targs[i]->nargs(); j++)
	    X.push_back(targs[i]->arg(j));
	}
	else if (targs[i]->op() != opa)
	  X.push_back(targs[i]);
	else {
	  fsflag = true;
	  BExpRef foundsum = targs[i];
	  std::vector<BExpRef> newsumargs;
	  for(int j = 0; j < foundsum->nargs(); j++) {
	    std::vector<BExpRef> tmp(X);
	    tmp.push_back(foundsum->arg(j));
	    for(int k = i+1; k < targs.size(); k++)
	      tmp.push_back(targs[k]);
	    newsumargs.push_back(flatten(mkfun(opm,tmp)));
	  }
	  R = mkfun(opa,newsumargs);
	  break;
	}
      }
      if (!fsflag) { R = mkfun(opm,X); }
    }
    /*
    std::cerr << "In FLATTEN! " << pp(E) << std::endl;
    std::cerr << "### " << R->rstr() << std::endl;
    std::cerr << ">>> " << pp(R) << std::endl;
    */
    } break;
  case opexp:
    if (!args[1]->isint())
      throw BExpException("Error in flatten!  opexp requires an integer second argument.");
    R = expexpand(args[0],args[1]->num()->val()); // OK! This checks for int
    break;
  case opdiv: {
    BExpRef numer = args[0];
    BExpRef denom = args[1];
    if (numer->isnum() && denom->isnum()) { return BNumObj::div(numer->num(),denom->num()); }
    if (numer->isint() && numer->num()->val() == 1) // OK! This checks for int
      R = mkfun(opinv,denom);
    else
      R = flatten(mkfun(opm, numer, mkfun(opinv,denom)));
  } break;
  default:
    R = mkfun(op,args);
  }
  return R;
}

/**
 * 
 **/
// Track (E,k) means E is on stack S with its index k child on top of it on S
class TrackObj; typedef GC_Hand<TrackObj> TrackRef;
class TrackObj : public GC_Obj {
public:
  BExpRef E; int k;
  TrackObj(BExpRef _E, int _k) : E(_E), k(_k) { }
};

class DenMapObj; typedef GC_Hand<DenMapObj> DenMapRef;
class DenMapObj : public GC_Obj {
public:
  struct Entry { BExpRef p; int e; Entry(BExpRef _p, int _e) : p(_p), e(_e) { } };
private:
  TrackRef T;
  int N;
  std::vector< vector<Entry> > F;
public:
  DenMapObj(TrackRef _T) : T(_T), N(T->E->op() == opa ? T->E->nargs() : 1), F(N) { }
  void put(BExpRef poly, int exp) {
    F[T->E->op() == opa ? T->k : 0].push_back(Entry(poly,exp));   
  }
  TrackRef getTrack() { return T; }
  BExpRef allprod() {
    return prodForTerm(-1);
    // vector<BExpRef> P;
    // for(int i = 0; i < F.size(); i++)
    //   for(int j = 0; j < F[i].size(); j++) {
    // 	P.push_back(F[i][j].e == 1 ? F[i][j].p : (BExpRef)mkexp(F[i][j].p,F[i][j].e));
    //   }
    // return P.size() == 0 ? (BExpRef)mknum(1) : (P.size() == 1 ? P[0] : (BExpRef)mkfun(opm,P));
  }
  BExpRef prodForTerm(int index) {
    vector<BExpRef> P;
    for(int i = 0; i < F.size(); i++) {
      if (index != i) 
	for(int j = 0; j < F[i].size(); j++) {
	  P.push_back(F[i][j].e == 1 ? F[i][j].p : (BExpRef)mkexp(F[i][j].p,F[i][j].e));
	}
    }
    return P.size() == 0 ? (BExpRef)mknum(1) : (P.size() == 1 ? P[0] : (BExpRef)mkfun(opm,P));	 
  }
  BExpRef clear() {
    BExpRef E = T->E;
    if (E->op() != opa)
      return E;
    else {
      int N = E->nargs();
      std::vector<BExpRef> X(N);
      for(int i = 0; i < N; i++)
	X[i] = mkfun(opm,prodForTerm(i),E->arg(i));
      return mkfun(opa,X);
    }
  }
};

// It is crucial that Etop is fully flattened!
// sideClear(Etop) returns vector res such that
// 1) Etop is equal to res[0]/res[1] as long as
//    res[2], res[3], ... are all non-zero, and
// 2) if any of res[2], res[3], ... are zero,
//    then partialEval (from my paper) of Etop
//    would be fail (i.e. this is my algorithm)
std::vector<BExpRef> sideClear(BExpRef Etop) {
  std::vector<BExpRef> denominatorPolys;
  std::stack<TrackRef> S;
  std::stack<DenMapRef> Smap;
  TrackRef t = new TrackObj(Etop,-1);
  S.push(t);
  Smap.push(new DenMapObj(t));
  while(!S.empty()) {
    TrackRef T = S.top();
    /** no children processed **/
    if (T->k == -1) { 
      if (T->E->op() == opnum || T->E->op() == opvar)
	S.pop();
      else if (T->E->op() != opinv) {
	T->k++;
	S.push(new TrackObj(T->E->arg(T->k),-1));
      }
      else { // this is opinv
	T->k++;	
	S.push(new TrackObj(T->E->arg(T->k),-1));
	Smap.push(new DenMapObj(S.top()));
      }
    }
    /** some children processed **/
    else { // note: opnum and opvar not possible here!
      if (T->E->op() != opinv) {
	T->k++;
	if (T->k < T->E->nargs())
	  S.push(new TrackObj(T->E->arg(T->k),-1));
	else 
	  S.pop();
      }
      else { // this is opinv
	T->k++;
	if (T->k < T->E->nargs()) // this is its one and only child
	  S.push(new TrackObj(T->E->arg(T->k),-1));
	else {
	  DenMapRef D = Smap.top();
	  Smap.pop();
	  S.pop();
	  
	  // Put the clear of this div into the next higher DenMap	  	  
	  BExpRef toPutPoly = D->clear();
	  denominatorPolys.push_back(toPutPoly);
	  int toPutExponent = 1;
	  if (!S.empty() && S.top()->E->op() == opexp) {
	    if (!S.top()->E->arg(1)->isint()) { throw BExpException("Error! non-int exponent"); }
	    toPutExponent = S.top()->E->arg(1)->num()->val(); // OK! this checks for int
	  }
	  Smap.top()->put(toPutPoly,toPutExponent);
	  /*
	  std::cerr << "Just put: "
		    << pp(toPutPoly) << std::endl;
	  std::cerr << "alprod is " << pp(D->allprod()) << std::endl<< std::endl;
	  */
	  if (!S.empty()) // Replace this div with 1
	    S.top()->E->arg(S.top()->k) = D->allprod();
	}
      }
    }    
  }
  DenMapRef D = Smap.top();
  BExpRef poly = D->clear();
  BExpRef den = D->allprod();
  /*
  std::cerr << "FINAL num: "
	    << pp(poly) << std::endl;	  
  std::cerr << "FINAL den: "
	    << pp(den) << std::endl;
  */
  std::vector<BExpRef> res({poly,den});
  for(auto itr = denominatorPolys.begin(); itr != denominatorPolys.end(); ++itr)
    res.push_back(*itr);  
  return res;
}

// Returns vector res such that res[0] relop res[1] is the atomic formula
// with the denominators cleared, and the remaining elements of res are the
// denominator polynomials.
vector<BExpRef> translate(BExpRef LHS, int relop, BExpRef RHS) {
  BExpRef mLHS = flatten(LHS); if (mLHS->op() != opa) mLHS = mkfun(opa,mLHS);
  BExpRef mRHS = flatten(RHS); if (mRHS->op() != opa) mRHS = mkfun(opa,mRHS);
  std::vector<BExpRef> Vlhs = sideClear(mLHS);
  std::vector<BExpRef> Vrhs = sideClear(mRHS);
  BExpRef newLHS, newRHS;
  if (relop == EQOP || relop == NEOP) {
    newLHS = flatten(mkfun(opm,Vrhs[1],Vlhs[0]));
    newRHS = flatten(mkfun(opm,Vlhs[1],Vrhs[0]));
  }
  else {
    newLHS = flatten(mkfun(opm,{mkexp(Vrhs[1],2),Vlhs[1],Vlhs[0]}));
    newRHS = flatten(mkfun(opm,{mkexp(Vlhs[1],2),Vrhs[1],Vrhs[0]}));
  }

  // Prepare output
  std::vector<BExpRef> res({newLHS,newRHS});
  for(int i = 2; i < Vlhs.size(); i++)
    res.push_back(Vlhs[i]);
  for(int i = 2; i < Vrhs.size(); i++)
    res.push_back(Vrhs[i]);
  return res;
}


  // (c,p) - where c is rational and p is an integral polynmomial
  // represent zero as (c,0) where c is non-zero, so c should always
  // be non-zero!
  class RPPair {
  public:
    BNumRef c;
    IntPolyRef p;
    RPPair() { } // null references
    RPPair(BNumRef _c, IntPolyRef _p) : c(_c), p(_p) {
      if (p->isZero()) { c = new BNumObj(1); }
      else if (c->isint() && c->val() == 0) { c = new BNumObj(1); p = new IntPolyObj(0); }
      else { }
    }
    static RPPair mul(const RPPair &a, const RPPair &b, PolyManager* pM) {
      return RPPair(BNumObj::mul(a.c, b.c), pM->prod(a.p, b.p));
    }
    static RPPair sum(const RPPair &a, const RPPair &b, PolyManager* pM) {
      bool ia = a.c->isint(), ib = b.c->isint();
      if (ia && ib) {
	VarSet S;
	Word ac = a.c->val(), bc = b.c->val();
	IntPolyRef pac = new IntPolyObj(0,ac,S);
	IntPolyRef pbc = new IntPolyObj(0,bc,S);
	IntPolyRef resp = pM->sum(pM->prod(pac,a.p),pM->prod(pbc,b.p));
	RPPair res(new BNumObj(1),resp);
	return res;
      }
      else
	throw BExpException("RPPair::sum this not yet implemented!");
    }
    static RPPair neg(const RPPair &a) {
      return RPPair(a.c->negative(),a.p);
    }	  
  };

  // E needs to be free of non-constant denominators
  // returns (c,p) where c is a BNumRef and p an IntPolyRef
  // and c*p equals E
  RPPair convertToIntPoly(BExpRef E, PolyManager *pM) {
    if (E->op() == opvar)
      return RPPair(new BNumObj(1), new IntPolyObj(pM->addVar(E->str())));	
    if (E->op() == opnum)
      return RPPair(E,new IntPolyObj(1));

    vector<RPPair> args;
    for(int i = 0; i < E->nargs(); i++)
      args.push_back(convertToIntPoly(E->arg(i),pM));
    
    switch(E->op()) {
    case opinv: {
      if (!args[0].p->isConstant()) { throw BExpException("Can't convert expression with non-const denominator into poly");}
      if (!args[0].p->isZero()) { throw BExpException("Divide by zero!"); }
      BNumRef one = new BNumObj(1), p = new BNumObj(args[0].p->valueIfConstant());
      BNumRef c =  BNumObj::div(one,BNumObj::mul(args[0].c,p));
      return RPPair(c,new IntPolyObj(1));
    }
    case opm: {
      RPPair res(new BNumObj(1),new IntPolyObj(1));
      for(int i = 0; i < args.size(); i++)
	res = RPPair::mul(res,args[i],pM);
      return res;
    }
    case opa: {
      RPPair res(new BNumObj(1),new IntPolyObj(0));
      for(int i = 0; i < args.size(); i++)
	res = RPPair::sum(res,args[i],pM);
      return res;
    }
    case opn:
      return RPPair(args[0].c->negative(),args[0].p);
    case opexp: {
      if (!args[1].p->isConstant()) {  throw BExpException("Error! non-constant exponent."); }
      integer k;
      Word C = args[1].c->val(), P = args[1].p->valueIfConstant();
      if (args[1].c->isint()) {
	k = IPROD(C,P);
      }
      else {
	Word N = FIRST(C), D = SECOND(C), Q, R;
	IQR(P,D,&Q,&R);
	if (R != 0) {  throw BExpException("Error! non-integer exponent."); }
	k = IPROD(N,Q);
      }
      if (!ISATOM(k)) { throw BExpException("Error! exponent too large."); }
      if (k < 0) { throw BExpException("Error! non-constant exponent."); }
	
      if (k == 0) { return RPPair(new BNumObj(1),new IntPolyObj(1)); }
      RPPair res = args[0];
      for(int i = 1; i < k; i++)
	res = RPPair::mul(res,args[0],pM); // NOTE: not efficient ... oh well!
      return res;
    }
    default:
      throw BExpException("Not yet implemented in foobar!");    
    }
  }

  // returns IntPoly similar to (i.e. const multiple of ) E
  IntPolyRef pullOutIntPoly(BExpRef E, PolyManager *pM) {
    RPPair res = convertToIntPoly(E,pM);
    return res.p;
  }
  
  TAtomRef convertBExpToTAtom(BExpRef E, PolyManager *pM) {
    int relop = E->op();
    BExpRef LHS = E->arg(0);
    BExpRef RHS = E->arg(1);
    RPPair res = convertToIntPoly(mkfun(opa,LHS,mkfun(opn,RHS)),pM);
    IntPolyRef p = res.p;
    if (res.c->sign() < 0) {
      p = p->integerProduct(-1);
      relop = reverseRelop(relop);
    }
    TAtomRef A = makeAtom(*pM,p,relop);
    return A;
  }

  const std::string BFunObj::str() { return pp(this); }


}
