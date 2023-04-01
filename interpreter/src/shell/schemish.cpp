#include "schemish.h"
#include "../tarski.h"
#include "utils.h"

using namespace std;

namespace tarski {

SRef TypeExtensionObj::evalMsg(SymRef msg, vector<SRef>& args) 
{ 
  if (msg->val == "help")
  {
    cout << "Object type " << name() << " accepts the following messages: " << endl;	
    int N = getTermWidth();
    for(int i = 0; !isMsgNull(i); i++)
    {
      ostringstream sout;
      sout <<  getMsgLabel(i) << " : " <<  getMsgDesc(i) << endl;
      writeToFit(sout.str(),cout,N,6);
    }
    return new SObj();
  }
  int j = this->getMsgIndex(msg->val.c_str());
  if (j == -1)
    return new ErrObj("Message " + msg->val + " unsupported by " + name() + " objects"); 
  else
    return this->getMsgManager().call(*this,j,args);
}


class MapToTFormWithInterpolation: public MapToTForm
{
public:
  Interpreter *pI;

  MapToTFormWithInterpolation(Interpreter *pI) : MapToTForm(*(pI->getPolyManagerPtr())) 
  { 
    this->pI = pI; 
  }

  void operator()(InterpVar* p) 
  {
    SRef retval = pI->rootFrame->lookup(p->var);
    TarRef F = retval->tar();
    if (!F.is_null()) { res = F->val; }
    else if (!retval->err().is_null()) { throw TarskiException(retval->err()->msg); }
    else { throw TarskiException("Interpolation with a variable of non-formula type."); }
  }

};

void Interpreter::markAndSweep() 
{ 
  currMark = (currMark + 1)%10;
  rootFrame->mark(this);
  for(int i = 0, N = GCFcreated.size(); i < N; ) 
    if (GCFcreated[i]->markVal != currMark) { 
      --N; swap(GCFcreated[i],GCFcreated[N]); delete GCFcreated[N]; GCFcreated.pop_back(); 
    }
    else ++i;
  for(int i = 0, N = GCCcreated.size(); i < N; ) 
    if (GCCcreated[i]->markVal != currMark) { 
      --N; swap(GCCcreated[i],GCCcreated[N]); GCCcreated[N].free(); GCCcreated.pop_back(); 
    }
    else ++i;
}

inline int char2sym(char c)
{
  switch(c) {
  case '0': return 0; 
  case '1':  case '2':  case '3':  case '4':  case '5':  case '6':  case '7':  case '8':  case '9': return 1;
  case '-': return 2;
  case '/': return 3;
  default: return 4;
    }
}

  SRef Interpreter::readnum(istream &in)
  {
    // DFA: states 0-5 sym: 0=0, 1=1-9,2=-,3=/,4=other
    const int T[6][5] = {
      { 1, 2, 3,-1,-1 },
      { 1, 2,-1,-1,-1 },
      { 2, 2,-1, 4,-1 },
      {-1, 2,-1,-1,-1 },
      {-1, 5,-1,-1,-1 },
      { 5, 5,-1,-1,-1 } };
    const int F[] = {0,1,2,0,0,1};
    stringstream sio;
    int s = 0;
    signed char c;
    while(true)
    {
      c = in.get();
      int ns = T[s][char2sym(c)];
      if (ns == -1) { in.putback(c); break; }
      s = ns;
      sio << c;
    }
    if (F[s] == 0) return 0;
    sio << ' '; // Hack for saclib purposes
    PushInputContext(sio);
    GCWord R = RNREAD();
    PopInputContext();
    return new NumObj(R);
  }
    
  
  SRef Interpreter::readstring(istream &in)
  {
    ostringstream sout;
    signed char c = in.get(); if (c != '"') { return 0; }
    bool escape = false;
    while((c = in.get()) && (escape || c != '"') && c != EOF)
    {
      if (escape)
      {
	escape = false;
	if (c == 'n') c = '\n';
	sout << c;
      }
      else if (c == '\\') escape = true;
      else sout << c;
    }
    if (c == '"') { return new StrObj(sout.str());}
    else return 0;
  }
  
