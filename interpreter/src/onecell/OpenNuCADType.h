#ifndef _OPEN_NU_CAD_TYPE_
#define _OPEN_NU_CAD_TYPE_

#include "../shell/schemish.h"
#include "opennucad.h"
#include "OCBuilderType.h"
#include "varorder.h"
#include "../formula/writeForQE.h"
#include <string>
#include <sstream>

namespace tarski {
class OpenNuCADObj; typedef GC_Hand<OpenNuCADObj> OpenNuCADRef;

class OpenNuCADObj : public TypeExtensionObj
{
 private:
  ONuCADRef nucad;
  void plotLeaves(const std::string & wininfo, const std::string startLabel, std::ostream& out);
  void exprop(int k);

 public:
  OpenNuCADObj(ONuCADRef nucad) { this->nucad = nucad; }
  std::string name() { return "OpenNuCAD"; }
  std::string shortDescrip() { return "an Open Non-uniform CAD (NuCAD)."; }
  std::string display() 
  { 
    return nucad->toString();
  }

  static std::ostream& fooxx(const float &f, std::ostream &out)
  {
    if (int(f) == f) { return out << int(f) << ".0"; }
    else { return out << f; }
  }


  NodeRef findSubtreeSizes(ONuCADRef nucad);

  SRef trial(std::vector<SRef>& args)
  {
    std::string label;

    if (args.size() > 0 && !args[0]->str().is_null())
    {
      StrRef s = args[0]->str();
      if (s.is_null()) { throw TarskiException("trial only accepts a string (label) as argument!"); }
      label = s->getVal();
    }
    if (args.size() > 0 && !args[0]->num().is_null())
    {
      int seed = args[0]->num()->numerator();
      srand(seed);
    }
    
    /*** Choose node ***/
    NodeRef node;
    if (label == "") // randomly choose node
    {
      node = findSubtreeSizes(this->nucad);// test
      /*
      std::vector<pair<int,NodeRef>> candidates;
      int leafThreshold = 2; //this->nucad->getRoot()->numNotknownSignInvariantConstraintPolys;
      int choicesThreshold = 2;
      this->nucad->getCandidateNodes(this->nucad->getRoot(),candidates,leafThreshold,choicesThreshold);
      if (candidates.size() == 0) { return new StrObj(""); }
      int i = rand() % candidates.size();
      node = candidates[i].second;
      */
    }
    else // node comes as agument
      node = this->nucad->getNode(label);

    std::vector<std::vector<float> > X;
    std::vector<std::vector<float> > y;
    nucad->trial(node,X,y);
    std::ostringstream sout;
    if (X.size() > 0)
    {
      int N = X[0].size(), M = X.size();
      for(int r = 0; r < M; ++r)
      {
	fooxx(X[r][0],sout);
	for(int c = 1; c < N; ++c)
	{ sout << ','; fooxx(X[r][c],sout); }
	sout << ":"; fooxx(y[r][0],sout);
	sout << ","; fooxx(y[r][1],sout);  sout << std::endl;
      }
    }
    return new StrObj(sout.str());
  }
  
  SRef numXCells(std::vector<SRef>& args) { nucad->toString(0); return new NumObj(nucad->nx); }
  SRef numTCells(std::vector<SRef>& args) { nucad->toString(0); return new NumObj(nucad->nt); }
  SRef numFCells(std::vector<SRef>& args) { nucad->toString(0); return new NumObj(nucad->nf); }
  SRef printTCells(std::vector<SRef>& args) { return new StrObj(nucad->toString(ONuCADObj::m_tcells)); }
  SRef printFCells(std::vector<SRef>& args) { return new StrObj(nucad->toString(ONuCADObj::m_fcells)); }
  SRef pfSummary(std::vector<SRef>& args) { return new StrObj(nucad->toString(ONuCADObj:: m_pfsumm)); }

  SRef getVarOrder(std::vector<SRef>& args)
  {
    LisRef L = new LisObj();
    VarOrderRef X = nucad->getVarOrder();
    PolyManager* p = X->getPolyManager();
    int N = X->size();
    for(int i = 1; i <= N; i++)
      L->push_back(new SymObj(p->getName(X->get(i))));
    return L;
  }

  SRef getUNSATCore(std::vector<SRef>& args)
  {
    return new TarObj(nucad->getUNSATCore());
  }

