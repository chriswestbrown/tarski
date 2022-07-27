#include "sacpolyutils.h"
#include "../formrepconventions.h"
#include <vector>
using namespace std;
using namespace tarski;

int OCOMP(Word a, Word b) // Object compare
{
  if (ISATOM(a) && ISATOM(b)) return SIGN(a-b);
  if (ISATOM(a)) return -1;
  if (ISATOM(b)) return 1;
  if (a == NIL) return -1*(b != NIL);
  if (b == NIL) return 1;
  int t = OCOMP(FIRST(a),FIRST(b));
  if (t == 0) t = OCOMP(RED(a),RED(b));
  return t;
}

int cmp(BDigit rp, Word a, Word b)
{
  if (rp == 0) return ICOMP(a,b);
  if (a == NIL && b != NIL) return -1;
  if (a != NIL && b == NIL) return 1;
  if (a == 0 && b == 0) return 0;
  if (PDEG(a) < PDEG(b)) return -1;
  if (PDEG(a) > PDEG(b)) return 1;
  int t = cmp(rp-1,PLDCF(a),PLDCF(b));
  if (t != 0) return t;
  return cmp(rp,PRED(a),PRED(b));
}

Word IORD2PPQ(Word A, Word* t)
{
  Word a,Ap,n, tval;

Step1: /* Count low-order BETA-digits. */
       n = 0;
       if (A < BETA)
	  a = A;
       else {
	  Ap = A;
	  while (FIRST(Ap) == 0) {
	     Ap = RED(Ap);
	     n = n + ZETA; }
	  a = FIRST(Ap); }

Step2: /* Count low-order bits. */
       a = absm(a);
       while ((a & 01) == 0) {
	  a = a >> 1;
	  n = n + 1; }
       tval = (RED(Ap) == NIL && (a ^ (01 == 0)));

Return: /* Prepare for return. */
       *t = tval;
       return(n);
}


Word GRNLBRN(Word A)
{
  Word a,b,k,B,tval;

Step1: /* A = 0. */
       tval = TRUE;
       if (A == 0) {
	  B = 0;
	  goto Return; }

Step2: /* A /= 0. */
       FIRST2(A,&a,&b);
       if (b == 1) {
	  k = IORD2(a);
	  a = ITRUNC(a,k);
	  k = -k;
       }
       else
       {
	 k = IORD2PPQ(b,&tval);
       }
       B = LIST2(a,k);

Return: /* Prepare for return. */
       if (!tval)
	 B = NIL;
       return(B);
}

Word LFS(const string &s)
{
  Word L = NIL;
  for(int i = s.length() - 1; i >= 0; --i) 
  {
    L = COMP(Word(s[i]),L);
  }
  return L;
}

void SWRITE(const string &s)
{
  for(unsigned int i = 0; i < s.length(); ++i) CWRITE(s[i]);
}

Word varList(VarContext &C, VarSet S)
{
  Word L = NIL;
  for(VarSet::iterator i = S.begin(); i != S.end(); ++i)
    L = COMP(LFS(C.getName(*i)),L);
  L = CINV(L);
  return L;
}

// Polynomial make main variable, preserve order
Word PMMVPO(Word r, Word A, Word i)
{
  Word P = LIST1(i);
  for(int k = r; k > 0; --k)
  {
    if (k != i)  P = COMP(k,P);
  }
  Word Ap = PPERMV(r,A,P);
  return Ap;
}

// Polynomial undo make main variable, preserve order
Word PUNDOMMVPO(Word r, Word A, Word i)
{
  Word P = NIL;
  int k;
  for(k = r-1; k >= i; --k)
    P = COMP(k,P);
  P = COMP(r,P);
  for(; k > 0; --k)
    P = COMP(k,P);
  
  Word Ap = PPERMV(r,A,P);
  return Ap;
}

Word signDeduce(Word r, Word A, Word L)
{
  if (A == 0) return EQOP;
  if (r == 0)   return signToSigma(ISIGNF(A));
  int S_x_r = FIRST(L);
  int S_A = EQOP; // This will eventually be the sign of A
  Word Lp = RED(L);
  for(Word Ap = A; Ap != NIL; Ap = RED2(Ap))
  {
    Word e = FIRST(Ap), c = SECOND(Ap); // term = c * x_r^e
    int S_pp = e == 0 ? GTOP : (e % 2 == 1 ? S_x_r : T_square[S_x_r]);
    int S_term = T_prod[S_pp][signDeduce(r-1,c,Lp)];
    S_A = T_sum[S_term][S_A];
  }
  return S_A;
}

