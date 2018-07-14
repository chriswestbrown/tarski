#include "qepcad.h"

void QepcadCls::SETINPUTFORMULA(Word V, Word Fs)
{
  /* Set input-related variables */
  GVVL = V;
  GVF = Fs;
  FIRST4(Fs,&(GVNV.W),&(GVNFV.W),&(GVQ.W),&(GVQFF.W));
  
  /* Set default projection operator to modified McCallum */
  PCPROJOP = NIL;
  for (int k = GVNV; k >= 2; k--)
    PCPROJOP = COMP('p',PCPROJOP);
}

QepcadCls::QepcadCls(Word V, Word Fs) : UNSATCORE(*this)
{
  INITGLOBALS();
  INITSTATS();
  SETINPUTFORMULA(V,Fs);
}

void QepcadCls::SETASSUMPTIONS(Word F)
{
  Word Fp = CHANGEASSUMPTIONSLEVEL(F,GVNFV,1);
  if (Fp == TRUE || Fp == UNDET) return;
  GVUA = Fp;
}

Word QepcadCls::GETDEFININGFORMULA(char opt)
{
  Word SF;
  switch(opt)
  {
  case 'T': /* Tarski formula */
    SF = SFC3f(GVPC,GVPF,GVPJ,GVNFV,LIST10(0,0,0,1,0,3,2,4,1,5));
    break;
  case 'E': /* Extended Language Formula */
    SF = SFC3f(GVPC,GVPF,GVPJ,GVNFV,CCONC(LIST10(1,0,0,1,0,3,2,4,1,5),LIST1(-1)));
    break;
  case 'F': /* Full Dim only Formula */
    SF = SFCFULLDf(GVPC,GVPF,GVPJ,GVNFV);
    break;
  default:
    FAIL("GETDEFININGFORMULA","Error!  Unknown option!\n");
    break;
  }
  Word SFp = FMA2QUNF(SF,GVPF);
  return SFp;
}

void QepcadCls::UnsatCore::FactorInfo::WRITEFULL(Word V)
{
  SWRITE("(");
  IPDWRITE(r,Q,V);
  //  SWRITE(" {"); IWRITE(i); SWRITE(","); IWRITE(j); SWRITE("}");
  SWRITE(")");
  if (exp > 1) { SWRITE("^"); IWRITE(exp); }
}

void QepcadCls::UnsatCore::OrigAtom::clearTags()
{
  for(int i = 0; i < factors.size(); i++)
    factors[i].clearTag();
}


void QepcadCls::UnsatCore::prepareForLift()
{
  Word Forig = Qptr->GVQFF;
  Word OP = FIRST(Forig);
  if (OP < ANDOP || OP > 19) { OP = ANDOP; Forig = LIST2(ANDOP,Forig); }
  if (OP != ANDOP) FAIL("UnsatCore::prepareForLift","Formula not a conjunction!");

  // fill array with the atoms of the original formula
  for(Word L = RED(Forig); L != NIL; L = RED(L))
  {
    Word A = FIRST(L), sigma, p, r;
    //    SWRITE("Processing ... ");  ATOMFWR(Qptr->GVVL,A);  SWRITE("\n");
    FIRST3(A,&sigma,&p,&r);
    conjuncts.push_back(OrigAtom(r,p,sigma));
  }

  // set in factored form
  for(int k = 0; k < conjuncts.size(); k++)
  {
    OrigAtom &A = conjuncts[k];
    Word r = A.r;
    Word P = A.P;
    Word sigma = A.relop;
    Word s, c, L;
    IPFACDB(r,P,&s,&c,&L); // FACTOR
    if (ISIGNF(c)<= 0) { FAIL("QepcadCls::UnsatCore::prepareForLift","Unexpected non-positive sign!"); }
    A.sign = s;
    A.content = c;
    for(Word Lp = L; Lp != NIL; Lp = RED(Lp))
    {
      Word e, B;
      FIRST2(FIRST(Lp),&e,&B);

      // set i to the level of B and unwrap B until it is an r-variate poly in saclib representation
      Word Bp = B;
      int i = r;
      while (PDEG(Bp) == 0) {
	Bp = PLDCF(Bp);
	i--;
      }

      // find index j s.t. projection factor (i,j) is B
      Word Fi = LELTI(Qptr->GVPF,i);
      int j = 1;
      while(Fi != NIL && !EQUAL(LELTI(FIRST(Fi),PO_POLY),Bp)) { j++; Fi = RED(Fi); }
      if (Fi == NIL)
	FAIL("QepcadCls::UnsatCore::prepareForLift","Factor of atom not found in Projection Factors!");
      
      A.factors.push_back(FactorInfo(r,B,e,i,j));    
      appearsIn[pair<int,int>(i,j)].push_back(pair<int,int>(k,A.factors.size()-1));
    }
  }
  ;

  // print for debug
  /* for(int i = 0; i < conjuncts.size(); i++) */
  /* { */
  /*   //    IPDWRITE(conjuncts[i].r,conjuncts[i].P,Qptr->GVVL); */
  /*   if (i > 0) { SWRITE(" /\\ "); } */
  /*   conjuncts[i].WRITE(Qptr->GVVL); */
  /*   SWRITE(" "); */
  /*   RLOPWR(conjuncts[i].relop); */
  /*   SWRITE(" 0"); */
  /*  } */
  /* SWRITE("\n"); */
}

