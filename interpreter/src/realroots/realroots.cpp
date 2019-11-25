#include "realroots.h"
#include "../poly/tracker.h"

using namespace std;

extern Word CSSP(Word a, Word b); // defined elsewhere
extern void modIPIIS(Word A1,Word A2,Word I1,Word I2,Word t1,Word t2,Word *J1_,Word *J2_,Word *s_);


namespace tarski {

  
extern Tracker compTracker;

Word IPIIR1BISECT(Word A, Word I, Word t);



/// Binary rational interval compare
// return -1,0,1 if a comparison can be made, and -2 otherwise.
Word BRNICOMP(Word A, Word B) 
{
  Word A_L = RNLBRN(FIRST(A)), A_R = RNLBRN(SECOND(A));
  Word B_L = RNLBRN(FIRST(B)), B_R = RNLBRN(SECOND(B));
  Word t1 = LBRNCOMP(A_R,B_L);
  if (t1 < 0 || (t1 == 0 && !(EQUAL(A_L,A_R) && EQUAL(B_L,B_R))))
    return -1;
  if (t1 == 0) return 0;
  Word t2 = LBRNCOMP(B_R,A_L);
  if (t2 <= 0)
    return +1;
  return -2;
}


int RealRootIUPObj::compareToBrittle(RealAlgNumRef y) 
{
  RealRootIUPRef z = asa<RealRootIUPObj>(y);
  if (z.is_null()) { return -y->compareTo(this); }
  if (!EQUAL(sP,z->sP))
  {    
    Word J1, J2, s;
    modIPIIS(sP,z->sP,I,z->I,t,z->t,&J1,&J2,&s);

    while((s < 0 && EQUAL(SECOND(J1),FIRST(J2))) || (s > 0 && EQUAL(SECOND(J2),FIRST(J1))))
    {
      J1 = IPIIR1BISECT(sP,J1,t);
      J2 = IPIIR1BISECT(z->sP,J2,z->t);
    }

    I = J1;
    z->I = J2;
    return s;
  }
  else
  {
    if (j == z->j) 
    { 
      Word J1 = I, J2 = z->I; 
      switch(LSICOMP(J1,J2))
      {
      case -2: case -1: 
	{
	  string m1 = "Error in RealRootIUP->compareToBrittle: bad case!",
	    m2 = this->toStr(),
	    m3 = y->toStr();
	  throw TarskiException(m1 + " " + m2 + " " + m3);	    
	  break;
	}
      case 1: z->I = J1; break;
      case 2: I = J2; break;
      case 0: default: break;
      }
      return 0;
    }
    return j < z->j ? -1 : 1;
  }
}


int RealRootIUPObj::compareToRobust(RealAlgNumRef y) 
{
  if (equal(this,y)) { return 0; }
  return this->compareToBrittle(y);
}


bool RealRootIUPObj::separate(RealAlgNumRef y) 
{
  if (equal(this,y)) { return false; }

  RealRootIUPRef z = asa<RealRootIUPObj>(y);
  if (z.is_null()) { return true; }
  if (!EQUAL(sP,z->sP))
  {    
    Word J1, J2, s;
    modIPIIS(sP,z->sP,I,z->I,t,z->t,&J1,&J2,&s);

    while((s < 0 && EQUAL(SECOND(J1),FIRST(J2))) || (s > 0 && EQUAL(SECOND(J2),FIRST(J1))))
    {
      J1 = IPIIR1BISECT(sP,J1,t);
      J2 = IPIIR1BISECT(z->sP,J2,z->t);
    }

    I = J1;
    z->I = J2;
    return true;
  }
  else
  {
    if (j == z->j) { return false; }
    else
    {
      Word J1 = I, J2 = z->I;
      while((j < z->j && EQUAL(SECOND(J1),FIRST(J2))) || (j > z->j && EQUAL(SECOND(J2),FIRST(J1))))
      {
	J1 = IPIIR1BISECT(sP,J1,t);
	J2 = IPIIR1BISECT(z->sP,J2,z->t);
      }
      I = J1;
      z->I = J2;
    }
    return true;
  }
}


string RealRootIUPObj::toStr() 
{ 
  ostringstream sout;
  PushOutputContext(sout);
  SWRITE("[");
  IPDWRITE(1,sP,LIST1(LFS("x")));
  SWRITE(",");
  IWRITE(j);
  SWRITE(",");
  LBRIWRITE(I);
  SWRITE(",");
  IWRITE(t);
  SWRITE("]");
  PopOutputContext();
  return sout.str();
}