 SRef getNumSplitOptions(std::vector<SRef>& args)
 {
   StrRef s = args[0]->str();
   if (args.size() == 0 || (s = args[0]->str()).is_null()) 
   { return new ErrObj("OpenNuCAD msg subtree-level requires argument of type str."); }
   NodeRef n = nucad->getNode(s->getVal());
   return new NumObj(n->numSplitOptions);   
 }
  
  SRef subtreeLevel(std::vector<SRef>& args)
  {
    StrRef s = args[0]->str();
    if (args.size() == 0 || (s = args[0]->str()).is_null()) 
    { return new ErrObj("OpenNuCAD msg subtree-level requires argument of type str."); }
    NodeRef n = nucad->getNode(s->getVal());
    int level = n->getSubtreeLevel(true);
    return new NumObj(level);
  }

  SRef getCell(std::vector<SRef>& args)
  {
    StrRef s = args[0]->str();
    if (args.size() == 0 || (s = args[0]->str()).is_null()) 
    { return new ErrObj("OpenNuCAD msg get-cell requires argument of type str."); }
    BuilderRef b;
    try { b = nucad->getCell(s->val); } catch(TarskiException e) { }
    if (b.is_null()) { return new ErrObj("Error in OpenNuCAD msg get-cell: no cell with label \"" + s->val + "\"."); }
    return new ExtObj(new OCBuilderObj(b));
  }
  SRef plotAll(std::vector<SRef>& args)
  {
    if (args.size() == 0) { return new ErrObj("OpenNuCAD plot-all requires argument of type str."); }
    StrRef s = args[0]->str();
    if (s.is_null()) { return new ErrObj("OpenNuCAD plot-all requires argument of type str."); }
    try { plotLeaves(s->getVal(),"C",std::cout); }
    catch(TarskiException &e) { return new ErrObj(e.what()); }
    return new SObj();
  }

  SRef plotLeaves(std::vector<SRef>& args)
  {
    StrRef plots, label, fname;
    if (!(args.size() > 2 && sCast(args[0],plots) && sCast(args[1],label) && sCast(args[2],fname)))
    { return new ErrObj("OpenNuCAD show-graph requires three std::string arguments."); }
    ofstream fout(fname->getVal().c_str());
    if (!fout) { return new ErrObj("File \"" + fname->getVal() + "\" could not be opened."); }
    try { plotLeaves(plots->getVal(),label->getVal(),fout); }
    catch(TarskiException &e) { return new ErrObj(e.what()); }
    return new SObj();
  }

  SRef showGraph(std::vector<SRef>& args)
  {
    StrRef label, fname;
    if (!(args.size() > 1 && sCast(args[0],label) && sCast(args[1],fname)))
    { return new ErrObj("OpenNuCAD show-graph requires two std::string arguments."); }
    std::string startLabel = label->getVal();
    ofstream fout(fname->getVal().c_str());
    if (!fout) { return new ErrObj("File \"" + fname->getVal() + "\" could not be opened."); }
    nucad->writeDot(startLabel,fout);
    fout.close();
    return new SObj();
  }
  SRef mergeLow(std::vector<SRef>& args)
  {
    StrRef lab = args[0]->str();
    NumRef k;
    if (args.size() < 2 || ((k = args[1]->num()), k.is_null())) 
      return new ErrObj("OpenNuCAD exprop requires argument of type num.");
    if (k->denominator() != 1)
      return new ErrObj("OpenNuCAD exprop requires an integer argument.");
    nucad->mergeLow(nucad->getNode(lab->getVal()),k->numerator(),TRUE);
    return new SObj();
  }
  SRef mergeHigh(std::vector<SRef>& args)
  {
    StrRef lab = args[0]->str();
    NumRef k;
    if (args.size() < 2 || ((k = args[1]->num()), k.is_null())) 
      return new ErrObj("OpenNuCAD exprop requires argument of type num.");
    if (k->denominator() != 1)
      return new ErrObj("OpenNuCAD exprop requires an integer argument.");
    nucad->mergeHigh(nucad->getNode(lab->getVal()),k->numerator(),TRUE);
    return new SObj();
  }
  SRef exprop(std::vector<SRef>& args)
  {
    StrRef lab = args[0]->str();
    NumRef k;
    if (args.size() < 2 || ((k = args[1]->num()), k.is_null())) 
      return new ErrObj("OpenNuCAD exprop requires argument of type num.");
    if (k->denominator() != 1)
      return new ErrObj("OpenNuCAD exprop requires an integer argument.");
    nucad->expropDownTo(nucad->getNode(lab->getVal()),k->numerator(),true);
    return new SObj();
  }

  SRef project(std::vector<SRef>& args, bool strictFlag);

