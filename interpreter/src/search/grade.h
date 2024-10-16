#ifndef _FORMULA_GRADE_
#define _FORMULA_GRADE_

#include "search.h"

namespace tarski {
  
// MinFormAndGrade: Takes a TFQueue and returns the formula with smallest grade
// represented by that queue, along with the grade itself.  The function is
// parameterized by a FormulaGrader object.
  
class FormulaGraderObj;
typedef GC_Hand<FormulaGraderObj> FormulaGraderRef;
pair<TFormRef,double> minFormAndGrade(TFQueueRef Q, VarSet QVars, FormulaGraderRef FG);

class FormulaGraderObj : public GC_Obj
{
public:
  virtual double grade(TFormRef F, VarSet QVars) = 0;
};

// SimpleGradeForQEPCAD - This provides a really simple (and probably quite stupid!)
// grader for formulas that tries to differentiate their potential as input for QEPCAD.
// if quantifier-free, then {# char's in printed form} else
// 1000*(10*20^{# of quantified variables} - {# of equations}) + {# char's in printed form}
class SimpleGradeForQEPCAD : public FormulaGraderObj
{
  double grade(TFormRef F, VarSet QVars);
};

// SmartGradeForQEPCADv1 - This uses some smarts in terms of formula
// factorization to better recognize the true complexity of a solution.
class SmartGradeForQEPCADv1 : public FormulaGraderObj
{
  double grade(TFormRef F, VarSet QVars);
};




class MinFormFinder
{
public:
  void process(TFQueueRef Q, FormulaGraderRef FG);
  double getMinGrade(TFQueueRef Q);
  double getMinGrade(QAndRef A);
  int getMinLength(TFQueueRef Q);
  TFormRef getMinFormula(TFQueueRef Q);

private:
  std::map<uint64, double>  minGrade;
  std::map<uint64, int>     minLength;
  std::map<uint64,TFormRef> minFormula;
 public:
  std::map<uint64,QNodeRef>   fromTtoQ; // This just maps the formulas back to nodes (temporarily!)

  void searchForMin(TFQueueRef Q, FormulaGraderRef FG);
  void searchForMin(QAndRef A, FormulaGraderRef FG);
  void searchForMin(QOrRef A, FormulaGraderRef FG);
};

class QAndGradeComp
{
  FormulaGraderRef FG;
 public:
  QAndGradeComp(FormulaGraderRef G) : FG(G) { }
  bool operator()(QAndRef A, QAndRef B) const { return FG->grade(A->F,A->QVars) < FG->grade(B->F,B->QVars); }
   
};
}//end namespace tarski
#endif