  SRef Interpreter::readsym(istream &in)
  {
    ostringstream sout;
    signed char c;
    while((c = in.peek()) && !isspace(c) && c != EOF && c != '('
	  && c != ')' && c != ';' && c != ']' && c != '['
	  )
    {
      in.get();
      sout << c;
    }
    if (sout.str() == "true") { return new BooObj(true); }
    else if (sout.str() == "false") { return new BooObj(false); }
    else if (sout.str().length() > 0)
      return new SymObj(sout.str());
    else 
      return 0;
  }
  
  
  SRef Interpreter::readcomment(istream &in) 
  {  
    signed char c = in.get(); if (c != ';') return 0;
    while((c = in.get()) && c != '\n' && c != EOF);
    return 0;
  }
  
  SRef Interpreter::readlist(istream &in)
  {
    LisRef res = new LisObj();
    signed char c = in.get(); if (c != '(') return 0;
    while(1)
    {
      while(in && isspace(c = in.peek())) { in.get(); }
      if (c == ')') { in.get(); break; }
      SRef tmp = next(in);
      if (tmp.is_null()) { return new ErrObj("Could not parse list object."); }
      if (tmp->type() == _err) { return tmp; }
      res->push_back(tmp);
    }
    if (c == ')') return res;
    else return 0;
  }
  
  SRef Interpreter::readTarAlg(istream &in)
  {
    TarskiRef T;
    LexContext LC(in);
    algparse(&LC,T);
    if (T.is_null()) { return new ErrObj("Could not parse Tarski or Algebraic object."); }
    if (asa<AlgebraicObj>(T))
    {
      try { return new AlgObj(parseTreeToPoly(T,*PM),*PM); }
      catch(TarskiException &e) { return new ErrObj(e.what()); }
    }
    else
    {
      try 
      { 
	//--- OLD WAY: return new TarObj(parseTreeToTForm(T,*PM)); 
	/* The code below implements a limited form of "interpolation" in the sense
	 * of Perl string interpolation.  It's limited because it only works at the top 
	 * level of the interpreter.  Something like (lambda (F) [ex x [ $F ]]) does not
	 * work ... well, if F is a global variable currently, something you'll get the
	 * formula using the value that F has globally at the time of reading the lambda 
	 * expression.
	 */
	/* NOTE: uncommenting the line below allows non-constant denominators and automatically clears them. */
	/* T = clearDenominators(T); */
	MapToTFormWithInterpolation MTTF(this);
	T->apply(MTTF);
	return new TarObj(MTTF.res);
      }
      catch(TarskiException &e) { return new ErrObj(e.what()); }
    }
  }