  SRef projectStrict(std::vector<SRef>& args) { return project(args,true); }

  SRef projectLoose(std::vector<SRef>& args) { return project(args,false); }

  SRef tiss(std::vector<SRef>& args)
  {
    nucad->truthInvariantSubtreeStats();
    return new SObj();
  }
  SRef defForm(std::vector<SRef>& args)
  {
    int count = 0;
    std::ostringstream sout;
    ONuCADObj::LeafIterator itr = nucad->iterator();
    while(itr.hasNext())
    {
      NodeRef n = itr.next();
      if (n->getTruthValue() == TRUE)
      {
	sout << (++count > 1 ? " \\/ " : "[ ");
	sout << n->getData()->getCell()->definingFormula();
      }
    }
    sout << (count == 0 ? "[ false ]" : " ]");
    return new StrObj(sout.str());
  }
  SRef refineSubtree(std::vector<SRef>& args)
  {
    StrRef s;
    if (args.size() == 0 || (s = args[0]->str()).is_null()) 
    { return new ErrObj("OpenNuCAD msg refine-subtree requires argument of type str."); }      
    try { nucad->refineSubtreeNuCADConjunction(s->val); }
    catch(TarskiException e) { return new ErrObj("Error in OpenNuCAD msg refine-subtree with label \"" + s->val + "\"."); }
    return new SObj();
  }
  SRef refineCell(std::vector<SRef>& args)
  {
    StrRef s;
    if (args.size() == 0 || (s = args[0]->str()).is_null()) 
    { return new ErrObj("OpenNuCAD msg refine-cell requires argument of type str."); }      
    try { nucad->refineNuCADConjunction(s->val); }
    catch(TarskiException e) { return new ErrObj("Error in OpenNuCAD msg refine-cell with label \"" + s->val + "\"."); }
    return new SObj();
  }
  SRef revertCell(std::vector<SRef>& args)
  {
    StrRef s;
    if (args.size() == 0 || (s = args[0]->str()).is_null()) 
    { return new ErrObj("OpenNuCAD msg revert-cell requires argument of type str."); }      
    try { nucad->revertCell(s->val); }
    catch(TarskiException e) { return new ErrObj("Error in OpenNuCAD msg revert-cell with label \"" + s->val + "\"."); }
    return new SObj();
  }
  SRef negate(std::vector<SRef>& args)
  {
    nucad->negateTruthValues();
    return new SObj();
  }

  SRef locate(std::vector<SRef>& args)
  {
    LisRef La;
    if (args.size() == 0 || (La = args[0]->lis()).is_null()) 
    { return new ErrObj("OpenNuCAD msg locate requires argument of type lis."); }    
      
    Word A = NIL;
    for(int i = La->length() - 1; i >= 0; --i)
    {
      NumRef n = La->get(i)->num();
      if (n.is_null()) { return new ErrObj("OpenNuCAD msg locate requires list of num objects."); }
      A = COMP(n->val,A);
    }

    pair<NodeRef,int> C = nucad->locate(nucad->getRoot(),A);
    if (C.second == 0)
      return new ExtObj(new OCBuilderObj(C.first->getData()));
    else
      return new ErrObj("Input point located on cell boundary!");

    /* std::vector<NodeRef> V = nucad->locate(A); */
    /* LisRef res = new LisObj(); */
    /* for(int i = 0; i < V.size(); i++) */
    /*   res->push_back(new ExtObj(new OCBuilderObj(V[i]->getData()))); */
    /* return res; */
  }

  SRef projectionResolve(std::vector<SRef>& args)
  {
    StrRef s1, s2;
    NumRef k;
    if (args.size() < 3 || (s1 = args[0]->str()).is_null() || (s2 = args[1]->str()).is_null() ||
	(k = args[2]->num()).is_null())
    {  return new ErrObj("OpenNuCAD msg test requires two std::string arguments and a number argument."); }    

    NodeRef n1;
    try { n1 = nucad->getNode(s1->val); }
    catch(TarskiException e) { return new ErrObj("Error in OpenNuCAD msg test: no cell with label \"" + s1->val + "\"."); }

    NodeRef n2;
    try { n2 = nucad->getNode(s2->val); }
    catch(TarskiException e) { return new ErrObj("Error in OpenNuCAD msg test: no cell with label \"" + s2->val + "\"."); }

    nucad->projectionResolve(n1,n2,RNROUND(k->getVal()),nucad->getDim());


    n2->setTruthValue(FALSE);
    n2->XYchild = NULL;
    n2->Lchild.clear();
    n2->Uchild.clear();

    return new SObj();
  }

