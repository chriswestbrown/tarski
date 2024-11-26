#include "uninterpform.h"
#include "nontarski.h"

#include "../tarski.h"
#include "bexpr.h"
#include "cwbpt.h"
#include "treemanip.h"
#include "../poly/factor.h"
#include "../formula/formmanip.h"
#include "../formula/normalize.h"

namespace tarski {
  

  // E must be a formula, not a real-valued expression
  // negate = true means that we have a negation to push inwards
  // S is the quantifier block stack
  // Init S to empty VarSet and S_qt to int -1 (for free vars)
  // and negate = false
  // clearProcess: 0 <- from my paper, 1 <- no guarding, 2 <- naive way (den /= 0)
  TFormRef divclear(BExpRef E, std::vector<VarSet>& S, std::vector<int>& S_qt,
		    PolyManager *pPM, bool negate, int clearProcess) {
#define _NO_DIVCLEAR_DEBUG_    
    if (isQuantifier(E->op())) {
      int qt = !negate ? E->op() : (E->op() == UNIVER ? EXIST : UNIVER);
      if (qt != S_qt.back()) {	
	S.push_back(VarSet());
	S_qt.push_back(qt);
      }
      VarSet V = S.back();
      for(int i = 0; i < E->nargs() - 1; i++)
	V = V + pPM->addVar(E->arg(i)->str());
      S.back() |= V;
      TFormRef res = divclear(E->arg(E->nargs() - 1),S,S_qt,pPM,negate,clearProcess);
#ifndef _NO_DIVCLEAR_DEBUG_          
      cout << "vars: ";
      for(auto itr = V.begin(); itr != V.end(); ++itr)
	cout << ' ' << pPM->getName(*itr);
      cout << endl;
#endif
      // put together result
      int cv = constValue(res);
      if (cv == FALSE || cv == TRUE)
	return res;
      else if (res->getTFType() != TF_QB) // i.e. res is not a quantifier block
	return new TQBObj(V,qt,res);
      else { // res is already a quantifier block
	asa<TQBObj>(res)->add(V,qt);
	return res;
      }
    }
    else if (E->op() == NOTOP) {
      return divclear(E->arg(0),S,S_qt,pPM,!negate,clearProcess);
    }
    else if (E->op() == ANDOP || E->op() == OROP) {
      TFormRef fres;
      vector<TFormRef> tmp;
      for(int i = 0; i < E->nargs(); i++)
	tmp.push_back(divclear(E->arg(i),S,S_qt,pPM,negate,clearProcess));
      if (E->op() == ANDOP && !negate || E->op() == OROP && negate) {
	TAndRef res = new TAndObj();
	for(int i = 0; i < tmp.size(); i++)
	  res->AND(tmp[i]);
	fres = res;
      }
      else {
	TOrRef res = new TOrObj();
	for(int i = 0; i < tmp.size(); i++)
	  res->OR(tmp[i]);
	fres = res;
      }
      return fres;
    }
    else if (isRelop(E->op())) {
      int relop = !negate ? E->op() : negateRelop(E->op());

      // translate to formula w/o divisions
      vector<BExpRef> res = translate(E->arg(0),relop,E->arg(1));
      BExpRef G = mkfun(relop,res[0],res[1]);

#ifndef _NO_DIVCLEAR_DEBUG_          
      // DEBUG OUTPUT
      std::cout << ppstr(G) << std::endl;
      std::cout << "Denom polys:";
      for(int i = 2; i < res.size(); i++)
	cout << (i != 2 ? ", " : " ") << ppstr(res[i]);
      cout << std::endl;
#endif
      
      // translate to TAtom
      TAtomRef A = convertBExpToTAtom(G,pPM);

      // factor, collect & translate denominator polys
      FactRef DPFs = new FactObj(*pPM);
      for(int i = 2; i < res.size(); i++)
	DPFs->addMultiple(pullOutIntPoly(res[i],pPM),1);
      DPFs->content = 1;
      for(auto itr = DPFs->factorBegin(); itr != DPFs->factorEnd(); ++itr)
	itr->second = 1;

#ifndef _NO_DIVCLEAR_DEBUG_          
      // DEBUG OUTOUT
      cerr << "As a TAtom: ";
      A->write();
      cerr << endl;
      cerr << "Denomiator Polys: ";
      DPFs->write();
      cerr << endl;
#endif
      
      Level3and4 localNormalizer(7,7);
      
      // Set up the array "N" from paper
      int k = S.size();
      vector< TOrRef > N(k+1);
      for(int i = 0; i <= k; i++)
	N[i] = new TOrObj();

      // Process the nullifying systems
      for(auto itr = DPFs->factorBegin(); itr != DPFs->factorEnd(); ++itr) {
	IntPolyRef p = itr->first;
	VarSet Vp = p->getVars();
	int s = k-1;
	while(s > 0 && (Vp & S[s]).isEmpty())
	  s--;
	VarSet V;
	int i = s+1;
	while(i > 0) {
	  TFormRef W = nullifySys(p,V,pPM);
	  RawNormalizer myNorm(localNormalizer);
	  myNorm.actOn(W);
	  TFormRef Wp = myNorm.getRes();
	  if (Wp->constValue() != FALSE)
	    N[i]->OR(Wp);
	  V = V + S[i-1];
	  i--;
	}
      }

      TFormRef H = A;
      if (clearProcess == 0) { //---- this is from the paper!
	for(int j = k; 0 < j; j--) {
#ifndef _NO_DIVCLEAR_DEBUG_          
	  cout << "H: ";
	  H->write();
	  cout << endl;
#endif	
	  if (S_qt[j-1] == UNIVER)
	    H = mkOR(N[j],H);
	  else {
	    TFormRef tmp = N[j]->negate();
#ifndef _NO_DIVCLEAR_DEBUG_          
	    cout << "tmp: ";
	    tmp->write();
	    cout << endl;
#endif
	    H = mkAND(tmp,H);
	  }
	}
      }
      else if (clearProcess == 1) { //---- no guard!
	; // noop
      }
      else if (clearProcess == 2) { //---- naive guarding!
	if (DPFs->numFactors() > 0)
	  H = mkAND(makeAtom(DPFs,NEOP),H);
      }
	
      RawNormalizer myNorm(localNormalizer);
      myNorm.actOn(H);
      H = myNorm.getRes();
      
#ifndef _NO_DIVCLEAR_DEBUG_                
      cout << "\nReWrite is: ";
      H->write();
      cout << endl;
#endif
      
      return H;
    }
    else {
      throw TarskiException("In non-tarski play, op not yet implemented!");
    }
  }

  UifObj::UifObj(TarskiRef T, PolyManager* pM) { E = toBExp(T,pM); this->pM = pM; }

  SRef UifObj::clear(int clearProcess) {
    std::vector<VarSet> S; S.push_back(VarSet());
    std::vector<int> S_qt; S_qt.push_back(-1);
    TFormRef res = divclear(E,S,S_qt,pM,false,clearProcess);
    return new TarObj(res);
  }



  
}

