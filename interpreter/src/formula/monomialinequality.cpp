#include "monomialinequality.h"

using namespace std;

namespace tarski {




/*
The monomial inequality framework starts with 
a conjunction C (we'll assume in factored form) and
does the following:
-1) remove content and true/false atoms.
0) For each distinct factor f determines whether f
   is "Strict" or not, where f is strict in C if it
   appears in a <, > or /= atom.
1) Maps distinct factors in C to distinct variables
   in x_1,...,x_n where variables x_1,..,x_r
   correspond to strict factors and x_{r+1},...,x_n
   to non-strict factors.
2) /= atoms are thrown away (implicit in the
   strict/nonstrict representation) and = atoms are
   transformed into the product of the squares or the
   x_i's in the equality set <= 0.  Atoms that are
   <= (resp. >=) and contain only strict variables
   are set to < (resp. >).

--- at this point we have the monomial inequalities

3) Each inequality is replaced with a vector v s.t.
   v[i] = the exponent of x_i for i > 0, and
   v[0] = 0 for > and >=, and 1 for < and <=.

A) Any such vector can be "normalized" by replacing
   v[i] = v[i] % 2 for 1 <= i <= r and
   v[i] = v[i] % 2 == 1 ? 1 : (v[i] == 0 ? 0 : 2)

B) The normalized form can be mapped back uniquely
   to monomial inequality, and further back to an
   inequality with the original factor set.

NOTES:  This extends to rational functions quite
simply ... all that happens is that denominators are
added to the "strict" set in all cases.
*/

// Returns TRUE, FALSE or UNDET and records strict/non-strict for each factor
int MonoIneqRep::processAtom(TAtomRef A) 
{
  if (A->relop == ALOP) return UNDET;
  int relop = (ICOMP(A->F->content,0) < 0) ? reverseRelop(A->relop) : A->relop;
  
  // constant LHS case
  if (A->F->content == 0)
    return relop == EQOP || relop == LEOP || relop == GEOP;
  if (A->factorsBegin() == A->factorsEnd())
    return relop == NEOP || relop == GTOP || relop == GEOP;
  
  // Record distinct factors and there strict/nonstrict category
  bool nonstrict = relopIsNonStrict(relop);
  for(map<IntPolyRef,int>::iterator itr = A->factorsBegin(); itr != A->factorsEnd(); ++itr)
  {
    map<IntPolyRef,int>::iterator mitr = RefToIndex.find(itr->first);
    if (mitr == RefToIndex.end()) { RefToIndex[itr->first] = nextIndex++; indexToType.push_back(1); } // indextoType is false if it is strict. Otherwise, true. 1 is true.
    int i = RefToIndex[itr->first];
    indexToType[i] = indexToType[i] && nonstrict;
  }

  int res = UNDET;
  return res;
}


int MonoIneqRep::processConjunction(TAndRef C)
{
  // First pass: find & categorize factors
  int res;
  for(TAndObj::conjunct_iterator itr = C->conjuncts.begin(); itr != C->conjuncts.end(); ++itr)
    if ((res = processAtom(*itr)) == FALSE) 
      return FALSE;

  // First pass complete, compile info:  When done,
  // n = number of factors, r = number of strict factors
  // RefToIndex assigns indices 1...r to strict factors and r+1...n to nonstrict
  // factors.
  int n = RefToIndex.size();
  reverseMap.push_back(RefToIndex.end());
  for(map<IntPolyRef,int>::iterator itr = RefToIndex.begin(); itr != RefToIndex.end(); ++itr)
    reverseMap.push_back(itr);
  int i = 1, j = n;
  while(1)
  {
    while(i <= n && indexToType[reverseMap[i]->second] == 0) ++i;
    while(j >= i && indexToType[reverseMap[j]->second] == 1) --j;
    if (i > j) break;
    swap(reverseMap[i],reverseMap[j]); 
    ++i;
    --j;
  }
  for(int i = 1; i <= n; ++i) reverseMap[i]->second = i;
  int r = j;

  // Second pass: represent each inequality as a vector pair
  for(TAndObj::conjunct_iterator itr = C->conjuncts.begin(); itr != C->conjuncts.end(); ++itr)
  {
    // Get atom and ignore constant atoms, which must be TRUE at this point
    TAtomRef A = asa<TAtomObj>(*itr); 
    if (A.is_null())
    {
      throw TarskiException("in MonomialInequality:processConjunction - unexpected non-atom!");
    }
    if (A->relop == ALOP) continue;
    int relop = (ICOMP(A->F->content,0) < 0) ? reverseRelop(A->relop) : A->relop;
    if (A->factorsBegin() == A->factorsEnd())
    {
      throw TarskiException("in MonomialInequality:processConjunction - unexpected constant atom!");
    }
    if (relop == NEOP) {
      for(map<IntPolyRef,int>::iterator fitr = A->factorsBegin(); fitr != A->factorsEnd(); ++fitr) {
        /*
        if (A->relop == 1 || A->relop == 4 || A->relop == 5)
          RefToSIneq[fitr->first].push_back(A); //Only store strong inequalities in RefToSIneq
        */
      }

      continue;
    }

    B.push_back(MIVectRep());
    MIVectRep &v = B.back();
    BtoC.push_back(A);
    v.strictPart.resize(r+1);
    v.nonstrictPart.resize(n - r);

    bool isNonStrict = false;
    int eqmul = relop == EQOP ? 2 : 1;


    for(map<IntPolyRef,int>::iterator fitr = A->factorsBegin(); fitr != A->factorsEnd(); ++fitr)
    {
      int index = RefToIndex[fitr->first];
      bool t = index > r; isNonStrict |= t;
      /*
      if (A->relop == 1 || A->relop == 4 || A->relop == 5)
        RefToSIneq[fitr->first].push_back(A); //Only store strong inequalities in RefToSIneq
      */
      if (!t) {
        v.strictPart.set(index,eqmul*fitr->second % 2);
      }
      else v.nonstrictPart.set(index-r-1,eqmul*fitr->second);
    }
    if (relop == EQOP) relop = LEOP;
    v.strictPart.set(0,((relop == LEOP || relop == LTOP) ? 1 : 0));
  }

  // if the conjunction only contains /='s, we need to insert the zero vector
  if (B.size() == 0)
  {
    B.push_back(MIVectRep());
    MIVectRep &v = B.back();
    v.strictPart.resize(r+1);
    v.nonstrictPart.resize(n - r);    
  }
  

  return UNDET; // NOTE:  Not sure what the meaning of this return value is!
}




void minWtBasis(MonoIneqRep &MIR, Matrix &M, vector<int> &piv, set<MIVectRep> &Bf);

extern int verbose;




/* The dreaded scoringFunction(map<IntPolyRef, vector <TAtomRef> > candidates, map<IntPolyRef, bool> weakFactors)
 * Input:
 * candidates, a mapping from each factor in the formula to all the atoms which can strengthen them
 * weakFactors, all the factors in need of strengthening
 *
 * Output:
 * strength, the vector<IntPolyRef> which contains all the factors necessary to strengthen all the members of weakFactors
 */
vector<TAtomRef> scoringFunction(map<IntPolyRef, vector<TAtomRef> > candidates, map<IntPolyRef, bool> weakFactors, TFormRef F)
{
  //Step 0: Make a array out of weakFactors
  vector<IntPolyRef> arrWeakFactors(weakFactors.size());
  int i = 0;
  for (map<IntPolyRef, bool>::iterator iter = weakFactors.begin(); iter != weakFactors.end(); ++iter){
    arrWeakFactors[i] = iter->first;
    i++;
    cout << endl;
  }
  //Step 1: figure out which members of candidates we actually need, put them in an  vector of TAtomRef
  //Step 1a: Make a mapping from weakFactors to the AtomRefs in goodCandidates
  //Step 1b: Penalize it if we've seen it the first time
  //Step 1c: +7 every time we see it
  vector < TAtomRef > goodCandidates;
  vector < int > scores;

  set < pair <TAtomRef, int> > goodCandFast;
  vector < vector<int> > factsToCand (weakFactors.size());
  vector < vector<int> > candToFacts;
  for  (unsigned i = 0; i < weakFactors.size(); i++){ //For every factor...
    //cerr << "first" << endl;
    bool done = false;
    vector<TAtomRef> toCheck = candidates[arrWeakFactors[i]];

    for (vector<TAtomRef>::iterator atomIt = toCheck.begin(); atomIt != toCheck.end(); ++atomIt) { //Check all its factors
      int ind = -1;
      bool exists = false;
      if (!goodCandidates.empty()) {
        for (vector<TAtomRef>::iterator iter2 = goodCandidates.begin(); iter2 != goodCandidates.end(); ++iter2) { //Check if a candidate already exists in the collection. This would be really improved with a good hashing algorithm!
          ind++;
          if (equals(*iter2, *atomIt) ){ //Meaning that it already appears in goodCandidates
            //cerr << "    "; b->write(); cerr << " already exists, so bringing score to " << scores[ind] + 7 << endl;
            factsToCand[i].push_back(ind); //Save the index so where we know where to find it when making our selections
            candToFacts[ind].push_back(i);
            scores[ind] += 7;
            exists = true;
            continue;
          }
        } 
      }
      if (!exists) {
        TAtomRef a = *atomIt;
        goodCandidates.push_back(*atomIt);
        scores.push_back(a->getVars().numElements()*-2 + 7);
        factsToCand[i].push_back(ind+1);
        vector <int> tmp (1, i);
        candToFacts.push_back(tmp);
      }
    }
  }

  

  //Step 2: Resize selected to the size of scores
  vector < bool > selected(scores.size());
  vector < TAtomRef > toReturn;
  //Step 3: Iterate through every element of arrWeakFactors
  //Step 3a: Check to make sure none of the potential inequalities have been chosen. If one has, skip this round (the inequality has already been strengthened)
  //Step 3b: Choose the highest scored inequality. Instert it into strength
  //Step 3c: For all other inequalities of the current member of arrWeakFactors, leverage a -7 penalty
  for (unsigned int i = 0; i < weakFactors.size(); i++) {
    bool alreadySelected = false;
    int highestIdx  = factsToCand[i][0];
    for (vector<int>::iterator iter = factsToCand[i].begin(); iter != factsToCand[i].end(); ++iter) {
      if (selected[*iter] == true) {
        alreadySelected = true;
      }
      scores[*iter] -= 7;
      if (!alreadySelected) {
        if (scores[highestIdx] < scores[*iter])
          highestIdx = *iter;
      }

    }

    if (!alreadySelected) {
      toReturn.push_back(goodCandidates[highestIdx]);
      for (vector<int>::iterator iter = candToFacts[highestIdx].begin(); iter != candToFacts[highestIdx].end(); ++iter) { //For all factors this candidate can strengthen
        for (vector<int>::iterator iter2 = factsToCand[*iter].begin(); iter2 != factsToCand[*iter].end(); ++iter2) {//Subtract the score since it's not necessary anymore
          scores[*iter2] -= 7;
        }
      }
      selected[highestIdx] = true;
    }
  }
  //Step 4: return strength.

  return toReturn;
}
/*******************************************************************
 * testmi(F)
 * Input: F, a conjunction or an atomic formula such that
 *        all atoms in F have gone through level-1 normalization, i.e.
 *        there are no "constant sigma 0" atoms, and no atoms where 
 *        the LHS factors are multiplied by a constant (other than 1).
 * Output: ???
 *******************************************************************/
TFormRef testmi(TFormRef F)
{
  const bool testwrite = verbose;

  // Process the formula
  if (asa<TConstObj>(F)) return F;
  TAndRef C = asa<TAndObj>(F);
  if (C.is_null()) {
    if (asa<TAtomObj>(F)) 
    { C = new TAndObj(); C->AND(F); }
    else { return F; }
  }
  if (C->size() == 0) return new TConstObj(TRUE);
  PolyManager &PM = *(F->getPolyManagerPtr());
  MonoIneqRep MIR;
  MIR.processConjunction(C);
  
  if (testwrite) {
    cout << "Vector Rep:" << endl;
    cout << "sigma";
    int r = MIR.numStrict();
    int m = MIR.numVars();
    for(int i = 1; i <= m; i++)
    {
      if (i == r + 1) { cout << " :"; }
      cout << " (";
      MIR.reverseMap[i]->first->write(*(F->getPolyManagerPtr()));
      cout << ")";
    }
    cout << endl;
    
    for(unsigned int i = 0; i < MIR.B.size(); ++i)
      MIR.B[i].write(); }


  
  // Pull out the strict part
  int r = MIR.numStrict();
  int m = 0; for(unsigned int i = 0; i < MIR.B.size(); ++i) if (MIR.B[i].nonstrictPart.isZero()) ++m;
  //  if (r == 0) return; // No strict part!
  Matrix M(m,1+r);
  vector<int> traceRow;
  for(unsigned int i = 0, j = 0; i < MIR.B.size(); ++i) {
    if (MIR.B[i].nonstrictPart.isZero()) {
      M[j++] = MIR.B[i].strictPart;
      traceRow.push_back(i);
    }
  }
  if (testwrite) { cout << "Strict Part Rep:" << endl; write(M); }

  //CHANGES
  // Gaussian Elimination!
  vector<int> piv;
  vector<int> piv2;
  vector<DBV> * george = identityMatrix(&M);
  //  gaussianElimination(M,piv,true);
  if (m > 0) reducedGaussExplain(M,*george,piv,piv2,true);

  //CHANGES
  if (testwrite) {
    cout << "After gauss:" << endl;
    cout << "M: " << endl; write(M);
    cout << "George: " << endl; write(*george);
  }

  //CHANGES
  // Detect inconsistency
  bool unsat = false;
  vector<int> reasons;
  unsat = detectUnsat(M, *george, reasons, testwrite);
  delete george;
  // Get result formula
  TFormRef res;
  if (unsat){
    res = new TConstObj(FALSE);
  }

  else
    {
      res = new TAndObj();
      TAndRef resa = res;
      bool first = true;
      // Get strict part
      vector<bool> tmp(m+1); for(int i = 1; i <= m; ++i) tmp[i] = false;
      for(int i = 0; i < m; ++i)
        {
          if (M[i].isZero()) continue;
          //      if (first) { first = false; } else { cout << " /\\ "; } 
          FactRef currF = new FactObj(PM);
          int c = 0;
          for(int j = 1; j <= r; ++j)
            if (M[i][j])
              {
                ++c;
                tmp[j] = true;
                currF->addFactor(MIR.reverseMap[j]->first,1);
              }
          resa->AND(makeAtom(currF,M[i][0] ? LTOP : GTOP));
        }
      // catch any /= cases
      bool ne = true; for(int i = 1; i <= r; ++i) ne = ne && tmp[i];
      if (!ne)
        {
          FactRef currF = new FactObj(PM);
          for(int i = 1; i <= r; ++i)
            if (!tmp[i])
              currF->addFactor(MIR.reverseMap[i]->first,1);
          resa->AND(makeAtom(currF,NEOP));
        }
      
      
      // Get reduced non-strict part
      set<MIVectRep> Bf;
      minWtBasis(MIR,M,piv,Bf);
      for(set<MIVectRep>::iterator i = Bf.begin(); i != Bf.end(); ++i) 
        {
          // If the strict part has only zeros in the polynomial positions and the non-strict
          // part has only even entries, we have an equality or TRUE.
          bool alleven = (i->strictPart.weight(true) == 0);
          FactRef currF = new FactObj(PM);
          for(int j = 1; j <= MIR.numVars(); (alleven = alleven && (i->get(j) != 3)) , ++j)
            if (i->get(j) != 0) 
              currF->addFactor(MIR.reverseMap[j]->first, (i->get(j) == 2 ? 2 : 1));
      
          int op = (i->get(0) != 0 ? LEOP : GEOP);
          if (!alleven) resa->AND(makeAtom(currF,op));
          else if (op == LEOP)
            { 
              // Set all multiplicities to 1 and 
              for(map<IntPolyRef,int>::iterator itr = currF->MultiplicityMap.begin(); itr != currF->MultiplicityMap.end(); ++itr)
                itr->second = 1;
              resa->AND(makeAtom(currF,EQOP)); 
            }
        }
      if (resa->size() == 0) res = new TConstObj(TRUE); else res = resa;
    }
  return res;
}




// Minimize non-strict part
//
// Let M be the res of guass elim on the strict ineqs
// Let Bi be the non-strict ineqs
//
// reduce non-strict part of each elt of Bi by M, call B
//
// 0: set Bf = { }
//
// 1: if B empty, return Bf
//    else chose w, a max wt element of B & remove w from B
//    let Ble be set of elts of B with support in s(w)
//    let Blt be set of elts of B with support in s(w) & wt < wt(w)
//
// 2) check whether Blt implies an equality, if yes, goto 1
//
// 3) Form matrix Mle over GF(2) whose rows are Ble mod 2.
//    Do Gauss on Mle & put into echelon form
//    let wp = result of reducing w by Mle
//    
//    if Mle contains [10...0] or wp = [10...0] 
//    then add equation with support of w to Bf, and remove from
//    B all elements with support equal to support of w
//
//    else if wp != [0...0] add w to Bf
//
// 4) goto 1

// Translates a split-representation to a mod2 image
DBV trans(const MIVectRep &w, int ns, int nn)
{
  DBV wp(ns + nn + 1);
  for(int i = 0; i <= ns; ++i)
    wp.set(i,w.strictPart.get(i));
  for(int i = 0; i < nn; ++i)
    wp.set(ns + 1 + i,w.nonstrictPart.get(i) % 2);
  return wp;
}

// computes weight defined as # non-zero entries in nonstrict part
int wt(const MIVectRep &w) 
{
  int i = 0;
  for(int j = 0; j < w.nonstrictPart.size()/2; ++j)
    if (w.nonstrictPart.get(j) != 0) ++i;
  return i;
}

// sc(a,b): 0 if s(a) = s(c), 1 if s(a) strictly contained in s(b), -1 otherwise
int sc(const MIVectRep &a, const MIVectRep &b)
{
  bool amissing = false, bmissing = false;
  for(int j = 0; j < a.nonstrictPart.size()/2 && !bmissing; ++j)
  {
    amissing |= (a.nonstrictPart.get(j) == 0 && b.nonstrictPart.get(j) != 0);
    bmissing |= (a.nonstrictPart.get(j) != 0 && b.nonstrictPart.get(j) == 0);
  }
  if (bmissing) return -1;
  if (!amissing) return 0;  
  return 1; //i.e. if (amissing) return 1;
}


/* Input:
   MIR - the original monomial inequality
   M & piv - the reduced matrix and pivot permutation for the strict part of MIR

   Output:
   Bf - a set of non-strict vectors which is equivalent to the non-strict part
        of MIR, and which has minimum wieght amongst all such.
 */
void minWtBasis(MonoIneqRep &MIR, Matrix &M, vector<int> &piv, set<MIVectRep> &Bf)
{
  int ns = MIR.numStrict();
  int nn = MIR.numVars() - ns;

  // Get Bi
  set<MIVectRep> Bi;
  for(unsigned int i = 0; i < MIR.B.size(); ++i) 
  {
    if (MIR.B[i].isStrict()) continue;
    
    // Reduce by strict part, and see if we've seen this before
    MIVectRep w = MIR.B[i];
    w.strictPart = reduce(M,piv,MIR.B[i].strictPart);
    if (Bi.find(w) != Bi.end()) continue;
    Bi.insert(w);
  }

  while(Bi.size() > 0)
  { 
    // Choose w
    int mw = -1; set<MIVectRep>::iterator mitr;
    for(set<MIVectRep>::iterator itr = Bi.begin(); itr != Bi.end(); ++itr)
    {
      int t = wt(*itr);
      if (t > mw) { mw = t; mitr = itr; }      
    }
    if (mw < 1) break;
    MIVectRep w = *mitr;
    Bi.erase(mitr);
    
    // Construct Blt and Beq
    vector<set<MIVectRep>::iterator> Blt, Beq; 
    for(set<MIVectRep>::iterator itr = Bi.begin(); itr != Bi.end(); ++itr)
    {
      int t = sc(*itr,w);
      if (t == 1) {/* add to Blt */ Blt.push_back(itr); }
      else if (t == 0) { /* add to Beq */ Beq.push_back(itr); }
    }

    // Check whether Blt implies an equality
    Matrix Mlt(Blt.size(),1+ns+nn);
    int i = 0;
    for(vector<set<MIVectRep>::iterator>::iterator itr = Blt.begin(); itr != Blt.end(); ++itr)
      Mlt[i++] = trans(**itr,ns,nn);
    // Gaussian Elimination!
    vector<int> pivlt;
    gaussianElimination(Mlt,pivlt,true);
     
    // Detect whether eq is implied
    bool implieseq = false;
    for(unsigned int i = 0; i < Blt.size() && !implieseq; ++i)
      implieseq = Mlt[i].get(0) && Mlt[i].weight(true) == 0;
    if (implieseq) continue;

    // Construct Mle
    int nr = Blt.size() + Beq.size(), j = 0;
    Matrix Mle(nr,1+ns+nn);
    for(vector<set<MIVectRep>::iterator>::iterator itr = Blt.begin(); itr != Blt.end(); ++itr)
      Mle[j++] = trans(**itr,ns,nn);
    for(vector<set<MIVectRep>::iterator>::iterator itr = Beq.begin(); itr != Beq.end(); ++itr)
      Mle[j++] = trans(**itr,ns,nn);
        
    // Gaussian Elimination!
    vector<int> pivle;
    reducedGauss(Mle,pivle,true);

    // Detect whether eq is implied
    implieseq = false;
    bool iszero = false;
    for(int i = 0; i < nr && !implieseq; ++i)
      implieseq = Mle[i].get(0) && Mle[i].weight(true) == 0;
    if (!implieseq)
    {
      DBV wp = trans(w,ns,nn);
      wp = reduce(Mle,pivle,wp);
      implieseq = wp.get(0) && wp.weight(true) == 0;     
      iszero = wp.isZero();
    }
    if (implieseq)
    {
      vector<MIVectRep> w2r;
      for(vector<set<MIVectRep>::iterator>::iterator itr = Beq.begin(); itr != Beq.end(); ++itr)
	w2r.push_back(**itr);
      for(unsigned int i = 0; i < w2r.size(); ++i)
	Bi.erase(w2r[i]);
      w.strictPart.set(0,1);
      for(int i = 1; i <= ns; ++i)
	w.strictPart.set(i,0);
      for(int i = 0; i < nn; ++i)
	w.nonstrictPart.set(i,(w.nonstrictPart.get(i) == 0 ? 0 : 2));
      Bf.insert(w);
      continue;
    }
    
    if (!iszero) Bf.insert(w);
  }      
}

}//end namespace tarski