  SRef unionOp(std::vector<SRef>& args)
  { 
    // NuCAD union
    ExtRef e = args[0]->ext();
    TypeExtensionObj* q = e.is_null() ? NULL : &*e->externObjRef;
    OpenNuCADObj* p = static_cast<OpenNuCADObj*>(q);
    if (p == NULL) { return new ErrObj("Error in OpenNuCAD msg union: argument must be an OpenNuCAD!"); }
    NodeRef n1 = nucad->getRoot();
    NodeRef n2 = p->nucad->getRoot();
    nucad->projectionResolve(n1,n2,nucad->getDim(),nucad->getDim());
    return new SObj();
  }

  SRef test(std::vector<SRef>& args)
  { 
    // test NuCAD union
    ExtRef e = args[0]->ext();
    TypeExtensionObj* q = &*e->externObjRef;
    OpenNuCADObj* p = static_cast<OpenNuCADObj*>(q);
    NodeRef n1 = nucad->getRoot();
    NodeRef n2 = p->nucad->getRoot();
    nucad->projectionResolve(n1,n2,nucad->getDim(),nucad->getDim());
    return new SObj();

    // test something else
    NumRef k;
    if (args.size() < 1 || (k = args[0]->num()).is_null())
    {  return new ErrObj("OpenNuCAD msg test requires a number argument."); }    

    NodeRef res = nucad->getRoot()->lowerMostNodeWithSubtreeLevelExceedingK(k->numerator());
    if (res.is_null()) return new LisObj();

    LisRef L = new LisObj();
    L->push_back(new SymObj("projection-resolve"));
    L->push_back(new StrObj(res->XYchild->getLabel()));
    int m = res->XYchild->getSplitLevel();
    if (res->Lchild.size() > 0 && res->Lchild.back()->getSplitLevel() == m)
      L->push_back(new StrObj(res->Lchild.back()->getLabel()));
    else
      L->push_back(new StrObj(res->Uchild.back()->getLabel()));
    L->push_back(k);
    return L;
  }

