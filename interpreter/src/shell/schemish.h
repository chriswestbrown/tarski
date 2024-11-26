#ifndef _CWBSCHEMISH_
#define _CWBSCHEMISH_
#include <algorithm>
#include <string>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <sstream>
#include <vector>
#include <map>
#include <cctype>
#include "../GC_System/GC_System.h"
#include "../algparse/algparse.h"
#include "../formula/formmanip.h"
#include "../formula/formula.h"
#include "../formula/normalize.h"
#include "../formula/linearSubs.h"
#include "../search/qfr.h"

namespace tarski {

inline std::string toString(int i) { std::ostringstream sout; sout << i; return sout.str(); }
inline std::string toString(void* i) { std::ostringstream sout; sout << i; return sout.str(); }


// type codes
const int _void = 0, _sym = 1, _num = 2, _str = 3, _alg = 4, _tar = 5, _lis = 6, _fun = 7, 
  _clo = 8, _ext = 9, _boo = 10, _uif = 11; // uif = uninterpreted formula
const int _err = -1;

// builtin function codes (0 means not builtin, < 0 means extensions)
const int _f_add = 1, _f_mul = 2, _f_list = 3, _f_eval = 4, _f_map = 5, _f_binred = 6, _f_length = 7,
  _f_cputime = 8, _f_sbt = 9, _f_head = 10, _f_get = 11, _f_quit = 12, _f_msg = 13, _f_version = 14,
  _f_load = 15, _f_equal = 16, _f_display = 17, _f_tail = 18, _f_typename = 19, _f_apply = 20;

class Interpreter;
class SObj;
typedef GC_Hand<SObj> SRef;

class SymObj; typedef GC_Hand<SymObj> SymRef;
class NumObj; typedef GC_Hand<NumObj> NumRef;
class StrObj; typedef GC_Hand<StrObj> StrRef;
class AlgObj; typedef GC_Hand<AlgObj> AlgRef;
class TarObj; typedef GC_Hand<TarObj> TarRef;
class LisObj; typedef GC_Hand<LisObj> LisRef;
class ErrObj; typedef GC_Hand<ErrObj> ErrRef;
class FunObj; typedef GC_Hand<FunObj> FunRef;
class CloObj; typedef GC_Hand<CloObj> CloRef;
class ExtObj; typedef GC_Hand<ExtObj> ExtRef;
class BooObj; typedef GC_Hand<BooObj> BooRef;
class UifObj; typedef GC_Hand<UifObj> UifRef;

class SObj : public GC_Obj
{
public:
  virtual int type() { return _void; }
  virtual std::string typeName() { return "void"; } // don't call if you can help it!
  virtual BooRef boo() { return 0; }
  virtual SymRef sym() { return 0; }
  virtual NumRef num() { return 0; }
  virtual StrRef str() { return 0; }
  virtual AlgRef alg() { return 0; }
  virtual TarRef tar() { return 0; }
  virtual LisRef lis() { return 0; }
  virtual ErrRef err() { return 0; }
  virtual FunRef fun() { return 0; }
  virtual CloRef clo() { return 0; }
  virtual ExtRef ext() { return 0; }
  virtual UifRef uif() { return 0; }
  virtual std::string toStr() { return ":void"; }
  virtual void display(std::ostream& out) { }
  virtual int length() { return -1; }
  virtual void mark(Interpreter *ptrI) { return; }
  virtual BooRef equal(SRef x); // defined below BooObj definition NOTE: always returns false!
};

class BooObj: public SObj
{
public:
  bool val;
  BooObj(bool s) : val(s) {  }
  virtual int type() { return _boo; }
  std::string typeName() { return "boo"; }
  BooRef boo() { return this; }
  std::string toStr() { return std::string(val ? "true" : "false") + ":boo"; }
  void display(std::ostream& out) { out << (val ? "true" : "false"); }
  BooRef equal(SRef x) { BooRef y = x->boo(); return new BooObj(!y.is_null() && val == y->val); }
};

inline BooRef SObj::equal(SRef x) { return new BooObj(false); }

class ErrObj: public SObj { 
public:
  std::string msg;
  ErrObj(const std::string &s) : msg(s) { }
  virtual int type() { return _err; }
  std::string typeName() { return "err"; }
  ErrRef err() { return this; }
  std::string toStr() { return "\"" + msg + "\":err"; }
  void display(std::ostream& out) { out << "Error! " << msg; }
  BooRef equal(SRef x) { ErrRef y = x->err(); return new BooObj(!y.is_null() && msg == y->msg); }
  std::string getMsg() { return msg; }
};

class SymObj: public SObj
{
public:
  std::string val; 
  SymObj(const std::string &s) : val(s) {  }
  virtual int type() { return _sym; }
  std::string typeName() { return "sym"; }
  SymRef sym() { return this; }
  std::string toStr() { return val + ":sym"; }  
  void display(std::ostream& out) { out << val; }
  BooRef equal(SRef x) { SymRef y = x->sym(); return new BooObj(!y.is_null() && val == y->val); }
  std::string getVal() { return val; }
};

class NumObj: public SObj 
{
public:
  GCWord val;
  NumObj(const int &s) : val(RNINT(s)) { }
  NumObj(const GCWord &X) : val(X) { }
  virtual int type() { return _num; }
  std::string typeName() { return "num"; }
  NumRef num() { return this; } 
  Word numerator() { return RNNUM(val); }
  Word getVal() { return val; }
  Word denominator() { return RNDEN(val); }
  void add(NumRef r) { val = RNSUM(val,r->val); }
  void sbt(NumRef r) { val = RNDIF(val,r->val); }
  void mul(NumRef r) { val = RNPROD(val,r->val); }
  std::string toStr() 
  { 
    std::ostringstream sout; 
    PushOutputContext(sout); RNWRITE(val); PopOutputContext();
    sout << ":num"; 
    return sout.str();
  }
  void display(std::ostream& out) { PushOutputContext(out); RNWRITE(val); PopOutputContext(); }
  BooRef equal(SRef x) { NumRef y = x->num(); return new BooObj(!y.is_null() && RNCOMP(val,y->val) == 0); }
};

class StrObj: public SObj { 
public:
  std::string val;
  StrObj(const std::string &s) : val(s) { }
  virtual int type() { return _str; }
  std::string typeName() { return "str"; }
  std::string getVal() { return val; }
  StrRef str() { return this; }
  std::string toStr() { return "\"" + val + "\":str"; }
  int length() { return val.length(); }  
  BooRef equal(SRef x) { StrRef y = x->str(); return new BooObj(!y.is_null() && val == y->val); }
  void display(std::ostream& out) { out << val; }
};

class LisObj: public SObj 
{
private:
  std::vector<SRef> elts;
  short int markVal;
public:
  LisObj() :  markVal(-1) { }
  LisObj(const SRef &x1) :  markVal(-1) { push_back(x1); }
  LisObj(const SRef &x1, const SRef &x2) :  markVal(-1) { push_back(x1); push_back(x2); }
  LisObj(const SRef &x1, const SRef &x2, const SRef &x3) :  markVal(-1)
  {
    push_back(x1); push_back(x2); push_back(x3);
  }
  virtual void mark(Interpreter *pI);
  void push_back(const SRef &s) { elts.push_back(s); }
  virtual int length() { return elts.size(); }
  virtual SRef get(int i) { return elts[i]; }