inline Word specf(Word a, Word b) { if (a == 0) return b; if (b == 0) return a; return a == b ? a : -1; }
Word PUDV(Word r, Word P)
{
  Word L = NIL;
  if (P == 0) { for(int i = 0; i < r; ++i) L = COMP(0,L); }
  else if (r == 1) 
  {
    if (PDEG(PRED(P)) <= 0)
      L = LIST1(PDEG(P));
    else
      L = LIST1(-1);
  }
  else
  {
    L = COMP(PDEG(P),PUDV(r-1,PLDCF(P)));
    for(Word Pp = PRED(P); Pp != 0; Pp = PRED(Pp))
    {
      Word T = NIL;
      for(Word N = COMP(PDEG(Pp),PUDV(r-1,PLDCF(Pp)));L != NIL; L = RED(L), N = RED(N))
	T = COMP(specf(FIRST(L),FIRST(N)),T);
      L = CINV(T);
    }
  }
  return L;
}

Word PGCDEXP(Word r, Word P, Word i)
{
  Word g = 0;
  if (r == i)
  {
    Word L = NIL;
    for(Word Pp = P; Pp != 0; Pp = PRED(Pp)) { L = COMP(PDEG(Pp),L); }
    for(;g != 1 && L != NIL; L = RED(L))
      g = DGCD(FIRST(L),g); // NOTE: since g <= FIRST(L), I can call DGCD
  }
  else
  {
    for(Word Pp = P; Pp != 0; Pp = PRED(Pp))
      g = IGCD(PGCDEXP(r-1,PLDCF(Pp),i),g);
  }
  return g;
}

// Lists of digits GCDs
Word LDGCD(Word L1, Word L2)
{
  return L1 == NIL ? NIL : COMP(IGCD(FIRST(L1),FIRST(L2)),LDGCD(RED(L1),RED(L2)));
}

static Word PGCDEXPALLrev(Word r, Word P)
{
  if (r == 0) return NIL;
  if (P == 0) return COMP(0,PGCDEXPALLrev(r-1,0));
  Word g = 0, Lp = NIL;
  for(Word Pp = P; Pp != NIL; Pp = RED2(Pp)) {
    Word e, c, Tp;
    FIRST2(Pp,&e,&c);
    g = IGCD(g,e);
    Tp = PGCDEXPALLrev(r-1,c);
    Lp = r == 1 || Pp == P ? Tp : LDGCD(Lp,Tp);
  }
  return COMP(g,Lp);
}
Word PGCDEXPALL(Word r, Word P) { return CINV(PGCDEXPALLrev(r,P)); }


Word PREDDEGXi(Word r, Word P, Word i, Word d)
{
  if (i == r)
  {
    if (PDEG(P) == 0) 
      return P;
    else
      return COMP2(PDEG(P)/d,PLDCF(P),PRED(P) == 0 ? NIL : PREDDEGXi(r,PRED(P),i,d));
  }
  else
    return COMP2(PDEG(P),PREDDEGXi(r-1,PLDCF(P),i,d),PRED(P) == 0 ? NIL : PREDDEGXi(r,PRED(P),i,d));
}


Word IPTERMOFPNS(Word r, Word P, Word K, bool reverseflag)
{
  return IPTERMOFPNShelp(r, P, 4 /*i.e. GTOP, since we start with an implicit "1"*/, NIL, K, reverseflag);
}

Word IPTERMOFPNShelp(Word r, Word P, Word s, Word dv, Word K, bool reverseflag)
{
  // P is constant
  if (P == 0) return NIL;
  if (r == 0)
  {
    int t = T_prod[signToSigma(ISIGNF(P))][s];
    if ((t & LTOP && !reverseflag) || (t & GTOP && reverseflag ))
      return LIST3(P,s,dv);
    else
      return NIL;
  }

  // x_r has "known" sign of zero
  if (FIRST(K) == EQOP) { return IPTERMOFPNShelp(r,PRED(P),s,dv,K,reverseflag); }

  // Otherwise check for term of PNS in lcdf, failing that recurse on reductum
  Word k = PDEG(P);
  Word s1 = k % 2 == 1 ? FIRST(K) : T_square[FIRST(K)];
  Word s2 = T_prod[s1][s];
  Word a = IPTERMOFPNShelp(r-1,PLDCF(P),s2,COMP(k,dv),RED(K),reverseflag);
  if (a != NIL) 
    return a;
  else
    return IPTERMOFPNShelp(r,PRED(P),s,dv,K,reverseflag);
}



// IMPLEMENTING: Word IPVARSTATS(Word r, Word P)
class Trip
{
public:
  int A[3];
  Trip(int a = 0, int b = 0, int c = 0) { A[0] = a; A[1] = b; A[2] = c; }  
};
Trip add(Trip a, Trip b)
{
  return Trip(max(a.A[0],b.A[0]),max(a.A[1],b.A[1]),a.A[2] + b.A[2]);
}