  /* SRef evalMsg(SymRef msg, std::vector<SRef>& args)  */
  /* {  */
  /*   if (msg->val == "num-x-cells") { nucad->toString(0); return new NumObj(nucad->nx); } */
  /*   else if (msg->val == "num-t-cells") { nucad->toString(0); return new NumObj(nucad->nt); } */
  /*   else if (msg->val == "num-f-cells") { nucad->toString(0); return new NumObj(nucad->nf); } */
  /*   else if (msg->val == "print-t-cells") { return new StrObj(nucad->toString(ONuCADObj::m_tcells)); } */
  /*   else if (msg->val == "print-f-cells") { return new StrObj(nucad->toString(ONuCADObj::m_fcells)); } */
  /*   else if (msg->val == "pf-summary") { return new StrObj(nucad->toString(ONuCADObj:: m_pfsumm)); } */
  /*   else if (msg->val == "get-cell") */
  /*   { */
  /*     StrRef s = args[0]->str(); */
  /*     if (args.size() == 0 || (s = args[0]->str()).is_null())  */
  /*     { return new ErrObj("OpenNuCAD msg get-cell requires argument of type str."); } */
  /*     BuilderRef b; */
  /*     try { b = nucad->getCell(s->val); } catch(TarskiException e) { } */
  /*     if (b.is_null()) { return new ErrObj("Error in OpenNuCAD msg get-cell: no cell with label \"" + s->val + "\"."); } */
  /*     return new ExtObj(new OCBuilderObj(b)); */
  /*   } */
  /*   else if (msg->val == "plot-all") */
  /*   { */
  /*     if (args.size() == 0) { return new ErrObj("OpenNuCAD plot-all requires argument of type str."); } */
  /*     StrRef s = args[0]->str(); */
  /*     if (s.is_null()) { return new ErrObj("OpenNuCAD plot-all requires argument of type str."); } */
  /*     try { plotLeaves(s->getVal(),"C"); } */
  /*     catch(TarskiException &e) { return new ErrObj(e.what()); } */
  /*     return new SObj(); */
  /*   } */
  /*   else if (msg->val == "plot-leaves") */
  /*   { */
  /*     if (args.size() < 2) { return new ErrObj("OpenNuCAD plot-leaves requires two arguments of type str."); } */
  /*     StrRef s = args[0]->str(); */
  /*     StrRef lab = args[1]->str(); */
  /*     if (s.is_null()) { return new ErrObj("OpenNuCAD plot-leaves requires two arguments of type str."); }       */
  /*     try { plotLeaves(s->getVal(),lab->getVal()); } */
  /*     catch(TarskiException &e) { return new ErrObj(e.what()); } */
  /*     return new SObj(); */
  /*   } */
  /*   else if (msg->val == "show-graph") */
  /*   { */
  /*     if (args.size() == 0) { return new ErrObj("OpenNuCAD show-graph requires argument of type str."); } */
  /*     StrRef s = args[0]->str(); */
  /*     if (s.is_null()) { return new ErrObj("OpenNuCAD show-graph requires argument of type str."); } */
  /*     std::string startLabel = s->getVal(); */
  /*     nucad->writeDot(startLabel); */
  /*     return new SObj(); */
  /*   } */
  /*   else if (msg->val == "merge-low") */
  /*   { */
  /*     StrRef lab = args[0]->str(); */
  /*     NumRef k; */
  /*     if (args.size() < 2 || ((k = args[1]->num()), k.is_null()))  */
  /* 	return new ErrObj("OpenNuCAD exprop requires argument of type num."); */
  /*     if (k->denominator() != 1) */
  /* 	return new ErrObj("OpenNuCAD exprop requires an integer argument."); */
  /*     nucad->mergeLow(nucad->getNode(lab->getVal()),k->numerator(),TRUE); */
  /*     return new SObj(); */
  /*   } */
  /*   else if (msg->val == "merge-high") */
  /*   { */
  /*     StrRef lab = args[0]->str(); */
  /*     NumRef k; */
  /*     if (args.size() < 2 || ((k = args[1]->num()), k.is_null()))  */
  /* 	return new ErrObj("OpenNuCAD exprop requires argument of type num."); */
  /*     if (k->denominator() != 1) */
  /* 	return new ErrObj("OpenNuCAD exprop requires an integer argument."); */
  /*     nucad->mergeHigh(nucad->getNode(lab->getVal()),k->numerator(),TRUE); */
  /*     return new SObj(); */
  /*   } */
  /*   else if (msg->val == "exprop") */
  /*   { */
  /*     StrRef lab = args[0]->str(); */
  /*     NumRef k; */
  /*     if (args.size() < 2 || ((k = args[1]->num()), k.is_null()))  */
  /* 	return new ErrObj("OpenNuCAD exprop requires argument of type num."); */
  /*     if (k->denominator() != 1) */
  /* 	return new ErrObj("OpenNuCAD exprop requires an integer argument."); */
  /*     nucad->expropDownTo(nucad->getNode(lab->getVal()),k->numerator()); */
  /*     return new SObj(); */
  /*   } */
  /*   else if (msg->val == "tiss") */
  /*   { */
  /*     nucad->truthInvariantSubtreeStats(); */
  /*     return new SObj(); */
  /*   } */
  /*   else if (msg->val == "def-form") */
  /*   { */
  /*     int count = 0; */
  /*     std::ostringstream sout; */
  /*     ONuCADObj::LeafIterator itr = nucad->iterator(); */
  /*     while(itr.hasNext()) */
  /*     { */
  /* 	NodeRef n = itr.next(); */
  /* 	if (n->getTruthValue() == TRUE) */
  /* 	{ */
  /* 	  sout << (++count > 1 ? " \\/ " : "[ "); */
  /* 	  sout << n->getData()->getCell()->definingFormula(); */
  /* 	} */
  /*     } */
  /*     sout << (count == 0 ? "[ false ]" : " ]"); */
  /*     return new StrObj(sout.str()); */
  /*   } */
  /*   else if (msg->val == "refine-cell") */
  /*   { */
  /*     StrRef s; */
  /*     if (args.size() == 0 || (s = args[0]->str()).is_null())  */
  /*     { return new ErrObj("OpenNuCAD msg refine-cell requires argument of type str."); }       */
  /*     try { nucad->refineNuCADConjunction(s->val); } */
  /*     catch(TarskiException e) { return new ErrObj("Error in OpenNuCAD msg refine-cell with label \"" + s->val + "\"."); } */
  /*     return new SObj(); */
  /*   } */
  /*   else if (msg->val == "negate") */
  /*   { */
  /*     nucad->negateTruthValues(); */
  /*     return new SObj(); */
  /*   } */
  /*   else if (msg->val == "locate") */
  /*   { */
  /*     LisRef La; */
  /*     if (args.size() == 0 || (La = args[0]->lis()).is_null())  */
  /*     { return new ErrObj("OpenNuCAD msg locate requires argument of type lis."); }     */
      
