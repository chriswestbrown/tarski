#include "prophandler.h"
#include "nucadcell.h"
#include "collectivetermbase.h"

using namespace tarski;

void TempPropHandler::handle(int currentLevel, PropertyRef pt, TermId nextid, NuCADCellRef c,
			     int& nextSectionIdentifier)
{
  //** HANDLE: check properties**//
  if (prop::isCheck(pt->getProperty()))
  {
    if (c->getSamplePointManager()->check(c->getSamplePointId(),pt) == TRUE)
      c->record(nextid,"check");
    else
      throw TarskiException("Error!  property check failed!");
    return;
  }
  
  //** HANDLE: non-check properties**//
  switch(pt->getProperty())
  {

  case (prop::si): { //-- si
    if (pt->getChild(0)->kind() == Term::factor) // FACTOR
    {
      int level = pt->level(c->getVarOrder());
      if (level < 1)
	c->record(nextid,"level < 1 ==> si");
      else
      {
	std::vector<TermRef> froms;
	FactRef f = pt->getChild(0)->getFactor();
	for(auto itr = f->factorBegin(); itr != f->factorEnd(); ++itr)
	  froms.push_back(new Property(prop::si, new PolyTerm(itr->first)));
	c->record(nextid,"si of factors implies si of product",froms);
      }
    }
    else // POLY
    {      
      PolyTermRef polyterm = pt->getChild(0);
      PropertyRef nullAtAlpha = new Property(prop::null_alpha, polyterm);
      if (c->getSamplePointManager()->check(c->getSamplePointId(),nullAtAlpha) == TRUE)
	c->record(nextid,"ni and nullified implies si",
		  std::vector<TermRef>{nullAtAlpha, new Property(prop::ni,polyterm)});
      else      
	c->record(nextid,"oi implies si", std::vector<TermRef>{new Property(prop::oi,polyterm)});
    }
  }break;

  case (prop::oi): { //-- oi
    if (pt->getChild(0)->kind() == Term::factor) // FACTOR
    {
      int level = pt->level(c->getVarOrder());
      if (level < 1)
	c->record(nextid,"level < 1 ==> oi");
      else
      {
	std::vector<TermRef> froms;
	FactRef f = pt->getChild(0)->getFactor();
	for(auto itr = f->factorBegin(); itr != f->factorEnd(); ++itr)
	  froms.push_back(new Property(prop::oi, new PolyTerm(itr->first)));
	c->record(nextid,"oi of factors implies oi of product",froms);
      }
    }
    else // POLY
    {
      PolyTermRef polyterm = pt->getChild(0);
      IntPolyRef p = polyterm->getPoly();
      int k = c->getVarOrder()->level(p);
      std::pair<SectionRef,SectionRef> res =
	c->getSamplePointManager()->boundingSections(c->getSamplePointId(),p);
      if (!res.second.is_null()) // p(alpha) /= 0
      {
	SectionTermRef secLower = new SectionTerm(res.first), secUpper = new SectionTerm(res.second);
	SectionTermRef secVar1 = new SectionVariable(nextSectionIdentifier++,k);
	SectionTermRef secVar2 = new SectionVariable(nextSectionIdentifier++,k);
	c->record(nextid,"sector free of sections of p", vector<TermRef>{
	    new Property(prop::sector, secVar1, secVar2),
	      new Property(prop::leq, secLower, secVar1),
	      new Property(prop::leq, secVar2, secUpper),
	      new Property(prop::ana_delin, polyterm)});
      }
      else
      {
	bool TESTING = true;
	if (TESTING)
	{
	  SectionTermRef secLower = new SectionTerm(res.first);
	  SectionTermRef secVar1 = new SectionVariable(nextSectionIdentifier++,k);
	  SectionTermRef secVar2 = new SectionVariable(nextSectionIdentifier++,k);
	  c->record(nextid,"sector free of sections of p", vector<TermRef>{
	      new Property(prop::sector, secVar1, secVar2),
		new Property(prop::leq, secLower, secVar1),
		new Property(prop::leq, secVar2, secLower),
		new Property(prop::ana_delin, polyterm)});
	}
	else
	  c->record(nextid,"assumption (unhandled case when p(alpha) = 0)");
      }
    }
  }break;


  case (prop::ni): {
    PolyTermRef polyterm = pt->getChild(0);
    IntPolyRef p = polyterm->getPoly();
    int k = c->getVarOrder()->level(p);
    if (k < 3)
      c->record(nextid,"level < 3 ==> ni");
    else
    {
      PolyManager* pPM = c->getPolyManager();
      VarSet x = c->getVarOrder()->getMainVariable(p);
      std::vector<IntPolyRef> C;
      //      pPM->nonZeroCoefficients(p,x,C);
      int d = p->degree(x);
      for(int n = d; d >= 0; --d)
      {
	IntPolyRef g = pPM->coef(p,x,d);
	if (!g->isZero())
	  C.push_back(g);
      }
      bool hasConstantCoefficient = false;
      for(int i = 0; i < C.size() && !hasConstantCoefficient; i++)
      {
	std::cerr << " DEBUG: ";
	C[i]->write(*pPM);
	std::cerr << " isContant() returns " << C[i]->isConstant() << std::endl;
	hasConstantCoefficient = C[i]->isConstant();
      }
      if (hasConstantCoefficient) { c->record(nextid,"has constant coefficient");  return; }
      FactRef ldcf = pPM->ldcfFactors(p,x);
      if (c->getSamplePointManager()->factorSignAt(c->getSamplePointId(),ldcf) != 0) {
	TermRef sildcf = new Property( prop::si, new FactorTerm(ldcf) );
	c->record(nextid,"ldcf is non-zero", std::vector<TermRef>{ sildcf });
	return;
      }
      c->record(nextid,"assumption (unhandled)");
    }
  }break;
    

  case (prop::ana_delin): {
    PolyManager* pPM = c->getPolyManager();
    PolyTermRef polyterm = pt->getChild(0);
    IntPolyRef p = polyterm->getPoly();
    int k = c->getVarOrder()->level(p);
    if (k < 2) { c->record(nextid,"level < 2 ==> ana-delin"); }
    else {
      TermRef T0 = new Property(prop::nnull_alpha,polyterm);
      int tid0 = c->getTermContext()->add(T0);	
      int res0;
      if (c->getCollectiveTermBase()->isIn(tid0)) res0 = TRUE;
      else { res0 = c->getSamplePointManager()->check(c->getSamplePointId(),T0);
	if (res0 == TRUE) c->record(tid0,"check"); }
      if (res0 == TRUE)
      {	
	int k = c->getVarOrder()->level(p);
	VarSet x = c->getVarOrder()->get(k);

	// If this cell is/will be bounded at this level, and p is not the defining
	// polynomial of the upper or lower bound sections for the cell, then the
	// leading coefficient isn't needed.  This comes from Brown JSC paper
	SectionRef cellLower = c->getLowerBoundAtLevel(currentLevel);
	SectionRef cellUpper = c->getUpperBoundAtLevel(currentLevel);
	FactRef ldcf = pPM->ldcfFactors(p,x);
	int s_ldcf = c->getSamplePointManager()->factorSignAt(c->getSamplePointId(),ldcf);
	if (cellLower->getKind() != Section::negInfty && cellUpper->getKind() != Section::posInfty
 	    && !cellLower->getPoly().identical(p) //-- Note: these are canonical references!
 	    && !cellUpper->getPoly().identical(p) //-- Note: these are canonical references!
	    )
	{
	  if (s_ldcf != 0)	  
	    c->record(nextid,"basic McCallum-Brown (bounded)",std::vector<TermRef>{
		new Property(prop::oi, new FactorTerm(pPM->discriminantFactors(p,x))),
		  new Property(prop::ni, polyterm) });
	  else
	  {
	    //----- If the ldcf is zero at alpha, this might give bad results, E.g.
	    //----- (test-comm 'oi [ (x^2 + y^2 - 1)(4 x y - 1) ] '(x y) '(0 1/2))
	    c->record(nextid,"basic McCallum-Brown with zero ldcf (bounded)",std::vector<TermRef>{
		new Property(prop::oi,
			     new FactorTerm(c->getPolyManager()->resultantFactors(cellLower->getPoly(),p,x))),
		new Property(prop::oi,
			     new FactorTerm(c->getPolyManager()->resultantFactors(cellUpper->getPoly(),p,x))),
		new Property(prop::oi, new FactorTerm(pPM->discriminantFactors(p,x))),
		  new Property(prop::ni, polyterm) });	    
	  }
	}
	else 
	  c->record(nextid,"basic McCallum-Brown",std::vector<TermRef>{
	      new Property(prop::si, new FactorTerm(ldcf)),
		new Property(prop::oi, new FactorTerm(pPM->discriminantFactors(p,x))),
		new Property(prop::ni, polyterm) });
      }
      else
      {
	c->record(nextid,"assumption (unhandled)");
      }
    }
  } break;


    
  case (prop::sector): {
    SectionTermRef t0 = pt->getChild(0); SectionRef s0 = t0->getSection();
    SectionTermRef t1 = pt->getChild(1); SectionRef s1 = t1->getSection();
    SectionTermRef secVar1 = new SectionVariable(nextSectionIdentifier++,currentLevel);
    SectionTermRef secVar2 = new SectionVariable(nextSectionIdentifier++,currentLevel);
    c->record(nextid,"sector surrounds cell", vector<TermRef>{
	new Property(prop::sector, secVar1, secVar2),
	  new Property(prop::leq, t0, secVar1),
	  new Property(prop::leq, secVar2, t1) });    
  } break;
    
  case (prop::leq): {
    SectionTermRef t0 = pt->getChild(0); SectionRef s0 = t0->getSection();
    SectionTermRef t1 = pt->getChild(1); SectionRef s1 = t1->getSection();
    if (s0->compare(s1) == 0)
      c->record(nextid,"leq is reflexive");
    else if (s0->getKind() == Section::negInfty && s1->getKind() == Section::posInfty)
      c->record(nextid,"negInfty leq posInfty");
    else if (s0->getKind() == Section::negInfty)
      c->record(nextid,"negInfty leq anything");
    else if (s1->getKind() == Section::posInfty)
      c->record(nextid,"anything leq posInfty");
    else if (s0->getKind() == Section::indexedRoot && s1->getKind() == Section::indexedRoot)
    {
      IntPolyRef p0 = s0->getPoly();
      IntPolyRef p1 = s1->getPoly();
      if (pt->level(c->getVarOrder()) == 1)
	c->record(nextid,"at level 1 leq_alpha ==> leq",vector<TermRef>{ new Property(prop::leq_alpha, t0, t1) });
      else if (p0.identical(p1)) // NOTE: these are canonical polys!
	c->record(nextid,"ana-delin for single poly",vector<TermRef>{ new Property(prop::leq_alpha, t0, t1),
	      new Property(prop::ana_delin, new PolyTerm(p0)) });
      else
      {
	VarSet x = c->getVarOrder()->get(currentLevel);
	c->record(nextid,"ana-delin for two polys", vector<TermRef>{
	    new Property(prop::leq_alpha, t0, t1),
	      new Property(prop::ana_delin, new PolyTerm(p0)),
	      new Property(prop::ana_delin, new PolyTerm(p1)),
	      new Property(prop::oi, new FactorTerm(c->getPolyManager()->resultantFactors(p0,p1,x))) });
      }
    }
    else
      throw TarskiException("Error in PropHandler! should not be possible to get here!");
  } break;
    
    
    
  default:
    std::cout << "don't know how to handle this property! assuming true" << std::endl;
    c->record(nextid,"assumption (unhandled)");
    break;
  }
}


