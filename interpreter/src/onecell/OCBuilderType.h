#ifndef _OC_BUILDER_TYPE_
#define _OC_BUILDER_TYPE_

#include "../shell/schemish.h"
#include "../shell/einterpreter.h"
#include "builder.h"

namespace tarski {
class OCBuilderObj; typedef GC_Hand<OCBuilderObj> OCBuilderRef;

class OCBuilderObj : public TypeExtensionObj
{
 public:
  BuilderRef B;
  OCBuilderObj() { }
  OCBuilderObj(BuilderRef _b) { B = _b; }
  void initRec(VarOrderRef X, Word alpha, PolyManager *ptrPM, std::vector<IntPolyRef> &Lp)
  {
    FactRef f = new FactObj(*ptrPM);
    for(int i = 0; i < Lp.size(); ++i)
      f->addMultiple(Lp[i],1);    
    B = BuilderObj::buildOneRec(new IntPolyObj((short int)1),X,alpha,X->size());
    B->addRec(f);
  }
  void initISSAC(VarOrderRef X, Word alpha, PolyManager *ptrPM, std::vector<IntPolyRef> &Lp)
  {
    FactRef f = new FactObj(*ptrPM);
    for(int i = 0; i < Lp.size(); ++i)
      f->addMultiple(Lp[i],1);    
    B = BuilderObj::buildOne(new IntPolyObj((short int)1),X,alpha,X->size());
    B->add(f);
    B->close();
  }
  std::string name() { return "OCBuilder"; }
  std::string shortDescrip() { return "an Open Cylindrical Cell + extra information about cell construction."; }
  std::string display() 
  { 
    if (B->inErrorState()) 
    { 
      return "Error! Projection factor " + 
	B->getCell()->getPolyManager()->polyToStr(B->getErrorPoly()) + 
	" zero at point alpha.";	   
    }
    std::ostringstream sout;
    PushOutputContext(sout);
    B->getCell()->writeAlpha();
    sout << " " << B->getCell()->definingFormula(); 
    PopOutputContext();
    return sout.str();
  }

  SRef plot2d(std::vector<SRef>& args) { B->getCell()->approx2D(LBRN(-3,1),LBRN(-3,1),2,2,8,8,std::cout); return new SObj(); }
  SRef pfset(std::vector<SRef>& args)
  {
    LisRef L = new LisObj();
    for(std::set<IntPolyRef,PolyManager::PolyLT>::iterator i = B->pfbegin(); i != B->pfend(); ++i)
      L->push_back(new AlgObj(*i,*(B->getPolyManager())));
    return L;
  }

  /* virtual SRef evalMsg(SymRef msg, std::vector<SRef>& args)  */
  /* {  */
  /*   if (msg->getVal() == "plot2d") */
  /*   { */
  /*     B->getCell()->approx2D(LBRN(-3,1),LBRN(-3,1),2,2,8,8,std::cout); */
  /*     return new SObj(); */
  /*   } */
  /*   else if (msg->getVal() == "pfset") */
  /*   { */
  /*     LisRef L = new LisObj(); */
  /*     for(std::set<IntPolyRef,PolyManager::PolyLT>::iterator i = B->pfbegin(); i != B->pfend(); ++i) */
  /* 	L->push_back(new AlgObj(*i,*(B->getPolyManager()))); */
  /*     return L; */
  /*   } */
  /*   else */
  /*     return new ErrObj("Message " + msg->val + " unsupported by " + name() + " objects");  */
  /* } */

  bool eqTest(GC_Hand<TypeExtensionObj> b) 
  { 
    OCBuilderRef Y = dynamic_cast<OCBuilderObj*>(&*b);
    if (Y.is_null())
      throw TarskiException("Equality between given types not supported");     
    OCBuilderRef X = this;
    OpenCellRef x = X->B->getCell();
    OpenCellRef y = Y->B->getCell();
    int minDim = std::min(x->dimension(),y->dimension());
    int maxDim = std::max(x->dimension(),y->dimension());
    bool tst = true;
    for(int i = 1; tst && i <= minDim; i++)
      tst = x->get(i)->equal(y->get(i));
    return (minDim == maxDim) && tst;
  }