  void Interpreter::init()
  {
    Frame& symTable = *rootFrame;
    symTable["list"] = new FunObj(_f_list);
    symTable["+"] = new FunObj(_f_add);
    symTable["-"] = new FunObj(_f_sbt);
    symTable["*"] = new FunObj(_f_mul);  
    symTable["eval"] = new FunObj(_f_eval);  
    symTable["map"] = new FunObj(_f_map);  
    symTable["bin-reduce"] = new FunObj(_f_binred);  
    symTable["length"] = new FunObj(_f_length);  
    symTable["cputime"] = new FunObj(_f_cputime);  
    symTable["head"] = new FunObj(_f_head);  
    symTable["get"] = new FunObj(_f_get);  
    symTable["quit"] = new FunObj(_f_quit);
    symTable["msg"] = new FunObj(_f_msg);
    symTable["version"] = new FunObj(_f_version);
    symTable["load"] = new FunObj(_f_load);
    symTable["equal?"] = new FunObj(_f_equal);
    symTable["display"] = new FunObj(_f_display);
    symTable["tail"] = new FunObj(_f_tail);
    symTable["typename"] = new FunObj(_f_typename);
    symTable["apply"] = new FunObj(_f_apply);
  }

string builtinTypesDoc = "\
Builtin Types\n\
sym - symbol.  e.g. 'foo\n\
num - number, integer or rational.  e.g. 123\n\
str - string, e.g. \"hello\"\n\
alg - algebraic, e.g. [x + y^2 - 5]\n\
tar - tarski, e.g. [x + y^2 - 5 < 0]\n\
err - error\n\
fun - function (builtin).  Note: def, set! and lambda are *not* fun's.\n\
clo - closure.  These are user-defined functions\n\
ext - extension types. They show up with their own names.\n\
";

string builtinFunctionsDoc = "\
Builtin Functions\n\
quote      - e.g. (quote (+ 3 4)) or '(+ 3 4)\n\
def        - e.g. (def x 128)\n\
set!       - e.g. (set! x 127)\n\
lambda     - e.g. (def f (lambda (x y) (+ (* x y) 1)))\n\
list       - e.g. (list '(+ 3 4) 8)\n\
+          - e.g. (+ 1 2 3)\n\
-          - e.g. (- 8 3)\n\
*          - e.g. (* 4 5)\n\
eval       - e.g. (eval '(+ 2 5))\n\
apply      - e.g. (apply + '(1 2 3))\n\
map        - e.g. (map * '(1 2 3) '(4 5 6))\n\
bin-reduce\n\
length     - length of a list\n\
cputime    - returns cpu time used in milliseconds\n\
head       - returns first element of a list\n\
get        - e.g. (get '(a b c d e) 2)\n\
quit       - (quit) exits program\n\
version    - prints current tarski version number and date\n\
msg        - (msg <obj> <sym> ....), e.g. (msg D 'print-t-cells)\n\
load       - (load <string>), e.g. (load \"../examples/in1.txt\")\n\
equal?     - tests for equality between two objects.\n\
display    - writes display representation of ojects to stdout.\n\
tail       - returns the rest of a list, e.g. (tail '(1 2 3)) return (2 3).\n\
typename   - returns the name of the argument's type, e.g. (typename 3) is num\n\
";
string tarskiVersionDateString() { return string("tarski version ") + tarskiVersion + ", " + tarskiVersionDate; }
string Interpreter::builtinHelpString() { return tarskiVersionDateString() + "\n\n" + builtinTypesDoc + "\n" + builtinFunctionsDoc; }




#ifdef _SCHEME_MAIN_
uint64 GC_Obj::next_tag = 1;
uint64 GC_Obj::num_deleted = 0;
void nln() { cout << endl; } // just to help with using gdb
int main(int argc, char **argv)
{
  // Get the CA Server up and running!
  int ac; char **av;
  string CASString = "MapleSac";
  for(int i = 1; i < argc; ++i)
  {
    if (argv[i] == string("-CASSaclib")) CASString = "Saclib";
    if (argv[i] == string("-CASMaple"))  CASString = "Maple";
    if (argv[i] == string("-CASMapleSac")) CASString = "MapleSac";
    if (argv[i] == string("-CASSingular"))  CASString = "Singular";
    if (argv[i] == string("-CASSingSac")) CASString = "SingSac";
    if (argv[i] == string("-v")) { verbose = true; }
  }
  if (CASString == "MapleSac") 
    SacModInit(argc,argv,ac,av,"MapleSac","Maple","/usr/local/bin");
  else if (CASString == "SingSac") 
    SacModInit(argc,argv,ac,av,"SingSac","Singular","/usr/bin");  
  else if (CASString == "Saclib")
    SacModInit(argc,argv,ac,av,"Saclib","","");
  else if (CASString == "Maple")
  { 
    SacModInit(argc,argv,ac,av,"Maple","Maple","/usr/local/bin");
  }
  else if (CASString == "Singular")
  { 
    SacModInit(argc,argv,ac,av,"Singular","Singular","/usr/bin");
  }

  cout << "Policy is: " << GVCAP->name() << endl;

  
  nln();
  PolyManager PM;
  Interpreter I(&PM);
  I.init();
  //  simpleAlgNumTest2(*(I.PM));
  while(cin)
  {
    cout << "> ";
    SRef x = I.next(cin);
    if (x.is_null()) break;    
    SRef res = I.eval(I.rootFrame,x);
    if (res->type() == _err && res->err()->msg == "INTERPRETER:quit") break;
    cout << res->toStr() << endl;
    I.markAndSweep();
    //cout << "Frames: " << I.GCFcreated.size() << " Closures: " << I.GCCcreated.size() << endl;
  }
  SacModEnd();
  return 0;
}
#endif

SRef Interpreter::next(istream &in)
  {
    signed char c;  while(in && isspace(c = in.peek())) { in.get(); }
    if (c == EOF) return 0;
    if (isdigit(c)) return readnum(in);
    if (c == '-') { in.get(); char d = in.peek(); in.putback(c); if (!isspace(d)) { return readnum(in); } }
    if (c == '(') return readlist(in);
    if (c == '"') return readstring(in);
    if (c == ';') { readcomment(in); return next(in); }
    if (c == '[') { return readTarAlg(in); }
    if (c == '\'') 
    { 
      in.get(); 
      SRef tmp = next(in); 
      LisRef res = new LisObj();
      res->push_back(new SymObj("quote"));
      if (!tmp.is_null() && tmp->type() != _err) { res->push_back(tmp); return res; }
      else return tmp;
    }
    SRef res = readsym(in);
    if (res.is_null())
    { 
      ostringstream sout; signed char c; while((c = in.get()) && c != EOF && c != '\n') sout << c;
      if (sout.str().length() > 0) 
	res = new ErrObj(string("Character '")
			 + (char)sout.str()[0] + "' unexpected!");
    }
    return res;
  }


SRef Interpreter::eval(Frame* env, SRef input)
{
  switch(input->type())
  {
  case _num: return input; break;
  case _boo: return input; break;
  case _str: return input; break;
  case _err: return input; break;
  case _tar: return input; break;
  case _alg: return input; break;
  case _clo: return input; break;
  case _fun: return input; break;    
  case _sym:{
    return env->lookup(input->sym()->val);
  }break;
  case _lis:{
    LisRef L = input->lis();
    SRef head;
    
    if (L->empty())
      return new ErrObj("Empty function invocation ().");
    head = L->get(0);

    // SPECIAL FORMS
    if (head->type() == _sym && head->sym()->val == "quote")
    {
      if (L->length() != 2)
	return new ErrObj(string("Form 'quote' requires one argument - received ")
			  + toString(L->length()) + string("."));
      return L->get(1);
    }
    if (head->type() == _sym && head->sym()->val == "def")
    {
      if (L->length() != 3)
	return new ErrObj(string("Form 'def' requires two arguments - received ")
			  + toString(L->length()) + string("."));
      SRef name = L->get(1);
      if (name->type() != _sym)
	return new ErrObj(string("Form 'def' requires first argument of type 'sym'."));
      SRef value = eval(env,L->get(2));
      if (value->type() == _err)
	return new ErrObj("In evaluating second argument to 'def', received error - " +
			  value->err()->toStr());
      (*env)[name->sym()->val] = value;
      return new SObj();
    }
    if (head->type() == _sym && head->sym()->val == "set!")
    {
      if (L->length() != 3)
	return new ErrObj(string("Form 'set!' requires two arguments - received ")
			  + toString(L->length()) + string("."));
      SRef name = L->get(1);
      if (name->type() != _sym)
	return new ErrObj(string("Form 'set!' requires first argument of type 'sym'."));
      SRef value = eval(env,L->get(2));
      if (value->type() == _err)
	return new ErrObj("In evaluating second argument to 'set!', received error - " +
			  value->err()->toStr());
      env->set(name->sym()->val,value);
      return new SObj();
    }
    if (head->type() == _sym && head->sym()->val == "lambda")
    {
      return new CloObj(this,L,env);
    }
    if (head->type() == _sym && head->sym()->val == "if")
    {
      int nargs = L->length();
      if (nargs != 3 && nargs != 4) { return new ErrObj("if requires two or three argument expressions."); }
      SRef e1 = eval(env,L->get(1));
      BooRef cond = e1->boo();
      if (cond.is_null())
	  return new ErrObj(e1->type() == _err ?
			    e1->err()->msg  :
			    string("if's first argument expression must evaluate to a boolean - ") +
			  "instead received: " + e1->toStr());
      if (cond->val == true) { return eval(env,L->get(2)); }
      else if (nargs == 3)   { return new SObj(); }
      else                   { return   eval(env,L->get(3)); }
    }
    if (head->type() == _sym && head->sym()->val == "typename")
    {
      if (L->length() != 2) { return new ErrObj("typename required exactly one argument."); }
      SRef e1 = eval(env,L->get(1));
      return new StrObj(e1->typeName());
    }

    // REGULAR FORMS
    SRef f = eval(env,head);
    vector<SRef> args; 
    for(int i = 1; i < L->length(); ++i) 
    {
      SRef t = eval(env,L->get(i));
      if (t->type() == _err) { return t; }
      args.push_back(t);
    }
    
    if (f->type() != _fun && f->type() != _clo) 
    { 
      return new ErrObj("Object " + f->toStr() + " is not a function."); 
    }
    if (f->type() == _clo)
    {      
      Frame *nenv = new Frame(this,f->clo()->env); // env that the eval will take place in
      LisRef parm = f->clo()->L->get(1);
      if (args.size() != parm->length()) 
      { 
	return new ErrObj("Closure expected " + toString(parm->length()) + " arguments, got " 
			  + toString(args.size()) + "."); 
      }
      for(int i = 0; i < args.size(); ++i) { (*nenv)[parm->get(i)->sym()->val] = args[i]; }
      SRef res = new SObj();
      for(int i = 2; i < f->clo()->L->length(); ++i)
	res = eval(nenv,f->clo()->L->get(i));
      return res;
    }
    if (f->fun()->builtin < 0) { return exFunEval(input,f->fun()->builtin,args); }
    switch(f->fun()->builtin)
    {
    case _f_apply: {
      if (args.size() != 2) {
	return new ErrObj("Function 'apply' requires two arguments."); 
      }
      LisRef Lp = new LisObj(new LisObj(new SymObj("quote"),args[0]));
      LisRef X = args[1]->lis();
      if (X.is_null()) {
	return new ErrObj("Function 'apply' requires a list as second argument."); 
      }
      for(int i = 0; i < X->length(); i++) {
	Lp->push_back(new LisObj(new SymObj("quote"),X->get(i)));
      }
      return eval(env,Lp);
    }break;
    case _f_eval:{
      if (args.size() != 1)
	return new ErrObj("Function eval expects exactly one argument.");
      else
      {
	SRef res = eval(env,args[0]);
	if (res->type() == _err)
	  return new ErrObj("Error in function eval: " + res->toStr());
	else
	  return res;
      }
    }break;
    case _f_list: {
      LisRef L = new LisObj();
      for(int i = 0; i < args.size(); ++i) { L->push_back(args[i]); }
      return L;
    }break;
    case _f_add:{ 
      NumRef res = new NumObj(0); 
      for(int i = 0; i < args.size(); ++i) 
      { 
	if (args[i]->type() != _num) { return new ErrObj("Function + requires arguments of type 'num'."); }
	res->add(args[i]->num());
      }
      return res;
    }break;
    case _f_sbt:{ 
      NumRef res = new NumObj(0); 
      if (args.size() < 1) { return new ErrObj("Function - requires at least one argument."); }
      if (args[0]->type() != _num) { return new ErrObj("Function - requires arguments of type 'num'."); }
      if (args.size() == 1) { res->sbt(args[0]->num()); return res; }
      res->add(args[0]->num());
      for(int i = 1; i < args.size(); ++i) 
      { 
	if (args[i]->type() != _num) { return new ErrObj("Function - requires arguments of type 'num'."); }
	res->sbt(args[i]->num());
      }
      return res;
    }break;
    case _f_mul:{ 
      NumRef res = new NumObj(1);
      for(int i = 0; i < args.size(); ++i) 
      { 
	if (args[i]->type() != _num) { return new ErrObj("Function * requires arguments of type 'num'."); }
	res->mul(args[i]->num());
      }
      return res;
    }break;
    case _f_map:{ 
      if (args[0]->type() != _fun && args[0]->type() != _clo) 
      { return new ErrObj("First argument to 'map' must be a function/closure."); }
      for(int i = 1; i < args.size(); ++i)
	if (args[i]->type() != _lis) { return new ErrObj("Subsequent argument to 'map' must be a list."); }
      LisRef res = new LisObj();
      if (args.size() > 1)
	for(int i = 0; i < args[1]->lis()->length(); ++i)
	{
	  LisRef e = new LisObj(); e->push_back(args[0]); 
	  for(int a = 1; a < args.size(); ++a)
	  {
	    //--- there's gotta be a more efficient way to deal with map and quote!
	    LisRef tmp = new LisObj(); tmp->push_back(new SymObj("quote")); tmp->push_back(args[a]->lis()->get(i));
	    e->push_back(tmp);
	  }
	  res->push_back(eval(env,e));
	}
      return res;
    }break;
    case _f_binred:{ 
      LisRef alist = args[1]->lis();
      SRef res = alist->get(0);
      for(int i = 1; i < alist->length(); ++i)
      {
	LisRef e = new LisObj(); e->push_back(args[0]); e->push_back(res); e->push_back(alist->get(i));
	res = eval(env,e);
      }
      return res;
    }break;
    case _f_length:{
      int n = args[0]->length();
      if (n < 0) return new ErrObj("length not defined for this type");
      return new NumObj(n);
    }break;
    case _f_quit:{
      return new ErrObj("INTERPRETER:quit");
    }break;
    case _f_version:{
      return new StrObj(tarskiVersionDateString());
    }break;
    case _f_msg:{
      int n = args.size();
      if (n < 2) { return new ErrObj("msg requires at least two arguments."); }
      ExtRef obj = args[0]->ext(); if (obj.is_null()) { return new ErrObj("first argument to msg must be of an 'extension' type."); }
      SymRef msg = args[1]->sym(); if (msg.is_null()) { return new ErrObj("second argument to msg must be of type sym."); }
      vector<SRef> nargs;
      for(int i = 0; i < n-2; i++) { nargs.push_back(args[i+2]); }
      return obj->externObjRef->evalMsg(msg,nargs);
    }break;
    case _f_head:{
      LisRef alist = args[0]->lis();
      if (alist.is_null()) return new ErrObj("head of non-list undefined.");
      if (alist->length() == 0) return new ErrObj("head of null list undefined.");
      return alist->get(0);
    }break;
    case _f_get:{
      if (args.size() < 2) return new ErrObj("get requires two arguments.");
      SRef res = args[0];
      for(int k = 1; k < args.size(); k++) {
	LisRef alist = res->lis();
	if (alist.is_null()) return new ErrObj("get of non-list undefined.");
	NumRef idx = args[k]->num();
	int i = RNFLOR(idx->val);
	if (i < 0 || alist->length() <= i) 
	{ return new ErrObj("bad index in get."); }
	res = alist->get(i);
      }
      return res;
    }break;
    case _f_cputime: {
      int t1 = CATime();
      //CAStats();
      return new NumObj(t1);
    }break;
    case _f_load: {
      if (args.size() < 1) { return new ErrObj("load requires an argument (the file to load)."); }
      StrRef path = args[0]->str();
      if (path.is_null()) { return new ErrObj("load requires an argument of type string."); }
      ifstream fin(path->val.c_str());
      if (!fin) { return new ErrObj("file '" + path->val + "' not found in load."); }            
      Interpreter &I = *this;
      while(fin)
      {
	SRef x = I.next(fin);
	if (x.is_null()) { break; }
	SRef res = I.eval(I.rootFrame,x);
	if (res->type() == _err && res->err()->msg == "INTERPRETER:quit") break;
	I.markAndSweep();
	if (res->type() == _err)
	{
	  return new ErrObj("Error in load: " + res->err()->msg); 
	}
      }
      return new SObj();
    }break;
    case _f_equal:{
      if (args.size() != 2) { return new ErrObj("Function equal? requires two arguments."); }
      BooRef res = args[0]->equal(args[1]);
      return res;
    }break;
    case _f_display:{
      for(int i = 0; i < args.size(); i++)
	args[i]->display(cout);
      return new SObj();
    }break;
    case _f_tail:{
      LisRef alist = args[0]->lis();
      if (alist.is_null()) return new ErrObj("tail of non-list undefined.");
      if (alist->length() == 0) return new ErrObj("tail of null list undefined.");
      return new LisTailObj(alist); //new ErrObj("Fuction 'tail' not yet implemented!");
    }break;
    default: return new ErrObj("Unkown function - " + f->toStr()); break;
    }break;
    return new ErrObj("Not yet implemented.");
  }break;
  }
  throw TarskiException("Unexpected condition in interpreter eval!");
}

Frame::Frame(Interpreter *pint, Frame *_parent) : parent(_parent), markVal(-1) { pint->GCFcreated.push_back(this); }

CloObj::CloObj(Interpreter *pint, LisRef _l, Frame* _env) : L(_l), env(_env), markVal(-1) 
{ 
  pint->GCCcreated.push_back(this); 
}

void Frame::mark(Interpreter *pI) 
{ 
  // this could be made more efficient!
  if (!pI->isMarked(markVal))
  {
    pI->mark(markVal);
    for(Frame::iterator itr = begin(); itr != end(); ++itr)
      itr->second->mark(pI);
    if (parent != NULL) parent->mark(pI);
  }
}

void CloObj::mark(Interpreter *pI) 
{ 
  // this could be made more efficient!
  if (!pI->isMarked(markVal)) { pI->mark(markVal); env->mark(pI); L->mark(pI); }
}

void LisObj::mark(Interpreter *pI) 
  { 
    if (!pI->isMarked(markVal))
    {
      //cerr << "marking list " << this << endl;
      pI->mark(markVal);
      for(vector<SRef>::iterator itr = elts.begin(); itr != elts.end(); ++itr) 
	(*itr)->mark(pI); 
    }
  }
}//end namespace tarski