  virtual int type() { return _lis; }
  std::string typeName() { return "lis"; }
  LisRef lis() { return this; }
  std::string toStr() 
  { 
    std::ostringstream sout; 
    sout << "("; 
    for(int i = 0; i < length(); ++i) sout << " " << get(i)->toStr(); 
    sout << " )"; 
    return sout.str(); 
  }
  void display(std::ostream& out)
  {
    out << "("; 
    if (length() > 0) { get(0)->display(out); }
    for(int i = 1; i < length(); ++i) { out << ' '; get(i)->display(out); }
    out << " )"; 
  }
  bool empty() { return length() == 0; }
  BooRef equal(SRef x) 
  { 
    LisRef y = x->lis(); 
    if (y.is_null() || length() != y->length()) { return new BooObj(false); }
    for(int i = 0; i < length(); i++)
    {
      BooRef t = get(i)->equal(y->get(i));
      if (t->val == false) { return new BooObj(false); }
    }
    return new BooObj(true);
  }
};

class LisTailObj : public LisObj
{
 private:
  LisRef parent;
  int i_start;
 public:
  virtual void mark(Interpreter *pI) { parent->mark(pI); }
  virtual int length() { return parent->length() - i_start; }
  virtual SRef get(int i) { return parent->get(i + i_start); }
  LisTailObj(LisRef r)
  {
    LisTailObj* p = dynamic_cast<LisTailObj*>(&(*r));
    if (p == NULL) { parent = r; i_start = 1; }
    else           { parent = p->parent; i_start = p->i_start + 1; }
  }
};

class FunObj: public SObj { 
public:
  int builtin;
  FunObj(int i) : builtin(i) { } // non-zero value means builtin
  virtual int type() { return _fun; }
  std::string typeName() { return "fun"; }
  FunRef fun() { return this; }
  std::string toStr() { return toString(builtin) + ":fun"; }
  BooRef equal(SRef x) { FunRef y = x->fun(); return new BooObj(!y.is_null() && builtin == y->builtin); }
  void display(std::ostream& out) { out << toString(builtin); }
};

class TarObj: public SObj
{
public:
  TFormRef val;
  TarObj(const TFormRef &s) : val(s) {  }
  virtual int type() { return _tar; }
  std::string typeName() { return "tar"; }
  TarRef tar() { return this; }
  TFormRef getValue() { return val; }
  std::string toStr() 
  {
    std::ostringstream sout;
    PushOutputContext(sout);
    val->write(true);
    PopOutputContext();
    sout << ":tar";
    return sout.str();
  }
  void display(std::ostream& out)
  {   
    PushOutputContext(out);
    val->write(true);
    PopOutputContext();
  }
  BooRef equal(SRef x) 
  { 
    // This is the most simplistic notion of equality for Tarski formulas: print equality
    // At least at this point it is print equality for the normalized version of the
    // formulas.  Is that a good idea?
    // I really should make syntactic equality a test done with a nice "Apply" object so
    // there's some efficiency to it.
    TarRef y = x->tar(); 
    if (y.is_null()) { return new BooObj(false); }
    Level1 basicNormalizer;	  
    TFormRef A; { RawNormalizer R(basicNormalizer); R(this->val); A = R.getRes(); }
    TFormRef B; { RawNormalizer R(basicNormalizer); R(y->val); B = R.getRes(); }
    // A->write(true); cerr << endl;
    // B->write(true); cerr << endl;
    // I think I need the 'true' argument to make this work right!
    std::ostringstream s1, s2;
    PushOutputContext(s1); A->write(true); PopOutputContext();
    PushOutputContext(s2); B->write(true); PopOutputContext();
    return new BooObj(s1.str() == s2.str());
  }
};

class AlgObj: public SObj 
{ 
public:
  IntPolyRef val;
  PolyManager *PMptr;
  AlgObj(const IntPolyRef &s, PolyManager &PM) : val(s), PMptr(&PM) {  }
  virtual int type() { return _alg; }
  std::string typeName() { return "alg"; }
  AlgRef alg() { return this; }
  IntPolyRef getVal() { return val; }
  std::string toStr() 
  {
    std::ostringstream sout;
    PushOutputContext(sout);
    val->write(*PMptr);
    PopOutputContext();
    sout << ":alg";
    return sout.str();
  }
  void display(std::ostream& out)
  {   
    PushOutputContext(out);
    val->write(*PMptr);
    PopOutputContext();
  }
  int length()
  {
    std::ostringstream sout;
    PushOutputContext(sout);
    val->write(*PMptr);
    PopOutputContext();
    return sout.str().length();
  }
  BooRef equal(SRef x) 
  { 
    AlgRef y = x->alg(); 
    if (y.is_null()) { return new BooObj(false); }
    return new BooObj(val->equal(y->val));
  }
};



}
#include "../algparse/uninterpform.h"
namespace tarski {


  
/**************************************************************************
 * BEGIN: Type extension
 **************************************************************************/

class TypeExtensionObj : public GC_Obj
{
public:
  template<class T>
  struct LFH
  {
    typedef SRef (T :: *FooMemFunc)(std::vector<SRef>& args);
    const char* label;
    FooMemFunc func;
    const char* desc;
    bool isNull() const { return label == 0; }
    const char* getLabel() { return label; }
    const char* getDesc() { return desc; }
  };