  // BEGIN: BOILERPLATE
  static AMsgManager<OCBuilderObj> _manager;
  const MsgManager& getMsgManager() { return _manager; } 
  static TypeExtensionObj::LFH<OCBuilderObj> A[];
  //   END: BOILERPLATE  
};

class NewOCBuilderComm : public EICommand
{
public:
  NewOCBuilderComm(NewEInterpreter* ptr) : EICommand(ptr) { }
  
  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    // Variable order
    LisRef Lv = args[0]->lis();
    VarOrderRef V = new VarOrderObj(interp->PM);
    for(int i = 0; i < Lv->length(); ++i)
    {
      SymRef s = Lv->get(i)->sym();
      V->push_back(s->val);
    }

    // Rational point alpha
    LisRef La = args[1]->lis();
    Word A = NIL;
    for(int i = La->length() - 1; i >= 0; --i)
      A = COMP(La->get(i)->num()->val,A);

    // Polynomial p
    std::vector<IntPolyRef> Lp;
    AlgRef Pa = args[2]->alg();
    if (!Pa.is_null())
      Lp.push_back(Pa->val);
    else
    {
      LisRef L = args[2]->lis();
      for(int i = 0; i < L->length(); ++i)
	Lp.push_back(L->get(i)->alg()->val);
    }

    // Check for extra argument!
    bool useISSAC = false;
    if (args.size() > 3)
    {
      SymRef S = args[3]->sym();
      useISSAC = (!S.is_null() && S->val == "issac");
    }

    OCBuilderRef OCB = new OCBuilderObj();
    if (useISSAC)
      OCB->initISSAC(V,A,interp->PM,Lp);
    else
      OCB->initRec(V,A,interp->PM,Lp);
    return new ExtObj(OCB);
  }
  std::string testArgs(std::vector<SRef> &args) { return ""; }
  std::string doc() { return "Given a variable order, point and polynomial, creates an OpenCell data structure. By default, the recursive divide and conquor algorithm is used. If the optional 'issac argument is provided, the non-D&C algorithm is used. An example call would be like (new-ocbuilder '(x y z) '(-1/2 3/5 0) [ x y^2 z^2 - 3 x z - 1 ]).  To use the non-D&C algorithm, the call would look like (new-ocbuilder '(x y z) '(-1/2 3/5 0) [ x y^2 z^2 - 3 x z - 1 ] 'issac)."; }
  std::string usage() { return "(new-ocbuilder var-order rat-point poly|list-of-poly ['issac])"; }
  std::string name() { return "new-ocbuilder"; }
};

class OCMergeInto : public EICommand
{
public:
  OCMergeInto(NewEInterpreter* ptr) : EICommand(ptr) { }
  
  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    TypeExtensionObj* t1 = &(*(args[0]->ext())->externObjRef);
    OCBuilderRef OCB1 = dynamic_cast<OCBuilderObj*>(t1);
    BuilderRef B1 = OCB1->B;

    TypeExtensionObj* t2 = &(*(args[1]->ext())->externObjRef);
    OCBuilderRef OCB2 = dynamic_cast<OCBuilderObj*>(t2);
    BuilderRef B2 = OCB2->B;

    int n = B1->getCell()->dimension();
    // get B1 sample point (call it alpha) and test that B2 contains alpha
    Word alpha = B1->getCell()->getAlpha();
    bool tst = B2->getCell()->rationalPointInCellQuery(alpha);
    if (!tst)
      return new ErrObj("Sample point of first cell passed to ocmergeinto must be contained within the second");
    BuilderObj::mergeBintoA(B1,B2,n);
    return args[0];
  }
  std::string testArgs(std::vector<SRef> &args) { return ""; }
  std::string doc() { return "Calling (ocmergeinto c1 c2) merges the two cells c1 and c2, i.e. produces a refined cell in which projection factors of both c1 and c2 are sign-invariant. It is required that the sample point of c1 is contained within c1. Note that c1 is modified in the process and becomes the refined cell."; }
  std::string usage() { return "(ocmergeinto cellbuilder1 cellbuilder2)"; }
  std::string name() { return "ocmergeinto"; }
};

class OCProjFacSummary : public EICommand
{
public:
  OCProjFacSummary(NewEInterpreter* ptr) : EICommand(ptr) { }
  
  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    TypeExtensionObj* t1 = &(*(args[0]->ext())->externObjRef);
    OCBuilderRef OCB1 = dynamic_cast<OCBuilderObj*>(t1);
    BuilderRef B1 = OCB1->B;

