#include "gcword.h"
#include "../GC_System/GC_System.h"
#include "variable.h"


// Compares any references just by comparing the address of the obj to which they refer
template<class S, class T>
  //bool operator<(const GC_Hand<S> &a, const GC_Hand<T> &b) { return a.vpval() < b.vpval(); }
bool operator<(const GC_Hand<S> &a, const GC_Hand<T> &b) { return a->tag < b->tag; }

int OCOMP(Word a, Word b); // Object compare

int cmp(BDigit rp, Word a, Word b); // Comparison for saclib polynomials.  
                                    // Both pols must be r-level.  Compares by degree, 
                                    // then by leading coefficient, then by reductum


/*===========================================================================
			       n <- IORD2PPQ(A;t)

Integer, order of 2 + pure power query

Inputs
  A : in Z, A not 0.  

Outputs
  t : 1 if A is a pure power of 2, 0 otherwise
  n : (returned) the largest integer n such that 2^n divides A.
===========================================================================*/
Word IORD2PPQ(Word A, Word* t);

/*======================================================================
                      B <- GRNLBRN(A)

General rational number to logarithmic binary rational number.

Input
   A : a rational number, not necessarily binary rational

Output
   B : A in logarithmic representation if A is binary rational, else NIL
======================================================================*/
Word GRNLBRN(Word A);

/***************************************************************************
 ** PCcmp - CLASS DEFINITION
 **
 ** Comparison object for saclib polynomials.  Both pols must be r-level.
 ** Compares by degree, then by leading coefficient, then by reductum
 ***************************************************************************/
class PCmp 
{ 
public: 
  int r; 
  PCmp(int x) : r(x) { } 
  inline bool operator()(Word a, Word b) { return cmp(r,a,b) < 0; }
};

Word LFS(const std::string &s);
void SWRITE(const std::string &s);
Word varList(tarski::VarContext &C, tarski::VarSet S);

// Polynomial make main variable, preserve order
Word PMMVPO(Word r, Word A, Word i);

// Polynomial undo make main variable, preserve order
Word PUNDOMMVPO(Word r, Word A, Word i);

/*****************************************************
  r - number of variables
  A - an r-variate saclib polynomial
  L - a list (sigma_r,sigma_{r-1},...,sigma_1),
      sigma_i \in {LTOP,EQOP,LEOP,GTOP,NEOP,GEOP,ALOP}
      where we know that "x_i sigma_i 0" holds.
 *****************************************************/
Word signDeduce(Word r, Word A, Word L);

// Polynomial Unique Degree Vector
// r : a BETA-digit
// P : a polynomial in (x1 ... xr)
// Returns list (dr ... d1) s.t. xi appears only to the power di if di > -1,
// and appears to more than one power otherwise.
// WARNING! This may return the reverse of what you expect!
Word PUDV(Word r, Word P);

// Polynomial GCD of Exponents
// r : Beta digit
// P : polynomial in (x1 ... xr)
// i : 1 <= i <= r
// Returns gcd(e1,...,ek), where P = c1 xi^e1 + ... + ck xi^ek, ci \in D[x1 ... x_{i-1} x_{i+1} ... x_r]
Word PGCDEXP(Word r, Word P, Word i);

// Polynomial Reduce Degree in x_i
// r : Beta digit
// P : polynomial in x_1 ... x_r
// i : 1 <= i <= r
// d : integer s.t. if c x_i^k is a term in P, then d|k
// Returns a new poly Q in x_1 ... x_r s.t. P = Q(x_1,...,x_{i-1},x_i^d,x_{i+1},...,x_r)
Word PREDDEGXi(Word r, Word P, Word i, Word d);

/* 
Integeral Polynomial term of possibly negative (resp. positvie) sign
Return the first term of P which cannot be deduced to be >= 0.
r : BETA-digit
P : an element of Z[x_1,...,x_r]

 s : a comparison operator, i.e. EQOP, NEOP, ... 
 dv: a list (d_1,..,d_a) of non-negative BETA-digits

K : a list (s_r,...,s_1) of comparison operators
reverseflag : if true then the sense is reversed, i.e. "possibly positive sign".
NOTE: We consider x_i s_i 0 to be known. s is the "sign so far", dv the degree std::vector
*/
Word IPTERMOFPNS(Word r, Word P, Word K, bool reverseflag);
Word IPTERMOFPNShelp(Word r, Word P, Word s, Word dv, Word K, bool reverseflag);

/*
Integral polynomial variable statistics.
Inputs
r : BETA-digit
P : an element of Z[x_1,...,x_r]

Outputs
L : a list (L1, ... ,Lr), where Li = (di,tdi,nti), where
    di  = degree of P in x_i
    tdi = the maximum total degree of any term containing x_i
    nti = the number of terms containing x_i

 */
Word IPVARSTATS(Word r, Word P);

/*
Prefix of List
Inputs
L : a list
k : an integer, 0 <= k <= LENGTH(L)

Outputs
Lp: a list consisting of the first k elements of L
 */
Word PREFIXLIST(Word L, int k);

/*===========================================================================
			      IPDWRITEMAPLE(r,A,V)

Integral polynomial distributive write, Maple format (i.e. '*' for mult)

Input
  r : a non-negative BETA-integer.
  A : in Z[x_1,...,x_r].
  V : a list of at least r distinct variables.

Side effects
  A is written out to the output stream using the first r variables in V.
===========================================================================*/
void IPDWRITEMAPLE(Word r, Word A, Word V);