// START FROM TARSKI
static const int NOOP = 0;
static const int ALOP = 7;
/*********************************************************************
 ** These tables do sign deductions. e.g. if we know "p sigma_1 0"
 ** and "q sigma_2 0", then we know "p*q T_prod[sigma_1][sigma_2] 0".
 *********************************************************************/
static
//___________________NOOP_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
short T_square[8] = {NOOP,GTOP,EQOP,GEOP,GTOP,GTOP,GEOP,GEOP};
static
short T_prod[8][8] = {
  //______|NOOP_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
  /*NOOP*/{NOOP,NOOP,NOOP,NOOP,NOOP,NOOP,NOOP,NOOP},
  /*LTOP*/{NOOP,GTOP,EQOP,GEOP,LTOP,NEOP,LEOP,ALOP},
  /*EQOP*/{NOOP,EQOP,EQOP,EQOP,EQOP,EQOP,EQOP,EQOP},
  /*LEOP*/{NOOP,GEOP,EQOP,GEOP,LEOP,ALOP,LEOP,ALOP},
  /*GTOP*/{NOOP,LTOP,EQOP,LEOP,GTOP,NEOP,GEOP,ALOP},
  /*NEOP*/{NOOP,NEOP,EQOP,ALOP,NEOP,NEOP,ALOP,ALOP},
  /*GEOP*/{NOOP,LEOP,EQOP,LEOP,GEOP,ALOP,GEOP,ALOP},
  /*ALOP*/{NOOP,ALOP,EQOP,ALOP,ALOP,ALOP,ALOP,ALOP}
};
static
short T_sum[8][8] = {
  //______|NOOP_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
  /*NOOP*/{NOOP,NOOP,NOOP,NOOP,NOOP,NOOP,NOOP,NOOP},
  /*LTOP*/{NOOP,LTOP,LTOP,LTOP,ALOP,ALOP,ALOP,ALOP},
  /*EQOP*/{NOOP,LTOP,EQOP,LEOP,GTOP,NEOP,GEOP,ALOP},
  /*LEOP*/{NOOP,LTOP,LEOP,LEOP,ALOP,ALOP,ALOP,ALOP},
  /*GTOP*/{NOOP,ALOP,GTOP,ALOP,GTOP,ALOP,GTOP,ALOP},
  /*NEOP*/{NOOP,ALOP,NEOP,ALOP,ALOP,ALOP,ALOP,ALOP},
  /*GEOP*/{NOOP,ALOP,GEOP,ALOP,GTOP,ALOP,GEOP,ALOP},
  /*ALOP*/{NOOP,ALOP,ALOP,ALOP,ALOP,ALOP,ALOP,ALOP}
};

inline int signToSigma(int s) { return s < 0 ? LTOP : (s == 0 ? EQOP : GTOP); }

// returns true if relop a is consistent with relop b, i.e. x a 0 ==> x b 0
inline bool consistentWith(int a, int b)
{
static
short consistencyTable[8][8] = {
  //______|ZERO_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
  /*ZERO*/{ 0  , 0,   0,   0,   0,   0,   0,   0 },
  /*LTOP*/{ 0,   1,   0,   1,   0,   1,   0,   1 },
  /*EQOP*/{ 0,   0,   1,   1,   0,   0,   1,   1 },
  /*LEOP*/{ 0,   0,   0,   1,   0,   0,   0,   1 },
  /*GTOP*/{ 0,   0,   0,   0,   1,   1,   1,   1 },
  /*NEOP*/{ 0,   0,   0,   0,   0,   1,   0,   1 },
  /*GEOP*/{ 0,   0,   0,   0,   0,   0,   1,   1 },
  /*ALOP*/{ 0,   0,   0,   0,   0,   0,   0,   1 }
};
 return consistencyTable[a][b]; 
};