    return new StrObj(B1->projFacSummary());
  }
  std::string testArgs(std::vector<SRef> &args) { return ""; }
  std::string doc() { return "Returns summary information about the projection factors for the cell."; }
  std::string usage() { return "(oc-projfac-summary cellbuilder)"; }
  std::string name() { return "oc-projfac-summary"; }
};


class OCProjFacSet : public EICommand
{
public:
  OCProjFacSet(NewEInterpreter* ptr) : EICommand(ptr) { }
  
  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    TypeExtensionObj* t1 = &(*(args[0]->ext())->externObjRef);
    OCBuilderRef OCB1 = dynamic_cast<OCBuilderObj*>(t1);
    BuilderRef B1 = OCB1->B;

     // return list of projection factors
    std::vector< std::set<IntPolyRef,PolyManager::PolyLT> >  PF = B1->projFacByLevel();
    int N = PF.size() - 1;
    LisRef res = new LisObj();
    for(int lev = 1; lev <= N; ++lev)
    {
      for(std::set<IntPolyRef,PolyManager::PolyLT>::iterator itr = PF[lev].begin(); 
	  itr != PF[lev].end(); ++itr)
	res->push_back(new AlgObj(*itr,*(interp->PM)));
    }
    return res;
  }
  std::string testArgs(std::vector<SRef> &args) { return ""; }
  std::string doc() { return "Returns list of the projection factors for the cell."; }
  std::string usage() { return "(oc-projfac-std::set cellbuilder)"; }
  std::string name() { return "oc-projfac-std::set"; }
};

class OCPointInCell : public EICommand
{
public:
  OCPointInCell(NewEInterpreter* ptr) : EICommand(ptr) { }
  
  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    // CellBuilder B1
    TypeExtensionObj* t1 = &(*(args[0]->ext())->externObjRef);
    OCBuilderRef OCB1 = dynamic_cast<OCBuilderObj*>(t1);
    BuilderRef B1 = OCB1->B;

    // Rational point A
    LisRef La = args[1]->lis();
    Word A = NIL;
    for(int i = La->length() - 1; i >= 0; --i)
      A = COMP(La->get(i)->num()->val,A);

    bool t = B1->getCell()->rationalPointInCellQuery(A);

    return new SymObj(t ? "true" : "false");
  }
  std::string testArgs(std::vector<SRef> &args) { return ""; }
  std::string doc() { return "Given cell D and rational point PT, returns true iff m = level PT and n = level D and a) m >= n and the projection of PT onto n-space is contained within cell D, or b) m <  n and PT is contained within cell D's induced cell of level n. Otherwise returns false."; }
  std::string usage() { return "(oc-point-in-cell cellbuilder point)"; }
  std::string name() { return "oc-point-in-cell"; }
};

class OCMcCallumProjSummary : public EICommand
{
public:
  OCMcCallumProjSummary(NewEInterpreter* ptr) : EICommand(ptr) { }

  //-- PF is a projection factor structure, N is the maximum level
  //-- This function fills PF with the projection closure.
  void openMcCallumProjClosure(std::vector< std::set<IntPolyRef,PolyManager::PolyLT> > &PF,
			       int N, VarOrderRef V)
  {
    // Do the Open McCallum Projection!
    for(int k = N; k > 1; --k)
    {
      //std::cerr << "OpenMcCallum: starting to project level " << k << ", " << PF[k].size() << " poly's to project!" << std::endl;
      for(std::set<IntPolyRef,PolyManager::PolyLT>::iterator itr = PF[k].begin(); itr != PF[k].end(); ++itr)
      {
	IntPolyRef p = *itr;
	FactRef f = new FactObj(*(interp->PM));
	FactRef discFactors = interp->PM->discriminantFactors(p,(*V)[k]);
	f->addFactors(discFactors,1);
	FactRef ldcfFactors = interp->PM->ldcfFactors(p,(*V)[k]);
	f->addFactors(ldcfFactors,1);
	std::set<IntPolyRef,PolyManager::PolyLT>::iterator jtr = itr;
	while(++jtr != PF[k].end())
	{
	  IntPolyRef q = *jtr;
	  FactRef resFactors = interp->PM->resultantFactors(p,q,(*V)[k]);
	  f->addFactors(resFactors,1);
	}
	for(std::map<IntPolyRef,int>::iterator ftr = f->MultiplicityMap.begin(); ftr != f->MultiplicityMap.end(); ++ftr)
	{
	  IntPolyRef pnew = ftr->first;
	  int lev = V->level(pnew);
	  PF[lev].insert(pnew);
	}
      }
    }
    
  }

  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    // Variable order
    LisRef Lv = args[0]->lis();
    VarOrderRef V = new VarOrderObj(interp->PM);
    for(int i = 0; i < Lv->length(); ++i)
    {
      SymRef s = Lv->get(i)->sym();
      V->push_back(s->val);
    }
    int N = V->size(); // maximum level