  /*     Word A = NIL; */
  /*     for(int i = La->length() - 1; i >= 0; --i) */
  /*     { */
  /* 	NumRef n = La->get(i)->num(); */
  /* 	if (n.is_null()) { return new ErrObj("OpenNuCAD msg locate requires list of num objects."); } */
  /* 	A = COMP(n->val,A); */
  /*     } */
  /*   } */
  /*   return new ErrObj("Message " + msg->val + " unsupported by " + name() + " objects");  */
  /* } */

  // BEGIN: BOILERPLATE
  static AMsgManager<OpenNuCADObj> _manager;
  const MsgManager& getMsgManager() { return _manager; } 
  static TypeExtensionObj::LFH<OpenNuCADObj> A[];
  //   END: BOILERPLATE  
};


class OCMakeNuCADConjunctionComm : public EICommand
{
public:
  OCMakeNuCADConjunctionComm(NewEInterpreter* ptr) : EICommand(ptr) { }
  

  SRef execute(TFormRef F) 
  {
    // variable order
    std::vector<VarSet> X = getBrownVariableOrder(F);
    VarOrderRef V = new VarOrderObj(interp->PM);
    for(int i = 0; i < X.size(); ++i)
      V->push_back(X[i]);
    
    // choose the origin for "point alpha"
    Word A = NIL;
    for(int i = 0; i < X.size(); ++i)
      A = COMP(0,A);

    // Build & return Open NuCAD
    TAndRef C = asa<TAndObj>(F);
    if (C.is_null())
    {
      TAtomRef a = asa<TAtomObj>(F);
      TExtAtomRef b = asa<TExtAtomObj>(F);
      if (a.is_null() && b.is_null()) 
	return new ErrObj("OCMakeNuCADConjunction requires a conjunction or atomic formula.");      
      C = new TAndObj(); C->AND(F);
    }
    ONuCADRef nucad = new ONuCADObj(V,C,V->size(),A);
    nucad->mkNuCADConjunction(V,C,V->size(),A);
    return new ExtObj(new OpenNuCADObj(nucad));
  }
  
  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    /*
      Options are pairs: ('chooser <string:name>)
    */
    int optionsCount = 0;
    std::string comp_name, nn_string, dl_string;    
    for(int i = 0; i < args.size(); ++i)
    {
      LisRef P = args[i]->lis();
      SymRef kind;
      StrRef name;
      if (P.is_null() || P->length() != 2 || ((kind = P->get(0)->sym()), kind.is_null())
	  || ((name = P->get(1)->str()), name.is_null()))
      {
	continue;
      }
      optionsCount++;
      if (kind->getVal() == "chooser") { comp_name = name->getVal(); }
      else if (kind->getVal() == "nn-chooser") { nn_string = name->getVal(); }
      else if (kind->getVal() == "dl-chooser") { dl_string = name->getVal(); }
      else throw TarskiException("Unknown option kind!");
    }    

    VarOrderRef V = NULL;
    Word A = NIL;
    TAndRef C = NULL;
    
    if (args.size() - optionsCount != 3)
    {
      TFormRef F = args[0]->tar()->getValue();
      
      // variable order
      std::vector<VarSet> X = getBrownVariableOrder(F);
      V = new VarOrderObj(interp->PM);
      for(int i = 0; i < X.size(); ++i)
	V->push_back(X[i]);
    
      // choose the origin for "point alpha"
      A = NIL;
      for(int i = 0; i < X.size(); ++i)
	A = COMP(0,A);

      // Make sure C is a conjunction!
      C = asa<TAndObj>(F);
      if (C.is_null())
      {
	TAtomRef a = asa<TAtomObj>(F);
	TExtAtomRef b = asa<TExtAtomObj>(F);
	if (a.is_null() && b.is_null()) 
	  return new ErrObj("OCMakeNuCADConjunction requires a conjunction or atomic formula.");      
	C = new TAndObj(); C->AND(F);
      }
    }
    else
    {
      // Variable order
      LisRef Lv = args[0]->lis();
      V = new VarOrderObj(interp->PM);
      for(int i = 0; i < Lv->length(); ++i)
      {
	SymRef s = Lv->get(i)->sym();
	V->push_back(s->val);
      }

      // Rational point alpha
      LisRef La = args[1]->lis();
      A = NIL;
      for(int i = La->length() - 1; i >= 0; --i)
	A = COMP(La->get(i)->num()->val,A);

      // Conjunction F
      TarRef TF = args[2]->tar();
      TFormRef F = TF->val;
      C = asa<TAndObj>(F);
      if (C.is_null())
      {
	TAtomRef a = asa<TAtomObj>(F);
	TExtAtomRef b = asa<TExtAtomObj>(F);
	if (a.is_null() && b.is_null()) 
	  return new ErrObj("OCMakeNuCADConjunction requires a conjunction or atomic formula.");      
	C = new TAndObj(); C->AND(F);
      }
    }