//-----------------------------------------------------------------------//


void TempPropHandlerMOD::handle(int currentLevel, PropertyRef pt, TermId nextid, NuCADCellRef c,
			     int& nextSectionIdentifier)
{

  

  bool isSection = c->isSectionAtLevel(currentLevel);
  SectionRef defSection = c->getLowerBoundAtLevel(currentLevel);
  IntPolyRef defPoly = defSection->getPoly();

  //** HANDLE: check properties**//
  if (prop::isCheck(pt->getProperty()))
  {
    if (c->getSamplePointManager()->check(c->getSamplePointId(),pt) == TRUE)
      c->record(nextid,"check");
    else
      throw TarskiException("Error!  property check failed!");
    return;
  }
  
  //** HANDLE: non-check properties**//
  switch(pt->getProperty())
  {

  case (prop::si): { //-- si
    if (pt->getChild(0)->kind() == Term::factor) // FACTOR
    {
      int level = pt->level(c->getVarOrder());
      if (level < 1)
	c->record(nextid,"level < 1 ==> si");
      else
      {
	std::vector<TermRef> froms;
	FactRef f = pt->getChild(0)->getFactor();
	for(auto itr = f->factorBegin(); itr != f->factorEnd(); ++itr)
	  froms.push_back(new Property(prop::si, new PolyTerm(itr->first)));
	c->record(nextid,"si of factors implies si of product",froms);
      }
    }
    else // POLY
    {      
      PolyTermRef polyterm = pt->getChild(0);
      PropertyRef nullAtAlpha = new Property(prop::null_alpha, polyterm);
      if (c->getSamplePointManager()->check(c->getSamplePointId(),nullAtAlpha) == TRUE)
	c->record(nextid,"ni and nullified implies si",
		  std::vector<TermRef>{nullAtAlpha, new Property(prop::ni,polyterm)});
      else if (isSection)
      {
	IntPolyRef newp = polyterm->getPoly();
	if (!defPoly.identical(newp))
	{
	  // int s_newp = getSamplePointManager()->polynomialSignAt(getSamplePointId(),newp);
	  VarSet x = c->getVarOrder()->get(currentLevel);
	  c->record(nextid,"McCallum Eqn Constraints", std::vector<TermRef>{
	      new Property(prop::oi,
			   new FactorTerm(c->getPolyManager()->resultantFactors(defPoly,newp,x)))});	  
	}
	else
	{
	  c->record(nextid,"Cell is section of this poly", std::vector<TermRef>{
	      new Property(prop::sector, new SectionTerm(defSection), new SectionTerm(defSection))
	    });
	}
      }
      else
      {
	// This mirrors the oi case
	IntPolyRef p = polyterm->getPoly();
	int k = c->getVarOrder()->level(p);
	std::pair<SectionRef,SectionRef> res =
	  c->getSamplePointManager()->boundingSections(c->getSamplePointId(),p);
	SectionTermRef secLower = new SectionTerm(res.first), secUpper = new SectionTerm(res.second);
	SectionTermRef cbLower = new SectionTerm(c->getLowerBoundAtLevel(currentLevel));
	SectionTermRef cbUpper = new SectionTerm(c->getUpperBoundAtLevel(currentLevel));
	vector<TermRef> reqs{
	    new Property(prop::sector, cbLower, cbUpper),
	      new Property(prop::leq, secLower, cbLower),
	      new Property(prop::leq, cbUpper, secUpper),
	      new Property(prop::ana_delin, polyterm)};
	// if (secLower->kind() == Section::negInfty || secUpper->kind() == Section::posInfty)
	//   reqs.push_back(/* si ldcf */);
	c->record(nextid,"sector free of sections of p", reqs);
      }
    }
  }break;

  case (prop::oi): { //-- oi
    if (pt->getChild(0)->kind() == Term::factor) // FACTOR
    {
      int level = pt->level(c->getVarOrder());
      if (level < 1)
	c->record(nextid,"level < 1 ==> oi");
      else
      {
	std::vector<TermRef> froms;
	FactRef f = pt->getChild(0)->getFactor();
	for(auto itr = f->factorBegin(); itr != f->factorEnd(); ++itr)
	  froms.push_back(new Property(prop::oi, new PolyTerm(itr->first)));
	c->record(nextid,"oi of factors implies oi of product",froms);
      }
    }
    else // POLY
    {
      PolyTermRef polyterm = pt->getChild(0);
      IntPolyRef p = polyterm->getPoly();
      int k = c->getVarOrder()->level(p);
      std::pair<SectionRef,SectionRef> res =
	c->getSamplePointManager()->boundingSections(c->getSamplePointId(),p);

      // added 9/2020
      if (!res.second.is_null()) // p(alpha) /= 0
      {
	c->record(nextid,"poly non-zero at alpha and si implies oi",vector<TermRef>{
	    new Property(prop::nz_alpha, polyterm),
	      new Property(prop::si,polyterm)
	      });
      }
      
      // if (!res.second.is_null() && !isSection) // p(alpha) /= 0
      // {
      // 	SectionTermRef secLower = new SectionTerm(res.first), secUpper = new SectionTerm(res.second);
      // 	SectionTermRef cbLower = new SectionTerm(c->getLowerBoundAtLevel(currentLevel));
      // 	SectionTermRef cbUpper = new SectionTerm(c->getUpperBoundAtLevel(currentLevel));
      // 	c->record(nextid,"sector free of sections of p", vector<TermRef>{
      // 	    new Property(prop::sector, cbLower, cbUpper),
      // 	      new Property(prop::leq, secLower, cbLower),
      // 	      new Property(prop::leq, cbUpper, secUpper),
      // 	      new Property(prop::ana_delin, polyterm)});
      // }
      // else if (!res.second.is_null() && isSection) // p(alpha) /= 0
      // {
      // 	c->record(nextid,"poly non-zero in section and si implies oi in section",vector<TermRef>{
      // 	    new Property(prop::nz_alpha, polyterm),
      // 	      new Property(prop::si,polyterm)
      // 	  });
      // }
      else if (isSection && defPoly.identical(p))
      {
	c->record(nextid,"cell is section of p", vector<TermRef>{
	    new Property(prop::ana_delin, polyterm)});
      }
      else if (isSection) // different poly defines the section
      {
	// TODO
	SectionTermRef secp = new SectionTerm(res.first);
	SectionTermRef cbLower = new SectionTerm(c->getLowerBoundAtLevel(currentLevel));
	c->record(nextid,"cell is section of p", vector<TermRef>{
	    new Property(prop::ana_delin, polyterm),
	      new Property(prop::leq, secp, cbLower),
	      new Property(prop::leq, cbLower, secp)	      
	      });
      }
      else
	c->record(nextid,"assumption (unhandled case when p(alpha) = 0)");
    }
  }break;


  case (prop::ni): {
    PolyTermRef polyterm = pt->getChild(0);
    IntPolyRef p = polyterm->getPoly();
    int k = c->getVarOrder()->level(p);
    if (k < 3)
      c->record(nextid,"level < 3 ==> ni");
    else
    {
      PolyManager* pPM = c->getPolyManager();
      VarSet x = c->getVarOrder()->getMainVariable(p);
      std::vector<IntPolyRef> C;
      //      pPM->nonZeroCoefficients(p,x,C);
      int d = p->degree(x);
      for(int n = d; d >= 0; --d)
      {
	IntPolyRef g = pPM->coef(p,x,d);
	if (!g->isZero())
	  C.push_back(g);
      }
      bool hasConstantCoefficient = false;
      for(int i = 0; i < C.size() && !hasConstantCoefficient; i++)
      {
	std::cerr << " DEBUG: ";
	C[i]->write(*pPM);
	std::cerr << " isContant() returns " << C[i]->isConstant() << std::endl;
	hasConstantCoefficient = C[i]->isConstant();
      }
      if (hasConstantCoefficient) { c->record(nextid,"has constant coefficient");  return; }
      FactRef ldcf = pPM->ldcfFactors(p,x);
      if (c->getSamplePointManager()->factorSignAt(c->getSamplePointId(),ldcf) != 0) {
	TermRef sildcf = new Property( prop::si, new FactorTerm(ldcf) );
	c->record(nextid,"ldcf is non-zero", std::vector<TermRef>{ sildcf });
	return;
      }
      c->record(nextid,"assumption (unhandled)");
    }
  }break;
    

  case (prop::ana_delin): {
    PolyManager* pPM = c->getPolyManager();
    PolyTermRef polyterm = pt->getChild(0);
    IntPolyRef p = polyterm->getPoly();
    int k = c->getVarOrder()->level(p);
    if (k < 2) { c->record(nextid,"level < 2 ==> ana-delin"); }
    else {
      TermRef T0 = new Property(prop::nnull_alpha,polyterm);
      int tid0 = c->getTermContext()->add(T0);	
      int res0;
      if (c->getCollectiveTermBase()->isIn(tid0)) res0 = TRUE;
      else { res0 = c->getSamplePointManager()->check(c->getSamplePointId(),T0);
	if (res0 == TRUE) c->record(tid0,"check"); }
      if (res0 == TRUE)
      {	
	int k = c->getVarOrder()->level(p);
	VarSet x = c->getVarOrder()->get(k);

	// If this cell is/will be bounded at this level, and p is not the defining
	// polynomial of the upper or lower bound sections for the cell, then the
	// leading coefficient isn't needed *provided* the resultant of p w.r.t to both
	// bounds is included in the projection.  This comes from Brown JSC paper
	SectionRef cellLower = c->getLowerBoundAtLevel(currentLevel);
	SectionRef cellUpper = c->getUpperBoundAtLevel(currentLevel);
	FactRef ldcf = pPM->ldcfFactors(p,x);
	int s_ldcf = c->getSamplePointManager()->factorSignAt(c->getSamplePointId(),ldcf);
	if (cellLower->getKind() != Section::negInfty && cellUpper->getKind() != Section::posInfty
 	    && !cellLower->getPoly().identical(p) //-- Note: these are canonical references!
 	    && !cellUpper->getPoly().identical(p) //-- Note: these are canonical references!
	    )
	{
	  if (s_ldcf != 0)	  
	    c->record(nextid,"basic McCallum-Brown (bounded)",std::vector<TermRef>{
		// I have to include this leading coefficient (for now). TODO: think about how to do this better! 
		  new Property(prop::si, new FactorTerm(pPM->ldcfFactors(p,x))),
		  new Property(prop::oi, new FactorTerm(pPM->discriminantFactors(p,x))),
		  new Property(prop::ni, polyterm) });
	  else
	  {
	    //----- If the ldcf is zero at alpha, this might give bad results, E.g.
	    //----- (test-comm 'oi [ (x^2 + y^2 - 1)(4 x y - 1) ] '(x y) '(0 1/2))
	    c->record(nextid,"basic McCallum-Brown with zero ldcf (bounded)",std::vector<TermRef>{
		new Property(prop::oi,
			     new FactorTerm(c->getPolyManager()->resultantFactors(cellLower->getPoly(),p,x))),
		new Property(prop::oi,
			     new FactorTerm(c->getPolyManager()->resultantFactors(cellUpper->getPoly(),p,x))),
		new Property(prop::oi, new FactorTerm(pPM->discriminantFactors(p,x))),
		  new Property(prop::ni, polyterm) });	    
	  }
	}
	else 
	  c->record(nextid,"basic McCallum-Brown",std::vector<TermRef>{
	      new Property(prop::si, new FactorTerm(ldcf)),
		new Property(prop::oi, new FactorTerm(pPM->discriminantFactors(p,x))),
		new Property(prop::ni, polyterm) });
      }
      else
      {
	c->record(nextid,"assumption (unhandled)");
      }
    }
  } break;


    
  case (prop::sector): {
    SectionTermRef t0 = pt->getChild(0); SectionRef s0 = t0->getSection();
    SectionTermRef t1 = pt->getChild(1); SectionRef s1 = t1->getSection();
    SectionTermRef cbLower = new SectionTerm(c->getLowerBoundAtLevel(currentLevel));
    SectionTermRef cbUpper = new SectionTerm(c->getUpperBoundAtLevel(currentLevel));
    c->record(nextid,"sector surrounds cell", vector<TermRef>{
	new Property(prop::sector, cbLower, cbUpper),
	  new Property(prop::leq, t0, cbLower),
	  new Property(prop::leq, cbUpper, t1) });    
  } break;
    
  case (prop::leq): {
    SectionTermRef t0 = pt->getChild(0); SectionRef s0 = t0->getSection();
    SectionTermRef t1 = pt->getChild(1); SectionRef s1 = t1->getSection();
    if (s0->compare(s1) == 0)
      c->record(nextid,"leq is reflexive");
    else if (s0->getKind() == Section::negInfty && s1->getKind() == Section::posInfty)
      c->record(nextid,"negInfty leq posInfty");
    else if (s0->getKind() == Section::negInfty)
      c->record(nextid,"negInfty leq anything");
    else if (s1->getKind() == Section::posInfty)
      c->record(nextid,"anything leq posInfty");
    else if (s0->getKind() == Section::indexedRoot && s1->getKind() == Section::indexedRoot)
    {
      IntPolyRef p0 = s0->getPoly();
      IntPolyRef p1 = s1->getPoly();
      if (pt->level(c->getVarOrder()) == 1)
	c->record(nextid,"at level 1 leq_alpha ==> leq",vector<TermRef>{ new Property(prop::leq_alpha, t0, t1) });
      else if (p0.identical(p1)) // NOTE: these are canonical polys!
	c->record(nextid,"leq at alpha and ana-delin for single poly",vector<TermRef>{ new Property(prop::leq_alpha, t0, t1),
	      new Property(prop::ana_delin, new PolyTerm(p0)) });
      else
      {
	VarSet x = c->getVarOrder()->get(currentLevel);
	c->record(nextid,"leq at alpha and ana-delin for two polys", vector<TermRef>{
	    new Property(prop::leq_alpha, t0, t1),
	      new Property(prop::ana_delin, new PolyTerm(p0)),
	      new Property(prop::ana_delin, new PolyTerm(p1)),
	      new Property(prop::oi, new FactorTerm(c->getPolyManager()->resultantFactors(p0,p1,x))) });
      }
    }
    else
      throw TarskiException("Error in PropHandler! should not be possible to get here!");
  } break;
    
    
    
  default:
    std::cout << "don't know how to handle this property! assuming true" << std::endl;
    c->record(nextid,"assumption (unhandled)");
    break;
  }
}