    // Put input polys into Projection Factor data structure PF
    std::vector< std::set<IntPolyRef,PolyManager::PolyLT> > PF(N+1);
    {
      LisRef L = args[1]->lis();
      FactRef f = new FactObj(*(interp->PM));
      for(int i = 0; i < L->length(); ++i)
	f->addMultiple(L->get(i)->alg()->val,1);
      for(std::map<IntPolyRef,int>::iterator itr = f->MultiplicityMap.begin(); itr != f->MultiplicityMap.end(); ++itr)
      {
	IntPolyRef p = itr->first;
	int k = V->level(p);
	PF[k].insert(p);
      }
    }

    // PROJECT!!
    openMcCallumProjClosure(PF,N,V);
    

    // return summary
    std::ostringstream sout;
    for(int i = 1; i < N; ++i) sout << PF[i].size() << ",";
    sout << PF[N].size();

    // compute sum of print lengths
    int LN = 0; 
    for(int i = 1; i <= N; ++i)
      for(std::set<IntPolyRef,PolyManager::PolyLT>::iterator itr = PF[i].begin(); itr != PF[i].end(); ++itr)
	LN += interp->PM->polyToStr(*itr).length();
    sout << " L=" << LN;
      
    return new StrObj(sout.str());
  }
  std::string testArgs(std::vector<SRef> &args) { return ""; }
  std::string doc() { return "UNDER CONSTRUCTION"; }
  std::string usage() { return "(oc-mccallum-proj-stats var-order list-of-polys)"; }
  std::string name() { return "oc-mccallum-proj-stats"; }
};

class OCMcCallumProjClosure : public EICommand
{
public:
  OCMcCallumProjClosure(NewEInterpreter* ptr) : EICommand(ptr) { }

  //-- PF is a projection factor structure, N is the maximum level
  //-- This function fills PF with the projection closure.
  void openMcCallumProjClosure(std::vector< std::set<IntPolyRef,PolyManager::PolyLT> > &PF,
			       int N, VarOrderRef V)
  {
    // Do the Open McCallum Projection!
    for(int k = N; k > 1; --k)
    {
      //std::cerr << "OpenMcCallum: starting to project level " << k << ", " << PF[k].size() << " poly's to project!" << std::endl;
      for(std::set<IntPolyRef,PolyManager::PolyLT>::iterator itr = PF[k].begin(); itr != PF[k].end(); ++itr)
      {
	IntPolyRef p = *itr;
	FactRef f = new FactObj(*(interp->PM));
	FactRef discFactors = interp->PM->discriminantFactors(p,(*V)[k]);
	f->addFactors(discFactors,1);
	FactRef ldcfFactors = interp->PM->ldcfFactors(p,(*V)[k]);
	f->addFactors(ldcfFactors,1);
	std::set<IntPolyRef,PolyManager::PolyLT>::iterator jtr = itr;
	while(++jtr != PF[k].end())
	{
	  IntPolyRef q = *jtr;
	  FactRef resFactors = interp->PM->resultantFactors(p,q,(*V)[k]);
	  f->addFactors(resFactors,1);
	}
	for(std::map<IntPolyRef,int>::iterator ftr = f->MultiplicityMap.begin(); ftr != f->MultiplicityMap.end(); ++ftr)
	{
	  IntPolyRef pnew = ftr->first;
	  int lev = V->level(pnew);
	  PF[lev].insert(pnew);
	}
      }
    }
    
  }

  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    // Variable order
    LisRef Lv = args[0]->lis();
    VarOrderRef V = new VarOrderObj(interp->PM);
    for(int i = 0; i < Lv->length(); ++i)
    {
      SymRef s = Lv->get(i)->sym();
      V->push_back(s->val);
    }
    int N = V->size(); // maximum level