// return [degOfRp1,maxtd,ntrms]
Trip help(Word r, Word P, int degOfRp1,int TDSoFar,vector<Trip> &varstats) 
{
  Trip res;
  if (P == 0) { return res; }
  if (r == 0) { return Trip(degOfRp1,TDSoFar,1); }
  for(Word L = P; L != NIL; L = RED2(L))
  {
    Word d, C;
    FIRST2(L,&d,&C);
    Trip resp;
    resp = help(r-1,C,d,TDSoFar+d,varstats);
    if (d > 0)
      varstats[r] = add(varstats[r],resp);
    res = add(res,resp);
  }
  res.A[0] = degOfRp1;
  return res;
}

Word IPVARSTATS(Word r, Word P)
{
  vector<Trip> varstats(r+1);
  help(r,P,0,0,varstats);
  Word L = NIL;
  for(int i = r; i >= 1; --i)
    L = COMP(LIST3(varstats[i].A[0],varstats[i].A[1],varstats[i].A[2]),L);
  
  return L;
}

Word PREFIXLIST(Word L, int k)
{
  Word Lp = NIL;
  for(int i = 0; i < k; L = RED(L), i++)
    Lp = COMP(FIRST(L),Lp);
  return CINV(Lp);
}

/*===========================================================================
			      IPDWRITE(r,A,V)

Integral polynomial distributive write.

Input
  r : a non-negative BETA-integer.
  A : in Z[x_1,...,x_r].
  V : a list of at least r distinct variables.

Side effects
  A is written out to the output stream using the first r variables in V.
===========================================================================*/
#include "saclib.h"

void IPDWRITEMAPLE(Word r, Word A, Word V)
{
       Word Ap,E,Ep,Vp,a,e,l,s,t,v;

Step1: /* r=0 or A=0. */
       Ap = DIPFP(r,A);
       if (r == 0 || Ap == 0) { 
          IWRITE(Ap);
          goto Return; }
       l = 1;

Step2: /* General case. */
        do {
	   ADV2(Ap,&a,&E,&Ap);
	   s = ISIGNF(a); 
	   a = IABSF(a);
	   if (s > 0 && l == 0) 
	      CWRITE('+');
	   if (s < 0) 
	      CWRITE('-');
	   if (a != 1) {
	      if (l == 0) 
		 CWRITE(' '); 
	      IWRITE(a); 
	      t = 1; }
	   else
	      t = 0;
	   Ep = CINV(E); 
	   Vp = V;
	   do {
	      ADV(Ep,&e,&Ep); 
	      ADV(Vp,&v,&Vp);
	      if (e > 0) {
          if ((l == 0 && t != 0) || t == 1)
		   CWRITE('*');
		 else if (l == 0 && t == 0)
 		   CWRITE(' ');
		 VWRITE(v); 
		 t = 1; }
	      if (e > 1) { 
		 CWRITE('^'); 
		 AWRITE(e); } }
	   while (Ep != NIL);
	   if (t == 0) { 
	      if (l == 0) 
		 CWRITE(' '); 
	      CWRITE('1'); }
	   if (Ap == NIL)
	      goto Return;
	   CWRITE(' ');
	   l = 0; } 
       while (1);

Return: /* Prepare for return. */
       return;
}

/*
Integer Polynomial Size Statistics
Input
  r : a non-negative BETA-integer.
  A : in Z[x_1,...,x_r].
Outputs
  nt  : number of terms in A
  sotd: sum of the total degrees of all terms in A
  mcbl: the maximum bit-length of any coefficint (zero for A = 0)
  t   : the sign of the constant coefficient
 */
int IPSIZESTATS(Word r, Word A, Word *nt, Word *sotd, Word* mcbl)
{
  Word nnt = 0, nsotd = 0, nmcbl = 0, t = 0;
  if (r == 0)
  {
    nnt = 1; nsotd = 0; nmcbl = ILOG2(A); t = ISIGNF(A);
  }
  else
  {
    for(Word Ap = A; Ap != NIL; Ap = RED2(Ap))
    {
      Word e, c, rnt, rsotd, rmcbl, rt;
      FIRST2(Ap,&e,&c);
      rt = IPSIZESTATS(r-1,c,&rnt,&rsotd,&rmcbl);
      nnt += rnt;
      nsotd += e*rnt + rsotd;
      nmcbl = std::max(nmcbl,rmcbl);
      if (e == 0) t = rt;
    }
  }
  *nt = nnt; *sotd = nsotd; *mcbl = nmcbl;
  return t;
}