    // Decide which search queue and chooser to use
    SearchQueueRef searchQueue = new SearchQueueObj();
    SplitSetChooserRef chooser = NULL;
    if (comp_name == "BPC") // basic polynomial compare
      chooser = new FeatureChooser(C, new BPCAsComp());
    else if (comp_name == "play")
      chooser = new FeatureChooser(C, new PlayComp());
    else if (comp_name == "daves")
      chooser = new FeatureChooser(C, new DavesComp());
    else if (comp_name == "rand")
      chooser = new FeatureChooser(C, new RandomComp());
    else if (comp_name == "nnet")
      chooser = new FeatureChooser(C, new NNetComp(nn_string));
    else if (comp_name == "dlist")
      chooser = new FeatureChooser(C, new DecisionListComp(dl_string));
    else
      chooser = new SplitSetChooserConjunction(C);

    
    // Build the OpenNuCAD and return it
    ONuCADRef nucad = new ONuCADObj(V,C,V->size(),A,searchQueue,chooser);
    nucad->mkNuCADConjunction(V,C,V->size(),A);
    return new ExtObj(new OpenNuCADObj(nucad));
  }

  std::string testArgs(std::vector<SRef> &args) 
  { 
    std::string s1 = requirepre(args,_tar);
    return s1 == "" ? s1 : requirepre(args,_lis,_lis,_tar);
  } 
  std::string doc() { return "Given a variable order, point and a conjunction of atomic formulas normalized so that each has an irreducible polynomial as left-hand side, returns a list of (l1,c1,...,lk,ck) of labels and cells. Can also be called with just a formula, in which case order and initial point are chosen for you.  After the regular arguments, you can add any number of option pairs (name value), where name is a symbol and value is a string.  Option name 'chooser allows you to specify the strategy used for choosing which polynomial to split on during the construction process. Values for 'chooser are \"BPC\" (the default), which is a simple heuristic, \"rand\" which creates a random heuristic function to be used for the construction (this is never a good idea!). For testing there is also the \"nnet\" chooser option, which requires an additional option, namely 'nn-chooser with value that is the string encoding of the network that will be used to make the choice."; }
  std::string usage() { return "(make-NuCADConjunction var-order rat-point tarski-formula)"; }
  std::string name() { return "make-NuCADConjunction"; }
};


class OCSATNuCADConjunctionComm : public EICommand
{
public:
  OCSATNuCADConjunctionComm(NewEInterpreter* ptr) : EICommand(ptr) { }
  