    // Put input polys into Projection Factor data structure PF
    std::vector< std::set<IntPolyRef,PolyManager::PolyLT> > PF(N+1);
    {
      LisRef L = args[1]->lis();
      FactRef f = new FactObj(*(interp->PM));
      for(int i = 0; i < L->length(); ++i)
	f->addMultiple(L->get(i)->alg()->val,1);
      for(std::map<IntPolyRef,int>::iterator itr = f->MultiplicityMap.begin(); itr != f->MultiplicityMap.end(); ++itr)
      {
	IntPolyRef p = itr->first;
	int k = V->level(p);
	PF[k].insert(p);
      }
    }

    // PROJECT!!
    openMcCallumProjClosure(PF,N,V);
    
    // return list of projection factors
    LisRef res = new LisObj();
    for(int lev = 1; lev <= N; ++lev)
    {
      for(std::set<IntPolyRef,PolyManager::PolyLT>::iterator itr = PF[lev].begin(); itr != PF[lev].end(); ++itr)
	res->push_back(new AlgObj(*itr,*(interp->PM)));
    }
    return res;
  }
  std::string testArgs(std::vector<SRef> &args) { return ""; }
  std::string doc() { return "UNDER CONSTRUCTION"; }
  std::string usage() { return "(oc-mccallum-proj-closure var-order list-of-polys)"; }
  std::string name() { return "oc-mccallum-proj-closure"; }
};



class OCErrorPoly : public EICommand
{
public:
  OCErrorPoly(NewEInterpreter* ptr) : EICommand(ptr) { }
  
  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    // CellBuilder B1
    TypeExtensionObj* t1 = &(*(args[0]->ext())->externObjRef);
    OCBuilderRef OCB1 = dynamic_cast<OCBuilderObj*>(t1);
    BuilderRef B1 = OCB1->B;

    LisRef res = new LisObj();
    if (B1->inErrorState()) { 
      res->push_back(new AlgObj(B1->getErrorPoly(),*(B1->getCell()->getPolyManager())));
    }
    return res;
  }
  std::string testArgs(std::vector<SRef> &args) { return ""; }
  std::string doc() { return "Returns an empty list if the cellbuilder argument is not in an error state.  Otherwise the 'error polynomial', i.e. a projection factor that is zero at the builder's sample point, is returned."; }
  std::string usage() { return "(oc-error-poly cellbuilder)"; }
  std::string name() { return "oc-error-poly"; }
};

class OCChoosePointCompletion : public EICommand
{
public:
  OCChoosePointCompletion(NewEInterpreter* ptr) : EICommand(ptr) { }
  
  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    TypeExtensionObj* t1 = &(*(args[0]->ext())->externObjRef);
    OCBuilderRef OCB1 = dynamic_cast<OCBuilderObj*>(t1);
    BuilderRef B1 = OCB1->B;

    LisRef PT = args[1]->lis();
    Word B = NIL;
    for(int i = PT->length() - 1; i >= 0; --i)
      B = COMP(PT->get(i)->num()->val,B);
    Word A = B1->getCell()->choosePointCompletion(B);
    LisRef PTC = new LisObj();
    for(Word Ap = A; Ap != NIL; Ap = RED(Ap))
      PTC->push_back(new NumObj(GCWord(FIRST(Ap))));

    return PTC;
  }
  std::string testArgs(std::vector<SRef> &args) { return ""; }
  std::string doc() { return "Takes a point of lower-dimimenion in the cell's projection, and chooses the remaining coordinates to produce a point in the cell."; }
  std::string usage() { return "(oc-choose-point-completion ocbuilder point)"; }
  std::string name() { return "oc-choose-point-completion"; }
};

class OCSplit : public EICommand
{
public:
  OCSplit(NewEInterpreter* ptr) : EICommand(ptr) { }
  
  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    TypeExtensionObj* t1 = &(*(args[0]->ext())->externObjRef);
    OCBuilderRef OCB1 = dynamic_cast<OCBuilderObj*>(t1);
    BuilderRef B1 = OCB1->B;

    TypeExtensionObj* t2 = &(*(args[1]->ext())->externObjRef);
    OCBuilderRef OCB2 = dynamic_cast<OCBuilderObj*>(t2);
    BuilderRef B2 = OCB2->B;

    // NOTE: B2 should be a refinement of B1
    int dim = B1->getCell()->dimension();
    std::vector< std::vector<BuilderRef> > V(dim+1);
    for(int k = 1; k <= dim; ++k)
      BuilderObj::splitAtLevelK(B1,B2,dim,k,V[k]);