  class MsgManager
  {
  public:
    virtual bool isNull(TypeExtensionObj &obj, int i) const = 0;
    virtual const char* getLabel(TypeExtensionObj &obj, int i) const = 0;
    virtual const char* getDesc(TypeExtensionObj &obj, int i) const = 0;
    virtual SRef call(TypeExtensionObj &obj, int i, std::vector<SRef>& arg) const = 0;
    virtual int getIndex(TypeExtensionObj &obj, const char* name) const = 0; // returns -1 if not found
  };

  template<class T>
  class AMsgManager : public MsgManager
  {
  public:
    bool isNull(TypeExtensionObj &obj, int i) const { T* ptr = (T*)&obj; return ptr->A[i].isNull(); } 
    const char* getLabel(TypeExtensionObj &obj, int i) const { T* ptr = (T*)&obj; return ptr->A[i].getLabel(); } 
    const char* getDesc(TypeExtensionObj &obj, int i) const { T* ptr = (T*)&obj; return ptr->A[i].getDesc(); } 
    SRef call(TypeExtensionObj &obj, int i,  std::vector<SRef>& arg) const { T* ptr = (T*)&obj; return ((*ptr).*(ptr->A[i].func))(arg); }
    int getIndex(TypeExtensionObj &obj, const char* name) const // returns -1 if not found
    {
      bool isn = false;
      T* ptr = (T*)&obj;
      int i = 0;
      while(!(isn = ptr->A[i].isNull()) && strcmp(ptr->A[i].getLabel(),name) != 0)
	++i;
      return isn ? -1 : i;
    }
  };