  Word RealRootIUPObj::tryToGetExactRationalValue()
  {
    if (PDEG(sP) == 1)
      return RED2(sP) == NIL ? RNINT(0) : RNRED(INEG(PTRCF(sP)),PLDCF(sP));
    return NIL;
  }

RealAlgNumRef integerToRealAlgNum(Word I) 
{ 
  Word b = ILBRN(I);
  Word A = I == 0 ? LIST2(1,1) : LIST4(1,1,0,INEG(I));
  return new RealRootIUPObj(A,1,LIST2(b,b),1,1); 
}

RealAlgNumRef rationalToRealAlgNum(Word R) // CHANGED 1/22/17
{ 
  if (R == 0) { return integerToRealAlgNum(0); }
  Word R1, R2;
  FIRST2(R,&R1,&R2);
  Word A = LIST4(1,R2,0,INEG(R1));

  //DEBUG
  return new RealRootIUPObj(A,1,FIRST(IPRRID(A)),1,1); 

  // Word x = GRNLBRN(R);
  // Word I;
  // if (x != NIL) // R is a binary rational
  //   I = LIST2(x,x);
  // else    
  // {
  //   interval H;
  //   BDigit flag;
  //   RHI(R1,R2,&H,&flag);
  //   if (flag == 1) // R successfully represented as a hardware interval
  //   {
  //     I = HILBRI(H);
  //   }
  //   else
  //     I = FIRST(IPRRID(A));
  // }
  // return new RealRootIUPObj(A,1,I,1,1); 
}

vector<RealRootIUPRef> RealRootIsolateSquarefree(Word p) // p must be a non-zero, squarefree integral univariate saclib poly
{
  vector<RealRootIUPRef> res;
  if (PDEG(p) != 0)
  {
    compTracker.recordSacPoly("rri",1,p);
    Word L = IPRRID(p);
    Word numRoots = LENGTH(L);
    Word M = ALSIL(p,L);
    Word I, A, t, N = M;  
    for(int j = 1; N != NIL; N = RED(N), ++j) { FIRST3(FIRST(N),&I,&A,&t); res.push_back(new RealRootIUPObj(A,j,I,t,numRoots)); }
  }
  return res;
}

std::vector<RealRootIUPRef> RealRootIsolateRobust(IntPolyRef p) // p must be a non-zero univariate poly
{
  // Easy cases
  Word A = p->sP;
  if (p->isZero()) { throw RealRootException("cannot isolate roots of the zero polynomial"); }
  if (p->isConstant()) { return std::vector<RealRootIUPRef>(); } 
  if (p->numVars() > 1) {  throw RealRootException("root isolation requires a univariate polynomial"); }

  // Isolate roots of squarefree factors
  Word L = IPSF(1,A);
  vector<int> multiplicity;
  vector< vector<RealRootIUPRef> > roots;
  for(Word Lp = L; Lp != NIL; Lp = RED(Lp)) {
    Word e, P;
    FIRST2(FIRST(Lp),&e,&P);
    roots.push_back(RealRootIsolateSquarefree(P));
    multiplicity.push_back(e); }

  return mergeMultipleBrittle(roots);
  
  // Merge squarefree factors' roots lists
  // vector<RealRootIUPRef> lold(roots[0]);
  // for(int i = 1; i < roots.size(); ++i)
  // {
  //   vector<RealRootIUPRef> lnew;
  //   int jo = 0, jn = 0;
  //   while(jo < lold.size() && jn < roots[i].size()) {
  //     if (roots[i][jn].compareToBrittle(lold[jo]) < 0)
  // 	lnew.push_back(roots[i][jn++]);
  //     else
  // 	lnew.push_back(lold[jo++]);
  //   }
  //   while(jo < lold.size())
  //     lnew.push_back(lold[jo++]);
  //   while(jn < roots[i].size())
  //     lnew.push_back(roots[i][jn++]);
  //   swap(lnew,lold);    
  // }
  // return lold;
  /* OLD
  Word Ap = IPDMV(1,A), GCD, C, Cp;
  IPGCDC(1,A,Ap,&GCD,&C,&Cp);
  return RealRootIsolateSquarefree(C); */
}

  

Word IPIIR1BISECT(Word A, Word I, Word t)
{
       Word  a,b,c,h,J,s;

Step1: /* Decompose. */
       FIRST2(I,&a,&b);

Step2: /* One-point interval. */
       if (EQUAL(a,b) == 1)
	  goto Step5;

Step4: /* Bisect as needed. */
       c = LSIM(a,b);
       s = IUPBRES(A,c);
       if (s == 0) {
	 a = c;
	 b = c;
	 goto Step5; }
       if (s == t)
	 b = c;
       else
	 a = c;

Step5: /* Compose. */
       J = LIST2(a,b);

Return: /* Return. */
       return(J);
}

Word rationalPointInInterval(RealAlgNumRef X, RealAlgNumRef Y)
{
  bool separated = true;
  RealRootIUPRef x1 = asa<RealRootIUPObj>(X);
  RealRootIUPRef x2 = asa<RealRootIUPObj>(Y);
  if (!x1.is_null()) { separated = x1->separate(Y); }
  else if (!x2.is_null()) { separated = x2->separate(X); }
  else if (!X->isFinite() && !Y->isFinite())
  {
    separated = X->compareToNegInfty() == 0 && Y->compareToPosInfty() == 0;
  }
  else
  {
    throw TarskiException("Error! Unknown RealAlgNum type in rationalPointInInterval!");
  }
  if (!separated) { cout << "Error in rationalPointInInterval! could not separate " 
			 << X->toStr() << " and " << Y->toStr() << endl; return NIL; }

  // NOTE: at this point we know that X and Y are distinct, so we can use compareToBrittle

  int c = X->compareToBrittle(Y);
  if (c >= 0) { cout << "Error in rationalPointInInterval! give empty open interval (" 
		     << X->toStr() << "," << Y->toStr() << ")" << endl; return NIL; }
  Word lb = X->LBRNPointAbove();
  Word ub = Y->LBRNPointBelow();
  Word Z = CSSP((lb == NIL ? lb : LBRNRN(lb)) , (ub == NIL ? ub : LBRNRN(ub)));
  return Z;
}

bool equal(RealAlgNumRef a, RealAlgNumRef b)
{
  RealRootIUPRef x1 = asa<RealRootIUPObj>(a);
  RealRootIUPRef x2 = asa<RealRootIUPObj>(b);
  if (x1.is_null() && x2.is_null())
    return a->compareTo(b) == 0;
  else if (x1.is_null() || x2.is_null())
    return false;
  else
  {
    if (x1->singlePointInterval() && x2->singlePointInterval())
      return EQUAL(x1->isolatingInterval(),x2->isolatingInterval());
    
    if (LBRNCOMP(x1->LBRNPointAbove(),x2->LBRNPointBelow()) <= 0 ||
	LBRNCOMP(x2->LBRNPointAbove(),x1->LBRNPointBelow()) <= 0)
      return false;
    Word C, Ab, Bb; IPGCDC(1,x1->saclibPoly(),x2->saclibPoly(),&C,&Ab,&Bb);
    if (PDEG(C) > 0) { 
      Word s1a = IUPBRES(C,x1->LBRNPointAbove()), s1b = IUPBRES(C,x1->LBRNPointBelow());
      Word s2a = IUPBRES(C,x2->LBRNPointAbove()), s2b =  IUPBRES(C,x2->LBRNPointBelow());
      if (s1a*s1b < 0 && s2a*s2b < 0) { return true; }
      if (s1a*s1b == 0 && EQUAL(x1->LBRNPointAbove(),x1->LBRNPointBelow())
          && (s2a*s2b < 0 || (s2a*s2b == 0 && EQUAL(x2->LBRNPointAbove(),x2->LBRNPointBelow())))) return true;
      if (s2a*s2b == 0 && EQUAL(x2->LBRNPointAbove(),x2->LBRNPointBelow())
          && (s1a*s1b < 0 || (s1a*s1b == 0 && EQUAL(x1->LBRNPointAbove(),x1->LBRNPointBelow())))) return true;
      
    }
    return false;
  }
}

void RealRootIUPObj::refineTo(int logOfWidth)
{
  if (singlePointInterval()) { return; }
  I = IPIIRB(sP,I,t,logOfWidth);
}

void RealRoot1ExtObj::refineTo(int logOfWidth)
{
  // Single point intervals can't be refined!
  if (singlePointInterval()) { return; }

  // Get width
  Word I = this->I;
  int w_now = LBRIW(I);
  Word num, dp2;
  FIRST2(w_now,&num,&dp2);
  Word k = ILOG2(num) - dp2;
  if (k <= logOfWidth) return;

  // get the root poly M & interval I_M that defines the extension
  Word M = alpha->saclibPoly();
  Word I_M = alpha->isolatingInterval();
  Word I_Mp = LBRIBRI(I_M);

  // get current interval width & refine enough times
  Word Ip = LBRIBRI(I);
  while(k > logOfWidth)
  {
    Ip = AFUPIIR(M,I_Mp,this->A,Ip);  
    --k;
  }
  this->I = BRILBRI(Ip);
}

// -1 if this is less than x, 0 if this is equal to x, +1 if this is greater than x
int RealRootIUPObj::compareToLBRN(Word x)
{
  if (singlePointInterval()) return LBRNCOMP(LBRNPointAbove(),x);
  if (LBRNCOMP(LBRNPointAbove(),x) <= 0) return -1;
  if (LBRNCOMP(LBRNPointBelow(),x) >= 0) return +1;
  int s = IUPBRES(sP,x);
  return s == getTrend() ? -1 : +1;
}

double approximateLBRN(Word x, int dir)
{  
  ieee res;
  BDigit u;
  LBRNIEEE(x,dir,&res,&u);
  if (u == 0) { throw RealRootException("overflow/underflow in conversion to hardware double."); }
  return res.num;
}


double RealRootIUPObj::approximate(int dir)
{
  ieee res;
  BDigit u;
  Word a, b;
  FIRST2(I,&a,&b);
  if (dir == -1) LBRNIEEE(a,-1,&res,&u);
  else LBRNIEEE(b,1,&res,&u);
  if (u == 0) { throw RealRootException("overflow/underflow in conversion to hardware double."); }
  return res.num;
}

double RealRoot1ExtObj::approximate(int dir)
{
  ieee res;
  BDigit u;
  Word a, b;
  FIRST2(I,&a,&b);
  if (dir == -1) LBRNIEEE(a,-1,&res,&u);
  else LBRNIEEE(b,1,&res,&u);
  if (u == 0) { throw RealRootException("overflow/underflow in conversion to hardware double."); }
  return res.num;
}

// This assumes that no roots a \in A and b \in B are such that a and b have the same defining polynomial sP
void mergeBrittle(vector<RealRootIUPRef> &A, vector<RealRootIUPRef> &B, vector<RealRootIUPRef> &dest)
{
  unsigned int i = 0, j = 0;
  while(i < A.size() && j < B.size())
    dest.push_back(A[i]->compareToBrittle(B[j]) < 0 ? A[i++] : B[j++]);
  while(i < A.size())
    dest.push_back(A[i++]);
  while(j < B.size())
    dest.push_back(B[j++]);  
}

// Yes I know this copies.  If I don't do that I modify the passed list, which I don't want to do
vector<RealRootIUPRef> mergeMultipleBrittle(vector< vector<RealRootIUPRef> > R) {
  while(R.size() > 1)
  {
    int n = R.size(), odd = n%2;
    vector< vector<RealRootIUPRef> > Rp(n/2 + odd);
    for(int i = 0; i < n - 1; i += 2)
      mergeBrittle(R[i],R[i+1],Rp[i/2]);
    if (odd)
      Rp[n/2] = R[n-1];
    swap(Rp,R);
  }
  return R[0];
}

vector<RealRootIUPRef> RealRootIsolate(FactRef F)
{
  // isolate real roots of each factor
  vector< vector<RealRootIUPRef> > R;
  for(FactObj::factorIterator itr = F->factorBegin(); itr != F->factorEnd(); ++itr)
    R.push_back(RealRootIsolateSquarefree(itr->first));

  return mergeMultipleBrittle(R);
  
  // merge roots from factors
  // while(R.size() > 1)
  // {
  //   int n = R.size(), odd = n%2;
  //   vector< vector<RealRootIUPRef> > Rp(n/2 + odd);
  //   for(int i = 0; i < n - 1; i += 2)
  //     mergeBrittle(R[i],R[i+1],Rp[i/2]);
  //   if (odd)
  //     Rp[n/2] = R[n-1];
  //   swap(Rp,R);
  // }
  // return R[0];
}



vector<RealRoot1ExtRef> RealRootIsolate(IntPolyRef p, int tmpOrderFlag, RealRootIUPRef alpha) 
{
  Word A = p->getSaclibPoly();
  Word r = p->getLevel();
  if (r != 2) {  throw RealRootException("In RealRootIsolate: level of poly must = 2 not yet implemented."); }
  Word M = alpha->saclibPoly();
  Word I = alpha->isolatingInterval();
  Word rep_alpha = LIST2(RNINT(1),LIST2(1,1)); // rep of alpha as (1/1,x)
  Word B = IPAFME(2,M,A,LIST1(rep_alpha)); // B is A(alpha)
  Word Bm = AFPMON(1,M,B); // Bm is the monic associate of B
  SWRITE("B  = "); AFUPWRITE(B,LFS("y"),LFS("a")); SWRITE("\n");
  SWRITE("Bm = "); AFUPWRITE(Bm,LFS("y"),LFS("a")); SWRITE("\n");
  SWRITE("Bm as saclib object = "); OWRITE(Bm); SWRITE("\n");


  Word tmp_s = AFUPSR(M,I,B,LIST2(1,2));
  SWRITE("A(alpha,1/2) = B(1/2) has sign "); IWRITE(tmp_s); SWRITE("\n");
  

  Word SFB = AFUPSF(M,Bm); // SFB is the squarefree factorization of Bm
  Word Basis = NIL;
  for(Word K = SFB; K != NIL; K = RED(K))
  {
    Basis = COMP(SECOND(FIRST(K)),Basis);
    SWRITE("basis poly = "); AFUPWRITE(SECOND(FIRST(K)),LFS("y"),LFS("a")); SWRITE("\n");
  }
  Basis = INV(Basis);
		
  vector<RealRoot1ExtRef> V;    
  Word L = NIL;
  BDigit t = 0;
  AFUPHIBRI(M,I,Basis, &L, &t);
  int numRoots = 0;
  for(Word Lp = L; Lp != NIL; Lp = RED2(Lp))
  {
    Word B1, I1;
    FIRST2(Lp,&I1,&B1);
    RealRoot1ExtRef r = new RealRoot1ExtObj(alpha,B1,I1,++numRoots);
    cerr << r->toStr() << endl;
    V.push_back(r);
  }
  return V;
}
}//end namespace tarski
