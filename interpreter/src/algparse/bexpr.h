#ifndef _BEXPR_
#define _BEXPR_

#include "multinomial.h"
#include "GC_System/GC_System.h"
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <stack>
#include "formrepconventions.h"
#include "../poly/poly.h"
#include "../poly/polymanager.h"
#include "../formula/formula.h"

namespace tarski {

typedef GCWord rational;

class BExpException : public std::exception
{
private:
  std::string msg;
public:
  BExpException(const std::string &msg) throw() : msg(msg) { }
  ~BExpException() throw() { }
  virtual const char* what() const throw() { return msg.c_str(); }
};

/*
# [op, arg1, ..., argk]
# +  : addition k-ary
# -  : negation, unary
# *  : multiplication
# inv: multiplicative inverse, unary
# div: division, binary (get rid of first!)
# ^  : exponentiation - binary, second arg must be positive int
# var: unary
# num: unary (for int), binary (for rational)

### op ranges ###
0-7  : relops
8-10 : false, true, undet
11-16: logical ops, e.g. ANDOP, NOTOP, etc
20-21: quantifiers
31-x : real number vars, constants and operators
*/
const std::string _opstr[] = {
  "","var","num","+","-","*","inv","div","^"
};
const int opvar = 31, opnum = 32, opa = 33, opn = 34, opm = 35,
  opinv = 36, opdiv = 37, opexp = 38;

const std::string _boolConstStr[] = {
  "","","","","","","","","false","true","undet" };

const int opfalse = 8, optrue = 9, opundet = 10;

inline bool isQuantifier(int op) { return 20 <= op && op <= 21; }
inline bool isRelop(int op) { return 0 <= op && op <= 7; }
  
inline const std::string opstr(int k) {
  if (k < 8) return relopString(k);
  else if (k < 10) return _boolConstStr[k];
  else if (k < 20) return logopStrings[k];
  else if (k <= 30) return k == 20 ? "all" : "ex";
  else return _opstr[k-30];
}

class BExpObj;
typedef GC_Hand<BExpObj> BExpRef;
class BVarObj;
typedef GC_Hand<BVarObj> BVarRef;
class BNumObj;
typedef GC_Hand<BNumObj> BNumRef;
class BFunObj;
typedef GC_Hand<BFunObj> BFunRef;

class BExpObj : public GC_Obj {
public:
  virtual int op() const = 0;
  const virtual std::string str() = 0;
  const virtual std::string rstr() = 0;
  virtual bool isint() const { return false; }
  virtual bool isnum() const { return false; }
  virtual BExpRef& arg(int i) { throw BExpException("BExp arg undefined!"); }
  virtual int nargs() { return 0; }
  virtual BNumRef num() { throw BExpException("num() undefined."); }
};

class BFunObj  : public BExpObj {
private:
  int _op;
  std::vector<BExpRef> args;
public:
  BFunObj(int op, const BExpRef &x) : _op(op) { args.push_back(x); }
  BFunObj(int op, const BExpRef &x, const BExpRef &y) : _op(op) {
    args.push_back(x);
    args.push_back(y);
  }
  BFunObj(int op, const std::vector<BExpRef> &V) : _op(op), args(V) { }
  virtual int op() const { return _op; }
  const std::string str();
  const virtual std::string rstr() {
    std::string res = "['" + opstr(op()) + "'";
    for(int i = 0; i < args.size(); i++) {
      res += "," + args[i]->rstr();
    }
    res += "]";
    return res;
  }
  BExpRef& arg(int i) { return args[i]; }
  int nargs() { return args.size(); }
};

class BVarObj : public BExpObj {
private:
  std::string _name;
public:
  BVarObj(const string &name) : _name(name) { }
  int op() const { return opvar; }
  const std::string str() { return _name; }
  const std::string rstr() { return "['var','" + _name + "']"; }
};

const rational BNUM_r_null = -1;
class BNumObj : public BExpObj {
private:
  // if _r == r_null, this is an int represented by _n
  // otherwise, this is a rational represented by _r
  // NOTE: only a rational if not zero AND denom is not 1
  integer _n;
  rational _r; 
  BNumObj(integer n, rational r) : _n(n), _r(r) { } // Only use if you know what you're doing!
public:
  BNumObj(const integer &n) : _n(n), _r(BNUM_r_null) { }
  int op() const { return opnum; }
  const std::string str() {
    return isint() ? integerToString(_n) : integerToString(FIRST(_r)) + "/" + integerToString(SECOND(_r));
  }
  const std::string rstr() { return "['num'," + str() + "]"; }
  bool isint() const { return _r == BNUM_r_null; }
  bool isnum() const { return true; }
  BNumRef num() { return BNumRef(this); }
  integer val() const { return isint() ? _n : _r; }
  int sign() const { return isint() ? ISIGNF(_n) : RNSIGN(_r); }
  BNumRef negative() const { return isint() ? new BNumObj(INEG(_n)) : new BNumObj(BETA,RNNEG(_r)); }
  BNumRef pow(int k) {
    if (!ISATOM(k) || k < 0) { throw BExpException("Error! invalid exponent in BNumObj::pow"); }
    if (k == 0) { return new BNumObj(1); }
    if (k == 1) { return this; }
    if (isint()) { return new BNumObj(power(_n,k)); }
    integer numer = FIRST(_r), denom = SECOND(_r);
    return new BNumObj(BETA,LIST2(power(numer,k),power(denom,k)));
  }
  static BNumRef fromNumerDenom(integer numer, integer denom) {
    if (denom == 0) { throw BExpException("Zero denominator in fromNumerDenom!"); }
    if (numer == 0 || denom == 1) { return new BNumObj(numer); }
    Word R = RNRED(numer,denom);
    return SECOND(R) == 1 ? new BNumObj(FIRST(R)) : new BNumObj(BETA,R);      
  }
  static BNumRef sum(BNumRef a, BNumRef b) {
    bool ia = a->isint(), ib = b->isint();
    if (ia && ib) { return new BNumObj(ISUM(a->val(),b->val())); }
    Word ra = ia ? RNINT(a->val()) : (Word)a->val();
    Word rb = ib ? RNINT(b->val()) : (Word)b->val();
    Word res = RNSUM(ra,rb);
    if (res == 0) return new BNumObj(0);
    return SECOND(res) == 1 ? new BNumObj(FIRST(res)) : new BNumObj(BETA,res);
  }
  static BNumRef mul(BNumRef a, BNumRef b) {
    bool ia = a->isint(), ib = b->isint();
    if (ia && ib) { return new BNumObj(IPROD(a->val(),b->val())); }
    Word ra = ia ? RNINT(a->val()) : (Word)a->val();
    Word rb = ib ? RNINT(b->val()) : (Word)b->val();
    Word res = RNPROD(ra,rb);
    if (res == 0) return new BNumObj(0);
    return SECOND(res) == 1 ? new BNumObj(FIRST(res)) : new BNumObj(BETA,res);
  }
  static BNumRef div(BNumRef a, BNumRef b) {
    bool ia = a->isint(), ib = b->isint();
    if (ia && ib) { return fromNumerDenom(a->val(),b->val()); }
    Word ra = ia ? RNINT(a->val()) : (Word)a->val();
    Word rb = ib ? RNINT(b->val()) : (Word)b->val();
    Word res = RNPROD(ra,RNINV(rb));
    if (res == 0) return new BNumObj(0);
    return SECOND(res) == 1 ? new BNumObj(FIRST(res)) : new BNumObj(BETA,res);
  }
};

// These are what I'm expecting outside code to use to create BExpObj's
inline BNumRef mknum(const integer x) { return new BNumObj(x); }
inline BVarRef mkvar(const string& s) { return new BVarObj(s); }
inline BFunRef mkfun(int op, const BExpRef &x) { return new BFunObj(op,x); }
inline BFunRef mkfun(int op, const BExpRef &x, const BExpRef &y) { return new BFunObj(op,x,y); }
inline BExpRef mkfun(int op, const std::vector<BExpRef> &X) {
  if (op == opm && X.size() == 0) return new BNumObj(1);
  if (op == opm && X.size() == 1) return X[0];
  if (op == opa && X.size() == 0) return new BNumObj(0);
  if (op == opa && X.size() == 1) return X[0];
  return new BFunObj(op,X);
}

BExpRef mkexp(const BExpRef &x, int k);

std::string ppstr(const BExpRef &E);

BExpRef expexpand(BExpRef E, int k);
  // exponential expand.  Expandds E^k
  // expand(E,k) requires that the arguments of E are
  // already exp-expanded and flattened! Also no k <= 0
  // NOTE: This makes sense, since flatten also expexpands
  //       as it goes.

BExpRef negative(BExpRef E);

// must guarantee that flatten ensures that if an "inv" is within
// an exponential, the exp is an immediate parent unless it is
// outside of the inv's inv-ancestor
BExpRef flatten(BExpRef E);

// It is crucial that Etop is fully flattened!
// sideClear(Etop) returns vector res such that
// 1) Etop is equal to res[0]/res[1] as long as
//    res[2], res[3], ... are all non-zero, and
// 2) if any of res[2], res[3], ... are zero,
//    then partialEval (from my paper) of Etop
//    would be fail (i.e. this is my algorithm)
std::vector<BExpRef> sideClear(BExpRef Etop);

// Returns vector res such that res[0] relop res[1] is the atomic formula
// with the denominators cleared, and the remaining elements of res are the
// denominator polynomials.
vector<BExpRef> translate(BExpRef LHS, int relop, BExpRef RHS);

TAtomRef convertBExpToTAtom(BExpRef E, PolyManager *pM);

// returns IntPoly similar to (i.e. const multiple of ) E
IntPolyRef pullOutIntPoly(BExpRef E, PolyManager *pM);
  
  
}
#endif