  virtual const MsgManager& getMsgManager() = 0;
  bool isMsgNull(int i)             { return this->getMsgManager().isNull(*this,i); }
  const char* getMsgLabel(int i)    { return this->getMsgManager().getLabel(*this,i); }
  const char* getMsgDesc(int i)     { return this->getMsgManager().getDesc(*this,i); }
  int getMsgIndex(const char* name) { return this->getMsgManager().getIndex(*this,name); }
  //  ^^^^^^^^^^^ returns -1 if not found

  virtual std::string name() = 0;
  virtual std::string shortDescrip() = 0;
  virtual std::string display() { return "?"; }

  virtual SRef evalMsg(SymRef msg, std::vector<SRef>& args) ;
  
  virtual bool eqTest(GC_Hand<TypeExtensionObj> b) 
  { throw TarskiException("Equality between given types not supported"); }


};

/**************************************************************************
 * END: Type extension
 **************************************************************************/

class ExtObj: public SObj { 
public:
  GC_Hand<TypeExtensionObj> externObjRef;
  ExtObj(GC_Hand<TypeExtensionObj> ref) : externObjRef(ref) { } 
  virtual int type() { return _ext; }
  std::string typeName() { return externObjRef->name(); }
  ExtRef ext() { return this; }
  std::string toStr() { return externObjRef->display() + ":" + externObjRef->name(); }

  template<class T>
    T* getExternObjPtr() { return dynamic_cast<T*>(&(*externObjRef)); }

  BooRef equal(SRef x) 
  { 
    BooRef res = NULL;
    ExtRef y = x->ext(); 
    if (y.is_null()) { return new BooObj(false); }
    if (this->externObjRef.identical(y->externObjRef)) { return new BooObj(true); }
    try { res = new BooObj(this->externObjRef->eqTest(y->externObjRef)); } 
    catch(TarskiException &e) { throw TarskiException("equal beteween ext objects not yet supported!"); }
    return res;
  }
  void display(std::ostream& out)
  {
    out <<  externObjRef->display();
  }  
};

class Interpreter;
class Frame : public std::map<std::string,SRef>
{
public:
  Frame* parent;
  short int markVal;
  Frame(Interpreter *pint, Frame *_parent);
  
  SRef lookup(const std::string &name)
  {
    Frame::iterator itr = find(name); if (itr != end()) { return itr->second; }
    if (parent == NULL) return new ErrObj("Symbol '" + name + "' not found!");
    return parent->lookup(name);
  }
  SRef set(const std::string &name, SRef value)
  {
    Frame::iterator itr = find(name); if (itr != end()) { itr->second = value; return new SObj(); }
    if (parent == NULL) return new ErrObj("Symbol '" + name + "' not found!");
    return parent->set(name,value);
  }
  void mark(Interpreter *pI);
};

class CloObj: public SObj
{
public:
  LisRef L; // The lambda expression
  Frame* env;
  short int markVal;
  CloObj(Interpreter *pint, LisRef _l, Frame* _env);
  virtual int type() { return _clo; }
  CloRef clo() { return this; }
  std::string toStr() { return "?:clo"; }
  void mark(Interpreter *pI);
};


class Interpreter
{
public:

  Frame* rootFrame;
  PolyManager *PM;
  short int currMark;

  // Garbage collection: reclaiming closures and frames!
  std::vector<Frame*> GCFcreated;
  std::vector<CloRef> GCCcreated;
  void markAndSweep();
  void mark(short int & markVal) { markVal = currMark; }
  bool isMarked(short int markVal) { return markVal == currMark; }
  Interpreter(PolyManager *p) : PM(p), currMark(0) { rootFrame = new Frame(this,0); }
  virtual ~Interpreter() { delete rootFrame; }

  PolyManager* getPolyManagerPtr() { return PM; }

  SRef next(std::istream &in);  
  SRef eval(Frame* env, SRef input);
  virtual SRef exFunEval(SRef input, int funcode,std::vector<SRef> &args) { return new ErrObj("Unkown function."); }
  SRef readnum(std::istream &in);
  SRef readstring(std::istream &in);
  SRef readsym(std::istream &in);
  SRef readcomment(std::istream &in);
  SRef readlist(std::istream &in);  
  SRef readTarAlg(std::istream &in);
  void init();
  std::string builtinHelpString();
};


inline bool sCast(SRef& rin, GC_Hand<StrObj>& rout) { return !(rout = rin->str()).is_null(); } 
inline bool sCast(SRef& rin, GC_Hand<BooObj>& rout) { return !(rout = rin->boo()).is_null(); } 

extern string builtinFunctionsDoc;
 
}//end namespace tarski
#endif