// Reverses relop ... e.g. for multipying both sides of an inequality by -1
inline int reverseRelop(int r) { static int T[] = {0,4,2,6,1,5,3,7}; return T[r]; }
inline int negateRelop(int r) { return ALOP ^ r; }

// Array gives strings for each of the relational operators
static const char *relopStrings[8] = { "ERR", "<", "=", "<=", ">", "/=", ">=", "??" };
inline string relopString(int r) { return relopStrings[r]; }


// END FROM TARSKI

void QepcadCls::UnsatCore::OrigAtom::WRITE(Word V)
{
  if (sign == 0) { SWRITE("0 = 0"); }
  else
  {
    if (sign < 0) SWRITE("-");
    if (ICOMP(content,1) != 0) { IWRITE(content); }
    for(int i = 0; i < factors.size(); i++)
      factors[i].WRITEFULL(V);
    SWRITE(" ");
    SWRITE(relopString(relop).c_str());
    SWRITE(" 0");
  }  
}



bool QepcadCls::UnsatCore::OrigAtom::isKnownFalse()
{
  int s = signToSigma(sign);
  for(int i = 0; i < factors.size(); i++)
  {    
    int sf = (factors[i].exp % 2 == 1) ? factors[i].tag : T_square[factors[i].tag];
    s = T_prod[s][sf];
  }
  return consistentWith(s,negateRelop(relop));
}


void QepcadCls::UnsatCore::record(Word C)
{
  // For the moment, all equational constraints will be returned
  // as part of the unsat core.  While undesireable, it is at least
  // correct which, for the moment will have to suffice.
  if (LELTI(C,HOWTV) == BYEQC) return;

  // Clear truth tags (i.e. set them all to UNDET)
  for(int i = 0; i < conjuncts.size(); i++)
    conjuncts[i].clearTags();
  
  int k = LELTI(C,LEVEL);

  // Record relevent sign information into "tag" fields
  // NOTE: I'm going to have to take into account
  //       things like equational constraints and
  //       the F quantifier to do this right
  Word S = LELTI(C,SIGNPF);
  for(int i = k; S != NIL; i--, S = RED(S))
  {
    Word Si = FIRST(S);
    for(int j = 1; Si != NIL; j++, Si = RED(Si))
    {
      int s = FIRST(Si);
      vector< pair<int,int> > &V = appearsIn[pair<int,int>(i,j)];
      for(int h = 0; h < V.size(); h++)
      {
	int a = V[h].first, b = V[h].second;
	conjuncts[a].factors[b].tag = signToSigma(s);
      }
    }
  }

  // go through each input atomic formula to determine if it's known to be false
  Word K = NIL;
  for(int i = 0; i < conjuncts.size(); i++)
  {
    if (conjuncts[i].isKnownFalse())
    {
      K = COMP(i,K);
    }
  }
  MHSP = COMP(K,MHSP);
}


void QepcadCls::UnsatCore::findUnsatCore(bool indexOnly)
{
  if (HITSETSOL == 0)
  {
    // Add equational constraints as size 1 sets.  Recall, right now
    // we adopt the expedient of making the unsat core include all
    // equational constraints.
    for(int i = 0; i < conjuncts.size(); i++)
    {
      if (conjuncts[i].relop == EQOP)
	MHSP = COMP(LIST1(i),MHSP);
    }

    HITSETSOL = MINHITSETSRDR(MHSP, -1);
  }
  
  int count = 0;
  for(Word L = HITSETSOL; L != NIL; L = RED(L))
  {
    if (indexOnly)
    {
      if (count++ > 0)
	SWRITE(", ");
      IWRITE(FIRST(L));
    }
    else
    {
      if (count++ > 0)
	SWRITE(" /\\ ");
      conjuncts[FIRST(L)].WRITE(Qptr->GVVL);
    }
  }
}

Word QepcadCls::UnsatCore::isApplicable()
{
  if (Qptr->GVNFV != 0) return FALSE; // Not a closed formula!
  for(Word L = Qptr->GVQ; L != NIL; L = RED(L))
    if (FIRST(L) != EXIST && FIRST(L) != FULLDE)
      return FALSE; // quantifier not either E or F!
  Word Forig = Qptr->GVQFF;
  Word OP = FIRST(Forig);
  if (OP < ANDOP || OP > 19) { OP = ANDOP; Forig = LIST2(ANDOP,Forig); }
  if (OP != ANDOP) return FALSE; // not a conjunction (or atom)!
  for(Word L = RED(Forig); L != NIL; L = RED(L))
  {
    Word A = FIRST(L);
    if (FIRST(A) < 0 || FIRST(A) > 7) // Not a relational operator, thus not an standard atom!
      return FALSE;
  }
  return TRUE;
}