  SRef execute(SRef input, std::vector<SRef> &args) 
  { 
    VarOrderRef V;
    Word A;
    TAndRef C;

    if (args.size() > 1 && args[0]->type() == _sym && args[0]->sym()->val == "core")
    {
      TFormRef F = args[1]->tar()->val;      
      OpenNuCADSATSolverRef solver = new OpenNuCADSATSolverObj(F);
      V = solver->getVarOrder();
      LisRef res = new LisObj();

      if (solver->isSATFound())
      {
	VarKeyedMap<GCWord> M = solver->getSatisfyingAssignment();
	LisRef Lord = new LisObj();
	LisRef Lpoint = new LisObj();
	for(int i = 1; i <= V->size(); i++)
	{
	  Lord->push_back(new SymObj(getPolyManagerPtr()->getName(V->get(i))));
	  Lpoint->push_back(new NumObj(M[V->get(i)]));
	}
	
	LisRef Lwit = new LisObj(); Lwit->push_back(Lord); Lwit->push_back(Lpoint);
	res->push_back(Lwit);
      }
      else
      {
	TarRef C = new TarObj(solver->getUNSATCore());
	res->push_back(C);
      }
      return res;
    }

    
    if (args.size() != 3)
    {
      TFormRef F = args[0]->tar()->val;

      // variable order
      std::vector<VarSet> X = getBrownVariableOrder(F);
      V = new VarOrderObj(interp->PM);
      for(int i = 0; i < X.size(); ++i)
	V->push_back(X[i]);
      
      // choose the origin for "point alpha"
      A = NIL;
      for(int i = 0; i < X.size(); ++i)
	A = COMP(0,A);
      
      // Build & return Open NuCAD
      C = asa<TAndObj>(F);
      if (C.is_null())
      {
	TAtomRef a = asa<TAtomObj>(F);
	TExtAtomRef b = asa<TExtAtomObj>(F);
	if (a.is_null() && b.is_null()) 
	  return new ErrObj("OCMakeNuCADConjunction requires a conjunction or atomic formula.");      
	C = new TAndObj(); C->AND(F);
      }
    }
    else
    {    
      // Variable order
      LisRef Lv = args[0]->lis();
      V = new VarOrderObj(interp->PM);
      for(int i = 0; i < Lv->length(); ++i)
      {
	SymRef s = Lv->get(i)->sym();
	V->push_back(s->val);
      }
      
      // Rational point alpha
      LisRef La = args[1]->lis();
      A = NIL;
      for(int i = La->length() - 1; i >= 0; --i)
	A = COMP(La->get(i)->num()->val,A);
      
      // Conjunction F
      TarRef TF = args[2]->tar();
      TFormRef F = TF->val;
      C= asa<TAndObj>(F);
      if (C.is_null())
      {
	TAtomRef a = asa<TAtomObj>(F);
	if (a.is_null()) { std::cerr << "OCMakeNuCADConjunction requires a conjunction or atomic formula." << std::endl; exit(1); }
	C = new TAndObj(); C->AND(a);
      }
    }

    // Build the OpenNuCAD and return it
    EarlyTerminateSearchQueueObjRef nodeQueue = new PriorityAEarlyTerminateSearchQueueObj(V,C);
    ONuCADRef nucad = new ONuCADObj(V,C,V->size(),A,nodeQueue);
    nucad->mkNuCADConjunction(V,C,V->size(),A);
    ExtRef D =  new ExtObj(new OpenNuCADObj(nucad));
    LisRef res = new LisObj();
    if (nodeQueue->SATAlphaFound())
    {
      Word alpha = nodeQueue->getSATAlpha();
      LisRef Lpoint = new LisObj();
      for(Word P = alpha; P != NIL; P = RED(P))
      {
	GCWord x = FIRST(P);
	Lpoint->push_back(new NumObj(x));
      }
      
      LisRef Lord = new LisObj();
      for(int i = 1; i <= V->size(); i++)
	Lord->push_back(new SymObj(getPolyManagerPtr()->getName(V->get(i))));

      LisRef Lwit = new LisObj(); Lwit->push_back(Lord); Lwit->push_back(Lpoint);
      res->push_back(Lwit);
    }
    else
      res->push_back(new BooObj(FALSE));
    res->push_back(D);
    return res;
  }

 
  std::string testArgs(std::vector<SRef> &args)
  { 
    if (args.size() == 1) return require(args,_tar);
    else if (args.size() == 2) return require(args,_sym,_tar);
    else return require(args,_lis,_lis,_tar);
  }
  
  std::string doc() { return "Given a variable order, point and a conjunction of atomic formulas normalized so that each has an irreducible polynomial as left-hand side, returns a pair (alpha,D) such that alpha is a satisfying point with respect to the given variable order, or FALSE if no such assignment exists, and D is the Open NuCAD constructed in the search."; }
  std::string usage() { return "(SAT-NuCADConjunction var-order rat-point tarski-formula)"; }
  std::string name() { return "SAT-NuCADConjunction"; }
};

class OCMakeNuCADDNFComm : public EICommand
{
public:
  OCMakeNuCADDNFComm(NewEInterpreter* ptr) : EICommand(ptr) { }
  
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

    // DNF F
    TarRef TF = args[2]->tar();
    TFormRef F = TF->val;

    // Build the OpenNuCAD and return it
    ONuCADRef nucad = new ONuCADObj(V,F,V->size(),A);
    nucad->mkNuCADDNF(V,F,V->size(),A);
    return new ExtObj(new OpenNuCADObj(nucad));
  }

  std::string testArgs(std::vector<SRef> &args) { return require(args,_lis,_lis,_tar); }
  std::string doc() { return "Given a variable order, point and a disjunction of conjunctions of atomic formulas normalized so that each has an irreducible polynomial as left-hand side, returns a list of (l1,c1,...,lk,ck) of labels and cells."; }
  std::string usage() { return "(make-NuCADDNF var-order rat-point tarski-formula)"; }
  std::string name() { return "make-NuCADDNF"; }
};
}//end namespace tarski
#endif