    LisRef res = new LisObj();
    for(int k = 1; k <= dim; ++k)
    {
      LisRef resk = new LisObj();
      for(int i = 0; i <= 1; ++i)
	if (V[k][i].is_null()) { resk->push_back( new NumObj(0) ); }
	else { OCBuilderRef OCB = new OCBuilderObj(); OCB->B = V[k][i]; resk->push_back(new ExtObj(OCB)); }
      res->push_back(resk);
    }
    return res;
  }
  std::string testArgs(std::vector<SRef> &args) { return ""; }
  std::string doc() { return "Takes a OpenCell A and a second OpenCell B that is a refinement of A, and returns the result of splitting A by B."; }
  std::string usage() { return "(oc-split ocbuilder ocbuilder)"; }
  std::string name() { return "oc-split"; }
};


class OCPoint : public EICommand
{
public:
  OCPoint(NewEInterpreter* ptr) : EICommand(ptr) { }
  
  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    TypeExtensionObj* t1 = &(*(args[0]->ext())->externObjRef);
    OCBuilderRef OCB1 = dynamic_cast<OCBuilderObj*>(t1);
    BuilderRef B1 = OCB1->B;

    Word A = B1->getCell()->getAlpha();

    LisRef PTC = new LisObj();
    for(Word Ap = A; Ap != NIL; Ap = RED(Ap))
      PTC->push_back(new NumObj(GCWord(FIRST(Ap))));

    return PTC;
  }
  std::string testArgs(std::vector<SRef> &args) { return ""; }
  std::string doc() { return "Given a cell B, oc-point returns the cell's sample point."; }
  std::string usage() { return "(oc-point ocbuilder)"; }
  std::string name() { return "oc-point"; }
};

class OCMovePoint : public EICommand
{
public:
  OCMovePoint(NewEInterpreter* ptr) : EICommand(ptr) { }
  
  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    TypeExtensionObj* t1 = &(*(args[0]->ext())->externObjRef);
    OCBuilderRef OCB1 = dynamic_cast<OCBuilderObj*>(t1);
    BuilderRef B1 = OCB1->B;

    LisRef PT = args[1]->lis();
    Word B = NIL;
    for(int i = PT->length() - 1; i >= 0; --i)
      B = COMP(PT->get(i)->num()->val,B);
    Word A = B1->getCell()->moveSamplePoint(B);
    LisRef PTC = new LisObj();
    for(Word Ap = A; Ap != NIL; Ap = RED(Ap))
      PTC->push_back(new NumObj(GCWord(FIRST(Ap))));

    return PTC;
  }
  std::string testArgs(std::vector<SRef> &args) { return ""; }
  std::string doc() { return "Takes a point of lower-dimimenion in the cell's projection, and chooses the remaining coordinates to produce a point in the cell, then changes the cell's sample point to the new value."; }
  std::string usage() { return "(oc-move-point ocbuilder point)"; }
  std::string name() { return "oc-move-point"; }
};



class OCMakeNuCAD1Comm : public EICommand
{
public:
  OCMakeNuCAD1Comm(NewEInterpreter* ptr) : EICommand(ptr) { }
  
  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    // Variable order
    LisRef Lv = args[0]->lis();
    VarOrderRef V = new VarOrderObj(interp->PM);
    for(int i = 0; i < Lv->length(); ++i)
    {
      SymRef s = Lv->get(i)->sym();
      V->push_back(s->val);
    }

    // Rational point alpha
    LisRef La = args[1]->lis();
    Word A = NIL;
    for(int i = La->length() - 1; i >= 0; --i)
      A = COMP(La->get(i)->num()->val,A);

    // Polynomial p
    AlgRef p = args[2]->alg();

    std::vector<BuilderRef> VV;
    std::vector<std::string> L;
    BuilderObj::mkNuCAD1(V,p->val,V->size(),A,VV,L);
    LisRef R = new LisObj();
    for(int i = 0; i < VV.size(); ++i)
    {
      R->push_back(new StrObj(L[i]));
      OCBuilderRef OCB = new OCBuilderObj(); OCB->B = VV[i];
      R->push_back(new ExtObj(OCB));
    }    
    return R;
  }
  std::string testArgs(std::vector<SRef> &args) { return ""; }
  std::string doc() { return "Given a variable order, point and polynomial, returns a list of (l1,c1,...,lk,ck) of labels and cells."; }
  std::string usage() { return "(make-NuCAD1 var-order rat-point poly"; }
  std::string name() { return "make-nuCAD1"; }
};
}//end namespace tarski
#endif
