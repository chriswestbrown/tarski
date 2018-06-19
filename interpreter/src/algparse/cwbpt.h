/*****************************************************************
 ** This file defines the objects for Tarski formula parse trees.
 ** We have tokens, algebraic expressions, and formulas.
 ** Each object is garbage collected through reference counts
 ** using the GC_Obj/GC_Ref system.  "TarskiObj" refers collectively
 ** to algebraic expressions and formulas.  Non-member polymorphic
 ** functions can be constructed using the TarskiApply class.
 *****************************************************************/
#ifndef _CWBPT_
#define _CWBPT_
#include "../GC_System/GC_System.h"
#include "../formrepconventions.h"
#include "../tarski.h"
#include <string>
#include <vector>

namespace tarski {
/*****************************************************************
 ** TOKENS
 *****************************************************************/
class TokenObj : public GC_Obj
{
public:
  std::string text;
  int sLine, sColumn, eLine, eColumn;

  TokenObj() { }
  TokenObj(const std::string& s, int sl, int sc, int el, int ec) 
    : text(s), sLine(sl), sColumn(sc), eLine(el), eColumn(ec) { }
  void print() { write(std::cout); }
  virtual void write(std::ostream &out) { out << text; }
  virtual ~TokenObj() { }
};
typedef GC_Hand<TokenObj> TokenRef;

/*****************************************************************
 ** Tarski Objects
 *****************************************************************/
class TarskiApply;

class TarskiObj : public GC_Obj
{
public:
  virtual ~TarskiObj() { }
  virtual void print() { write(std::cout); }
  virtual void write(std::ostream &out) = 0;
  virtual void apply(TarskiApply &F) = 0;
};
typedef GC_Hand<TarskiObj> TarskiRef;

/*****************************************************************
 ** Algebraic Objects
 *****************************************************************/
class AlgebraicObj : public TarskiObj
{
public:
  virtual ~AlgebraicObj() { }
};
typedef GC_Hand<AlgebraicObj> AlgebraicRef;

class Var : public AlgebraicObj
{
public:
  std::string value;
  Var(TokenRef T) : value(T->text) { }
  void write(std::ostream &out) { out << value; }
  inline void apply(TarskiApply &F) ;
};

class Num : public AlgebraicObj
{
public:
  std::string value;
  Num(TokenRef T) : value(T->text) { }
  void write(std::ostream &out) { out << value; }
  inline void apply(TarskiApply &F) ;
};

class UnaryOp : public AlgebraicObj
{
public:
  char op;
  AlgebraicRef arg;
  UnaryOp(char t, AlgebraicRef a) { op = t; arg = a; }
  void write(std::ostream &out) { out << "-("; arg->write(out); out << ")"; }
  inline void apply(TarskiApply &F) ;
};

class BinaryOp : public AlgebraicObj
{
public:
  char op;
  AlgebraicRef arg1, arg2;
  BinaryOp(char t, AlgebraicRef a1, AlgebraicRef a2) {
    op = t; arg1 = a1; arg2 = a2;
  }
  void write(std::ostream &out) { out << "("; arg1->write(out); out << ")" << op << "("; arg2->write(out); out << ")"; }
  inline void apply(TarskiApply &F) ;
};

/*****************************************************************
 ** Functions called by grammar actions for Algebraics
 *****************************************************************/
inline AlgebraicRef* newVar(TokenRef *pTR) { return new AlgebraicRef(new Var(*pTR)); }
inline AlgebraicRef* newNum(TokenRef *pTR) { return new AlgebraicRef(new Num(*pTR)); }
inline AlgebraicRef* newExp(AlgebraicRef *p1, AlgebraicRef *p2) { return new AlgebraicRef(new BinaryOp('^',*p1,*p2)); }
inline AlgebraicRef* newDiv(AlgebraicRef *p1, AlgebraicRef *p2) { return new AlgebraicRef(new BinaryOp('/',*p1,*p2)); }
inline AlgebraicRef* newMlt(AlgebraicRef *p1, AlgebraicRef *p2) { return new AlgebraicRef(new BinaryOp('*',*p1,*p2)); }
inline AlgebraicRef* newSbt(AlgebraicRef *p1, AlgebraicRef *p2) { return new AlgebraicRef(new BinaryOp('-',*p1,*p2)); }
inline AlgebraicRef* newAdd(AlgebraicRef *p1, AlgebraicRef *p2) { return new AlgebraicRef(new BinaryOp('+',*p1,*p2)); }
inline AlgebraicRef* newOpo(AlgebraicRef *p) { return new AlgebraicRef(new UnaryOp('-',*p)); }
inline AlgebraicRef* copy(AlgebraicRef *p) { return new AlgebraicRef(*p); }

/*****************************************************************
 ** Formula Objects
 *****************************************************************/
class FormulaObj : public TarskiObj
{
public:
  virtual ~FormulaObj() { }
};
typedef GC_Hand<FormulaObj> FormulaRef;


/*****************************************************************
 ** Casting
 *****************************************************************/
template<class T>
T* asa(const FormulaRef& p) { return dynamic_cast<T*>(&(*p)); }
template<class T>
T* asa(const AlgebraicRef& p) { return dynamic_cast<T*>(&(*p)); }
template<class T>
T* asa(const TarskiRef& p) { return dynamic_cast<T*>(&(*p)); }



class TarskiConst : public FormulaObj
{
public:
  int value;
  TarskiConst(int x) : value(x) { }
  void write(std::ostream& out) { out << (value == TRUE ? "true" : "false"); }
  inline void apply(TarskiApply &F) ;
};

class InterpVar : public FormulaObj // This is for interpolation of variables from the interpreter
{
public:
  std::string var;
  InterpVar(TokenRef T) : var(T->text) { }
  void write(std::ostream& out) { out << "$" << var; }
  inline void apply(TarskiApply &F) ;
};

class TarskiAtom : public FormulaObj
{
public:
  AlgebraicRef LHS, RHS;
  int relop;
  TarskiAtom(AlgebraicRef L, std::string sigma, AlgebraicRef R) : LHS(L), RHS(R) 
  { 
    int i = 1;
    while(i < 7 && sigma != relopStrings[i]) ++i;
    if (i == 7) { std::cerr << "Error in TarskiAtom!" << std::endl; exit(1); }
    relop = i;
  }
  void write(std::ostream& out) { LHS->write(out); out << relopStrings[relop]; RHS->write(out); }
  inline void apply(TarskiApply &F) ;
};

class ExtTarskiAtom : public FormulaObj
{
public:
  AlgebraicRef LHS, RHS, rootIndex;
  int relop;
  std::string var; int rootIndexAsInt;
  std::string getVar() { return var; }
  int getRootIndex() { return rootIndexAsInt; }
  ExtTarskiAtom(AlgebraicRef L, std::string sigma, AlgebraicRef I, AlgebraicRef R) : LHS(L), RHS(R), rootIndex(I)
  {     
    if (!asa<Var>(L)) { throw TarskiException("LHS of an Extended Tarski Atom must be a variable!"); }
    var = asa<Var>(L)->value;
    if (!asa<Num>(I)) { throw TarskiException("RootIndex of an Extended Tarski Atom must be a number!"); }
    rootIndexAsInt = atoi(asa<Num>(I)->value.c_str());
    int i = 1;
    while(i < 7 && sigma != relopStrings[i]) ++i;
    if (i == 7) { std::cerr << "Error in TarskiAtom!" << std::endl; exit(1); }
    relop = i;
  }
  void write(std::ostream& out)
  { LHS->write(out); out << relopStrings[relop]; out << "_root_"; 
    rootIndex->write(out); out << " "; RHS->write(out); }
  inline void apply(TarskiApply &F) ;
};


class BinaryLogOp  : public FormulaObj
{
public:
  FormulaRef LHS, RHS;
  int logop;
  BinaryLogOp(FormulaRef L, int optype, FormulaRef R) : LHS(L), RHS(R) { logop = optype; }
  void  write(std::ostream& out)
  { out << "["; LHS->write(out); out <<"]" << logopStrings[logop] << "["; RHS->write(out); out << "]"; }
  inline void apply(TarskiApply &F) ;
};

class NotOp  : public FormulaObj
{
public:
  FormulaRef arg;
  NotOp(FormulaRef A) : arg(A) { }
  void write(std::ostream& out)
  { out << "~["; arg->write(out); out <<"]"; }
  inline void apply(TarskiApply &F) ;
};

class QuantifierBlock : public FormulaObj
{
public:
  int kind; // i.e. UNIVER or EXIST
  std::vector<AlgebraicRef> vars;
  FormulaRef arg;
  QuantifierBlock(const std::string &text, TokenRef x) {    
    kind = text == "ex" ? EXIST : UNIVER; vars.push_back(new Var(x)); 
  }
  void addVar(TokenRef x) { vars.push_back(new Var(x)); }
  void addFormula(FormulaRef f) { arg = f; }
  void write(std::ostream& out) { 
    out << "ex "; vars[0]->write(out);
    for(unsigned int i = 1; i < vars.size(); ++i) { out << ','; vars[i]->write(out); }
    out << "["; 
    if (!arg.is_null()) arg->write(out);
    out << "]";
  }
  inline void apply(TarskiApply &F) ;
};

/*****************************************************************
 ** Functions called by grammar actions for Formulas
 *****************************************************************/
inline FormulaRef* newTrue() { return new FormulaRef(new TarskiConst(TRUE)); }
inline FormulaRef* newFalse() { return new FormulaRef(new TarskiConst(FALSE)); }
inline FormulaRef* newInterpVar(TokenRef *pTR) { return new FormulaRef(new InterpVar(*pTR)); }
inline FormulaRef* newTarskiAtom(AlgebraicRef *p1, TokenRef *pt, AlgebraicRef *p2)
{ return new FormulaRef(new TarskiAtom(*p1,(*pt)->text,*p2)); }
inline FormulaRef* newExtTarskiAtom(AlgebraicRef *p1, TokenRef *pt, AlgebraicRef *pi, AlgebraicRef *p2)
{ return new FormulaRef(new ExtTarskiAtom(*p1,(*pt)->text,*pi,*p2)); }
inline FormulaRef* copy(FormulaRef *p) { return new FormulaRef(*p); }
inline FormulaRef* newAnd(FormulaRef *p1, FormulaRef *p2) { return new FormulaRef(new BinaryLogOp(*p1,ANDOP,*p2)); }
inline FormulaRef* newOr(FormulaRef *p1, FormulaRef *p2) { return new FormulaRef(new BinaryLogOp(*p1,OROP,*p2)); }
inline FormulaRef* newNot(FormulaRef *p) { return new FormulaRef(new NotOp(*p)); }
inline FormulaRef* newQuantifierBlock(TokenRef *pt, TokenRef *px) { 
  return new FormulaRef(new QuantifierBlock((*pt)->text,*px)); }
inline FormulaRef* addVarToQuantifier(FormulaRef* pq, TokenRef *px)
{
  QuantifierBlock *q = dynamic_cast<QuantifierBlock*>(&(**pq));
  q->addVar(*px);
  return new FormulaRef(q);
}
inline FormulaRef* addFormulaToQuantifier(FormulaRef* pq, FormulaRef *pf)
{
  QuantifierBlock *q = dynamic_cast<QuantifierBlock*>(&(**pq));
  q->addFormula(*pf);
  return new FormulaRef(q);
}


/*****************************************************************
 ** Tarski Apply --- By deriving from this class you create an
 ** object that can act as a polymorphic function of a single
 ** TarskiObj* object.
 *****************************************************************/
class TarskiApply
{
public:
  virtual void operator()(Var* p) { }
  virtual void operator()(Num* p) { }
  virtual void operator()(InterpVar* p) { }
  virtual void operator()(UnaryOp* p) { }
  virtual void operator()(BinaryOp* p) { }
  virtual void operator()(TarskiConst* p) { }
  virtual void operator()(TarskiAtom* p) { }
  virtual void operator()(ExtTarskiAtom* p) { }
  virtual void operator()(BinaryLogOp* p) { }
  virtual void operator()(NotOp* p) { }  
  virtual void operator()(QuantifierBlock* p) { } 
};

void Var::apply(TarskiApply &F) { F(this); }
void InterpVar::apply(TarskiApply &F) { F(this); }
void Num::apply(TarskiApply &F) { F(this); }
void UnaryOp::apply(TarskiApply &F) { F(this); }
void BinaryOp::apply(TarskiApply &F) { F(this); }
void TarskiConst::apply(TarskiApply &F) { F(this); }
void TarskiAtom::apply(TarskiApply &F) { F(this); }
void ExtTarskiAtom::apply(TarskiApply &F) { F(this); }
void BinaryLogOp::apply(TarskiApply &F) { F(this); }
void NotOp::apply(TarskiApply &F) { F(this); }
void QuantifierBlock::apply(TarskiApply &F) { F(this); }

}//end namespace tarski

#endif
