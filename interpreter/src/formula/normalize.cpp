#include "normalize.h"
#include <vector>
#include <set>
#include <queue>
#include "../poly/dedTableDecl.h"

using namespace std;

namespace tarski {

  extern int verbose;

  /************************************************************************
   * LEVEL 1 normalization implementation
   * 
   * Level 1 normalization normalizes a single atom A in isolation.
   *
   * Input: A is the atom to normalize, C is an empty conjunction that
   *        is "filled in" with the normalized version of A.
   *
   * Return value: false if A is inconsistent, true otherwise
   * Side effects: if A is not found to be inconsistent, A is filled in
   *               with atoms such that A <==> C, and C is the "normalized"
   *               version of A.
   ************************************************************************/
  // Returns false iff A is found to be inconsistent.  In that case ignore C.
  // If A is not found to be inconsistent, C is the conjunction obtained by
  // "Level 1" normalization: removing content, eliminating exponents from 
  // = and !=, normalizing all exponenents to 1 and 2 in >= and <=, and 
  // splitting > and < into even and odd parts - odds all go to one, evens
  // all go to one as well, but in != atoms.  Atoms like p1 * ... * pk /= 0
  // are broken up into p1 /= 0 /\ ... /\ pk /= 0.
  bool level1_atom(TAtomRef A, TAndRef C)
  {
    int relop = (ICOMP(A->F->content,0) < 0) ? reverseRelop(A->relop) : A->relop;

    // constant LHS case
    if (A->F->content == 0)
      return relop == EQOP || relop == LEOP || relop == GEOP;
    if (A->factorsBegin() == A->factorsEnd())
      return relop == NEOP || relop == GTOP || relop == GEOP;
  
    // non-constant LHS case
    switch(relop)
      {
      case EQOP: {
        TAtomRef newA = new TAtomObj(*(A->F->M)); newA->relop = relop;
        for(map<IntPolyRef,int>::iterator itr = A->factorsBegin(); itr != A->factorsEnd(); ++itr)
          newA->F->addFactor(itr->first,1);
        C->conjuncts.insert(newA);
      } break;
      case NEOP: {
        for(map<IntPolyRef,int>::iterator itr = A->factorsBegin(); itr != A->factorsEnd(); ++itr)
          {
            TAtomRef newA = new TAtomObj(*(A->F->M)); newA->relop = relop; 
            newA->F->addFactor(itr->first,1);
            C->conjuncts.insert(newA);
          }
      } break;
      case LEOP: case GEOP: {
        bool oddFactor = false;
        TAtomRef newA = new TAtomObj(*(A->F->M)); newA->relop = relop;
        for(map<IntPolyRef,int>::iterator itr = A->factorsBegin(); itr != A->factorsEnd(); ++itr)
          {
            newA->F->addFactor(itr->first,(itr->second % 2 == 0) ? 2 : 1);
            oddFactor = oddFactor || (itr->second % 2 == 1);
          }
        if (oddFactor) C->conjuncts.insert(newA);    
        else if (relop == LEOP) { 
          newA->relop = EQOP;
          return level1_atom(newA,C); }
      } break;
      case LTOP: case GTOP: {
        TAtomRef newA = new TAtomObj(*(A->F->M)); newA->relop = relop;
        for(map<IntPolyRef,int>::iterator itr = A->factorsBegin(); itr != A->factorsEnd(); ++itr)
          if (itr->second % 2 == 0)
            {
              TAtomRef newA = new TAtomObj(*(A->F->M)); 
              newA->relop = NEOP; 
              newA->F->addFactor(itr->first,1);
              C->conjuncts.insert(newA);
            }
          else
            newA->F->addFactor(itr->first,1);
        if (newA->factorsBegin() != newA->factorsEnd())
          C->conjuncts.insert(newA);    
        else 
          return relop == NEOP || relop == GTOP || relop == GEOP;
      } break;
      }
    return true;
  }

  TFormRef level1_and(TAndRef Cinit, TAtomRef* ptr = NULL)
  {
    TAndRef Cfinal = new TAndObj;
    for(set<TFormRef>::iterator itr = Cinit->conjuncts.begin(); itr != Cinit->conjuncts.end(); ++itr)
      if (level1_atom(*itr,Cfinal) == false)
      {
	if (ptr != NULL) { (*ptr) = *itr; }
        return new TConstObj(FALSE);
      }

    if (0)
      { // stats gathering ... temporary stuff!
        cout << "AFTER L1: ";
        Cfinal->write();
        cout << endl << endl;
      }


    return Cfinal;
  }

  TFormRef level1(TFormRef F, TAtomRef* ptr)
  {
    TAndRef A = asa<TAndObj>(F); 
    if (A.is_null())
      {
        TAtomRef T = asa<TAtomObj>(F); 
        if (!T.is_null()) { A = new TAndObj; A->conjuncts.insert(T); }
      }
    if (!A.is_null()) return level1_and(A,ptr);
    else return F;
  }


  /*********************************************************************
   ** LEVEL 2 - assume LEVEL 1 already!
   *********************************************************************/
  /*
    We have conjunction C of atoms.  The irreducible factors
    in C are p_1,p_2,...,p_r, and we have sigma_1,...,sigma_r s.t.
    we know from having examined the atoms in C that
    C ==> p_i sigma_i 0, for each i.

    Now we wish to process and simplify a multifactor atom A,
    A = p_j1^e1 * ... * p_js^es sigma' 0.  Thus, for each
    factor p_jk^ek we may have to act.  Our actions are
    "do nothing", "simplify A to TRUE", "simplify A to FALSE",
    "remove factor p_jk^ek", "remove factor p_jk^ek and swap sigma' -
    like multiplying by -1", "Allows Strengthening" - [where 
    strengthening is changing >= to > or <= to <. note that all
    factors must allow strenthening inorder to make the change.]
    Below we define the array ACTIONS so
    that ACTIONS[sigma'][ek][sigma_jk] tells us what to do.
  
    Now, of course, if we remove enough factors, A may have a
    constant LHS, in which case A will be thrown out (if true)
    or C will be replaced with FALSE (otherwise).  If only one
    factor p_jk^ek remains, we update sigma_jk and have one less
    multifactor atom.

  */
#define I 0
#define T 1
#define F 2
#define R 3
#define S 4
#define N 5
#define G 6
  const int IMPOSSIBLE  = I;
  const int REP_W_TRUE  = T;
  const int REP_W_FALSE = F;
  const int REMOVE_FACT = R;
  const int REM_W_SWAP  = S;
  const int DO_NOTHING  = N;
  const int ALLOWS_STR  = G;
  const char ACTIONS[][8][8] = { /*
                                   mult. one         mult. two
                                   ___________       __________
                                   L E L G N G *     L E L G N G *
                                   T Q E T E E *     T Q E T E E * */
    /*     */{{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}},
    /* LTOP*/{{0,0,0,0,0,0,0,0},{I,S,F,I,R,N,I,N},{I,I,I,I,I,I,I,N}},
    /* EQOP*/{{0,0,0,0,0,0,0,0},{I,R,T,N,R,R,N,N},{I,I,I,I,I,I,I,N}},
    /* LEOP*/{{0,0,0,0,0,0,0,0},{0,S,T,N,R,G,N,N},{0,R,T,N,R,R,N,N}},
    /* GTOP*/{{0,0,0,0,0,0,0,0},{I,S,F,I,R,N,I,N},{I,I,I,I,I,I,I,N}},
    /* NEOP*/{{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,N},{0,0,0,0,0,0,0,N}},
    /* GEOP*/{{0,0,0,0,0,0,0,0},{0,S,T,N,R,G,N,N},{0,R,T,N,R,R,N,N}}
  };
#undef I
#undef T
#undef F
#undef R
#undef S
#undef N
#undef G

  // This class collects information based on pure "level 2" reasoning.
  // In particular, If we have "f relop 0" from a NormPoly object, that
  // means that the formula structure essentially states "f relopo 0",
  // even though that may be distributed over several atoms.  For example,
  // "f >= 0 and f g /= 0" essentially states "f > 0".  Higher "simplification
  // level" reasoning may deduce something stronger.  However, assuming
  // what gets deduced is still consistent, it's not clear that we always
  // want to make the stronger statement explicit in the simplified formula.
  class NormPoly
  {
  public:
    IntPolyRef f; // The polynomial itself
    int relop;    // what we know, i.e. we know that "f relop 0" is true
    set<int> mfaIndices; // set of indices of multi-factor atoms that have f as a factor

    // Level 3 and 4 fields
    bool implied, mayProvideSignInfo; // Both involved in deducing signs based on sign info for variables
    int l3dedRelop; // The relop deduced from level 3
    int l4dedRelop; // The relop deduced from level 4

    NormPoly() : implied(false), mayProvideSignInfo(false), l3dedRelop(ALOP), l4dedRelop(ALOP) { }
  };

  class NormMFAtom : public vector< pair<int,int> >
  {
  public:
    int relop;
    bool in_queue;
  };


  // Return false iff an inconsistency is found: whatWeKnow[p] = (relop,index)
  bool addInfoTo(map<IntPolyRef, pair<int,int> > &whatWeKnow, TAtomRef A, IntPolyRef p, int &nextIndex)
  {
    // Define what we know of p based on A
    int info;
    if (A->F->numFactors() == 1)
      info = A->relop;
    else if ((A->relop & EQOP) == 0)
      info = NEOP;
    else
      info = ALOP;
 
    // Add new info to whatWeKnow
    map<IntPolyRef, pair<int,int> >::iterator itrK = whatWeKnow.find(p);
    if (itrK == whatWeKnow.end()) 
      { 
        whatWeKnow.insert(pair<IntPolyRef, pair<int,int> >(p,pair<int,int>(info,nextIndex++)));
        itrK = whatWeKnow.find(p);
      }
    itrK->second.first &= info;
    return bool(itrK->second.first);
  }

  TConstRef levelThreePart(vector<NormPoly> &P, VarKeyedMap<int> &varSign, PolyManager* ptrPM);
  TConstRef levelFourPart(vector<NormPoly> &P, VarKeyedMap<int> &varSign, PolyManager* ptrPM, 
                          TAndRef Cinit, int l4flags);

  TFormRef level2_and(TAndRef Cinit, int l3flags = 0, int l4flags = 0)
  {
    PolyManager* ptrPM = Cinit->getPolyManagerPtr();

    //>>>TEST
    VarSet appearing = Cinit->getVars();
    for(VarSet::iterator itr = appearing.begin(); itr != appearing.end(); ++itr)
      Cinit->AND(makeAtom(*ptrPM,new IntPolyObj(*itr),ALOP));
    //<<<

    //------ Figure out what we know about each polynomial factor, -------
    //------ return False formula if we find an inconsistency      -------
    bool f = true;
    int N = 0; // Eventually set to # of distinct factors appearing in Cinit (also used in indexing!)
    map<IntPolyRef, pair<int,int> > whatWeKnow;
    set<TAtomRef> multiFactorAtoms;
    for(set<TFormRef>::iterator itr = Cinit->conjuncts.begin(); f && itr != Cinit->conjuncts.end(); ++itr)
      {
        TAtomRef A = asa<TAtomObj>(*itr); if (A.is_null()) { cerr << "Non-atom in conjunct!" << endl; exit(1); }
        for(map<IntPolyRef,int>::iterator itrF = A->factorsBegin(); f && itrF != A->factorsEnd(); ++itrF)
          f = addInfoTo(whatWeKnow,A,itrF->first,N);
        if (A->F->numFactors() > 1)
          multiFactorAtoms.insert(A);
      }
    if (!f) { return new TConstObj(FALSE); }

    // Set up P, the vector of factors
    vector<NormPoly> P(N);
    for(map<IntPolyRef, pair<int,int> >::iterator itr = whatWeKnow.begin(); itr != whatWeKnow.end(); ++itr)
      {
        P[itr->second.second].f = itr->first;
        P[itr->second.second].relop = itr->second.first;
      }

    //---------- LEVEL 3 SECTION -------------------------------
    // which tries to deduce signs of factors by what it knows of the signs of vars
    VarKeyedMap<int> varSign(ALOP);
    if (l3flags) 
      { 
        TConstRef res = levelThreePart(P,varSign,ptrPM); 
        if (!res.is_null()) 
          {
            if (verbose) { cout << "Level 3 normalization found conflict in "; Cinit->write(); cout << endl; }
            return res;
          }
      }
    if (l3flags & nf_ded_strengthenings) { for(unsigned int i = 0; i < P.size(); ++i) P[i].relop &= P[i].l3dedRelop; }

    //------------- LEVEL 4 SECTION --------------------------------
    // Level 4 simplification tries to determine if some polynomial sign
    // condition is in fact implied by another + what we know about signs of variables.
    // We must be careful about linear univariate polynomials, however, since they
    // might be the basis for a decision about the sign of a variable! 
    if (l3flags && l4flags)
      {
        TConstRef res = levelFourPart(P,varSign,ptrPM,Cinit,l4flags); 
        if (!res.is_null()) return res;
      }
    if (l4flags & nf_ded_strengthenings) { for(unsigned int i = 0; i < P.size(); ++i) P[i].relop &= P[i].l4dedRelop; }

    //------- Simplfify the multiFactorAtoms if possible ----------------
    // Set up A, the vector of multiFactorAtoms, and enqueue indices of all multi-factor atoms
    queue<int> Q;
    vector<NormMFAtom> A(multiFactorAtoms.size());
    int M = 0; // Eventually set to the size of A, also used for assigning indices
    for(set<TAtomRef>::iterator itr = multiFactorAtoms.begin(); itr != multiFactorAtoms.end(); ++itr)
      {
        Q.push(M);    
        A[M].in_queue = true;
        A[M].relop = (*itr)->relop;
        for(map<IntPolyRef,int>::iterator itrF = (*itr)->factorsBegin(); f && itrF != (*itr)->factorsEnd(); ++itrF)
          {
            int indexInP = whatWeKnow[itrF->first].second;
            A[M].push_back(pair<int,int>(indexInP,itrF->second));
            P[indexInP].mfaIndices.insert(M);      
          }
        ++M;
      }

    // Simplification loop! Note A[i].relop == ALOP means it's been simplified away to TRUE
    while (!Q.empty())
      {
        // Dequeue i and process multi-factor atom A[i]
        int i = Q.front(); Q.pop(); A[i].in_queue = false;
        int nf = A[i].size();
        int strengthencount = 0;
        for(int j = 0; j < nf; ++j)
          {
            int k = A[i][j].first;
            int factorRelop = P[k].relop & P[k].l3dedRelop & P[k].l4dedRelop; // Strongest relop we know!
            switch(ACTIONS [A[i].relop] [A[i][j].second] [factorRelop])
              {
              case REP_W_TRUE:  
                { for(int k = 0; k < nf; ++k) P[A[i][k].first].mfaIndices.erase(i); nf = 0; A[i].relop = NEOP;  }break;
              case REP_W_FALSE: 
                nf = 0; A[i].relop = EQOP; break;
              case REMOVE_FACT: 
                P[A[i][j].first].mfaIndices.erase(i); --nf; swap(A[i][j],A[i][nf]); --j; break;
              case REM_W_SWAP:  
                P[A[i][j].first].mfaIndices.erase(i); 
                --nf; swap(A[i][j],A[i][nf]); --j; A[i].relop = reverseRelop(A[i].relop); break;
              case ALLOWS_STR: ++strengthencount; break;
              case DO_NOTHING: break;
              case IMPOSSIBLE: 
                cerr << "P[" << k << "] = "; P[k].f->write(*ptrPM); cerr << endl;
                cerr << P[k].relop <<  P[k].l3dedRelop  <<  P[k].l4dedRelop << endl;
                cerr << A[i].relop << ' ' << A[i][j].second <<  ' ' << factorRelop << " : ";
                cerr << "Norm2 impossible case reached!" << endl; exit(1); break;
              }
          }
        // Strenghten <= to < or >= to > if allowed.
        if (nf > 0 && strengthencount == nf) { 
          if (A[i].relop == GEOP) A[i].relop = GTOP; 
          else if (A[i].relop == LEOP) A[i].relop = LTOP; } 

        // If A[i] after processing consists of 0 or 1 atom, deal with it!
        if (nf == 0 && (A[i].relop & GTOP))
          A[i].relop = (EQOP|NEOP);
        else if (nf == 0)
          return new TConstObj(FALSE);
        else if (nf == 1)
          {
            int k = A[i][0].first, relop = A[i].relop;
            P[k].mfaIndices.erase(i);
            if (A[i][0].second == 2) {
              if (relop == GEOP) relop = (EQOP|NEOP);
              else relop = EQOP;
            }
            if (P[k].relop != (P[k].relop & relop))
              {
                P[k].relop &= relop;
                for(set<int>::iterator itr = P[k].mfaIndices.begin(); itr != P[k].mfaIndices.end(); ++itr)
                  if (!A[*itr].in_queue) { Q.push(*itr); A[*itr].in_queue = true; }
              }
          }
        A[i].resize(nf);	
      }

    //------- Reconstruct formula------------------------------

    // Add all the single factor atoms.
    TAndRef Res = new TAndObj;
    for(unsigned int i = 0; i < P.size(); ++i)
      {    
        bool implicit = false;
        if (P[i].relop == NEOP) 
          {
            for(set<int>::iterator itr = P[i].mfaIndices.begin(); !implicit && itr != P[i].mfaIndices.end(); ++itr)
              implicit = ((A[*itr].relop & EQOP) == 0);
          }
        if (P[i].relop != ALOP && !implicit && !((l3flags & nf_ded_implications) && P[i].implied))
          Res->AND(makeAtom(*ptrPM,P[i].f,P[i].relop));
      }

    // Add all the multi-factor atoms.
    for(unsigned int i = 0; i < A.size(); ++i)
      {
        if (A[i].relop == (EQOP|NEOP)) continue;
        TAtomRef newA = new TAtomObj(*ptrPM);
        newA->relop = A[i].relop;
        for(unsigned int j = 0; j < A[i].size(); ++j)
          newA->F->addFactor(P[A[i][j].first].f,A[i][j].second);
        Res->AND(newA);
      }

    // Deal with the possiblity that we end up with one atom
    if (Res->size() == 0)
      return new TConstObj(TRUE);
    else if (Res->size() == 1) 
      return *(Res->conjuncts.begin());
    else
      return Res;
  }

  TFormRef level2(TFormRef F)
  {
    TAndRef C = asa<TAndObj>(F);
    if (C.is_null()) return F;
    TFormRef Fp = level2_and(C);
    return Fp;
  }

  /*********************************************************************
   ** LEVEL 3
   *********************************************************************/
  TFormRef level3(TFormRef F, int l3flags, int l4flags)
  {
    TAndRef C = asa<TAndObj>(F);
    if (C.is_null()) return F;
    if (C->size() == 0) return new TConstObj(TRUE);
    TFormRef Fp = level2_and(C,l3flags,l4flags);
    return Fp;
  }

  // Fills varSign (see below), annotates elements of P setting "implied"
  // and "mayProvideSignInfo" fields.  In current setup, may tighten P[i].relop.
  // Not sure if this is good!
  TConstRef levelThreePart(vector<NormPoly> &P, VarKeyedMap<int> &varSign, PolyManager* ptrPM)
  {

    // Fill varSign with what we know about the signs of variables.  We only deduce
    // info about signs of variables by considering linear, univariate polynomials!
    int N = P.size();
    VarKeyedMap<int> indexOfSignProvider(-1);
    for(int i = 0; i < N; ++i)
      {
        if (P[i].f->numVars() == 1 && P[i].f->degree(P[i].f->getVars()) == 1)
          {
            Variable x = P[i].f->getVars();
            int relop = P[i].relop, deducedRelop = ALOP, oldRelop = varSign[x];
            int s1 = ptrPM->ldcf(P[i].f,x)->signIfConstant();
            int s0 = ptrPM->coef(P[i].f,x,0)->signIfConstant();
            if (s1 < 0) { s0 *= -1; relop = reverseRelop(relop); }
            if      (s0 < 0 && (relop == EQOP || relop == GTOP || relop == GEOP)) deducedRelop = GTOP;
            else if (s0 == 0) deducedRelop = relop;
            else if (s0 > 0 && (relop == LTOP || relop == EQOP || relop == LEOP)) deducedRelop = LTOP;

            int refinedDeducedRelop = oldRelop & deducedRelop;
            if (!refinedDeducedRelop) { return new TConstObj(FALSE); }

            if (deducedRelop == ALOP || strictlyStronger(oldRelop,deducedRelop)) continue;

            if (refinedDeducedRelop == deducedRelop && deducedRelop != oldRelop) 
              { varSign[x] = deducedRelop; P[i].mayProvideSignInfo = true; indexOfSignProvider[x] = i; }
            else if (oldRelop == deducedRelop) // Figure out which is stronger!
              {  
                int j = indexOfSignProvider[x];
                if (P[i].f->linearRootFurtherFromZero(P[j].f))
                  {
                    P[j].mayProvideSignInfo = false;
                    P[j].implied = true;
                    indexOfSignProvider[x] = i;
                    varSign[x] = deducedRelop; 
                    P[i].mayProvideSignInfo = true; 
                  }
                else
                  {
                    P[i].implied = true;
                  }
              }
            else
              {
                cerr << "I didn't think this could ever happen!" << endl;
                exit(1);
              }
          }
      }
    
    // See if we can deduce a little extra!
    for(int i = 0; i < N; ++i)
      {
        if (P[i].f->isVariable() != 0) continue;
        int ded = P[i].f->signDeduce(varSign);

        if (ded != ALOP && verbose) 
          { 
            SWRITE("LEVEL3 Deduced: "); P[i].f->write(*ptrPM); SWRITE(" "); SWRITE(relopString(ded).c_str()); SWRITE(" 0 using ");
            VarSet tmp; P[i].f->signDeduceMinimalAssumptions(varSign,tmp);
            for(VarSet::iterator itr = tmp.begin(); itr != tmp.end(); ++itr)
              {	SWRITE(ptrPM->getName(*itr).c_str()); SWRITE(" "); }
            SWRITE("\n");
          }

        int refined = (P[i].relop & ded);
        P[i].l3dedRelop = ded;
    
        if (refined == 0) { return new TConstObj(FALSE); }

        // If P[i] does not provide sign-info on any variable, and if we can
        // deduce P[i].relop from our variable sign-info, "P[i] P[i].relop 0"
        // can be dropped from the result.
        if (ded == refined && refined != ALOP && !P[i].mayProvideSignInfo) { P[i].implied = true; }          
      }  

    return NULL;
  }

  /*********************************************************************
   ** LEVEL 4
   *********************************************************************/
  class ImpGraphNode;
  class ImpGraph
  {
  public:
    ImpGraph(vector<NormPoly> &P);
    void addEdge(int j, int i, int relop); // Edge from j to i with weight relop
    void findAndRecordImplications(); // Sets P[i].implied = true when it can deduce without decisions
  private:
    int N;
    vector<ImpGraphNode> G;
    vector<NormPoly> *ptrP;
  };

  TConstRef levelFourPart(vector<NormPoly> &P, VarKeyedMap<int> &varSign, PolyManager* ptrPM, 
                          TAndRef Cinit, int l4flags)
  {
    int N = P.size();
    static int dn = 0;

    ImpGraph G(P);

    for(int i = 0; i < N; ++i)
      {

        for(int j = 0; j < N; ++j)
          {
            //	cout << "dn == " << dn << endl;
            //	++dn;
            // if (P[i].mayProvideSignInfo) continue;
            //      if (P[i].implied) continue; // at this point the only "implied" are implied by variable sign info.
            if (i == j) continue;
	
            // If we don't have any info about P[j]'s sign, it can't help, nor can we make the deduction
            // below if all we have is p[j] /= 0.  In either case, the only deduction comes when b = 0,
            // which is the same as deducing the sign of P[i] without P[j].
            if (P[j].relop == ALOP || P[j].relop == NEOP) continue;

            // Find an x s.t. P[i] + x*P[j] >= 0.
            // if x*P[j] <= 0, then P[j]'s sign condition implies p >= 0
            int c_strict1,c_equal1, sL1, sR1;
            int s1 = specialLinCombQ(P[i].f,P[j].f,varSign,GTOP,sL1,sR1,c_strict1,c_equal1,ptrPM);

            // cout << "\n\n\n";
            // cout << "P[i] = "; P[i].f->write(*ptrPM); cout << " " << relopString(P[i].relop) << endl;
            // cout << "P[j] = "; P[j].f->write(*ptrPM); cout << " " << relopString(P[j].relop) << endl;
            // cout << "c_strict1 = " << c_strict1 << endl;
            // cout << "c_equal1 = " << c_equal1 << endl;
            // cout << "sL1 = " << sL1 << endl;
            // cout << "sR1 = " << sR1 << endl;
            // cout << "\n\n\n";


            if (s1 != FALSE  && !(sL1 == 0 && sR1 == 0))
              {
                int ded = dedTableGE[P[j].relop][sL1+1][sR1+1][c_strict1][c_equal1];

                //<<< Test Section
                if (ded != ALOP)
                  {
                    bool interesting = true;
                    if (P[i].f->isVariable() != 0)
                      {
                        VarSet x = P[i].f->isVariable();
                        int known = varSign[x];
                        if (strictlyStronger(known,ded)) interesting = false;
                        else if (known == ded)
                          {
                            VarKeyedMap<int> varSignp(varSign);
                            varSignp[x] = ALOP;
                            int c_strictT,c_equalT, sLT, sRT;
                            int sT = specialLinCombQ(P[i].f,P[j].f,varSignp,GTOP,sLT,sRT,c_strictT,c_equalT,ptrPM);	  
                            if (sT == FALSE || ded != dedTableGE[P[j].relop][sLT+1][sRT+1][c_strictT][c_equalT])
                              interesting = false;
                          }
                      }
                    if (!interesting && verbose) cout << "Stupid deduction! ";
      
                    int sLast = s1;
                    if (verbose) {
                      cout << "Ded 1: " << "P := "; P[i].f->write(*ptrPM); cout << " Q := "; 
                      P[j].f->write(*ptrPM); cout << " " << flush; }
                    VarSet usedM = P[i].f->svars + P[j].f->svars, usedF;
                    VarKeyedMap<int> varSignp(varSign);
                    for(VarSet::iterator itr = usedM.begin(); itr != usedM.end(); ++itr)
                      {
                        int tmp = varSignp[*itr];
                        varSignp[*itr] = ALOP;
                        int c_strictT,c_equalT, sLT, sRT;
                        int sT = specialLinCombQ(P[i].f,P[j].f,varSignp,GTOP,sLT,sRT,c_strictT,c_equalT,ptrPM);
	
                        if (sT == FALSE || ded != dedTableGE[P[j].relop][sLT+1][sRT+1][c_strictT][c_equalT])
                          { 
                            varSignp[*itr] = tmp; 
                            usedF = usedF + *itr; 
                          }	
                        else
                          sLast = sT;
                      }
      
                    if (verbose) {
                      SWRITE("Used: ");
                      for(VarSet::iterator itr = usedF.begin(); itr != usedF.end(); ++itr)
                        {	SWRITE(ptrPM->getName(*itr).c_str()); SWRITE(" "); }
                      OWRITE(sLast);
                      SWRITE("\n");
                    }
                  }
                //>>>

                if (ded != ALOP /*&& sL*sR == 1*/) { /* if [sL,sR] contains zero, it's actually a level 3 deduction! NOT ALWAYS */
                  if (verbose) {
                    cout << "Proved:1 that ";  P[j].f->write(*ptrPM); cout << " " << relopString(P[j].relop) << " 0 implies ";
                    P[i].f->write(*ptrPM); cout << " " << relopString(ded) <<  " 0 in ";
                    Cinit->write();
                    cout << ": sL1 = " << sL1 << " sR1 = " << sR1 << " .implied = " 
                         << P[i].implied << " relop = " << relopString(P[i].l3dedRelop);
                    cout << endl; }

                  P[i].l4dedRelop &= ded;
                  if ((P[i].relop & P[i].l4dedRelop) == 0) { 
                    if (verbose) { cout << "This is a conflict!!!!" << endl; }
                    return new TConstObj(FALSE);  }
                  G.addEdge(j,i,ded);
                }
              }
	

            // Find an x s.t. P[i] + x*P[j] <= 0.
            // if x*P[j] >= 0, then P[j]'s sign condition implies p <= 0
            int c_strict2,c_equal2, sL2, sR2;
            int s2 = specialLinCombQ(P[i].f,P[j].f,varSign,LTOP,sL2,sR2,c_strict2,c_equal2,ptrPM);
            if (s2 != FALSE  && !(sL2 == 0 && sR2 == 0))
              {
                int ded = dedTableLE[P[j].relop][sL2+1][sR2+1][c_strict2][c_equal2];


                //<<< Test Section
                if (ded == ALOP) continue;

                bool interesting = true;
                if (P[i].f->isVariable() != 0)
                  {
                    VarSet x = P[i].f->isVariable();
                    int known = varSign[x];
                    if (strictlyStronger(known,ded)) interesting = false;
                    else if (known == ded)
                      {
                        VarKeyedMap<int> varSignp(varSign);
                        varSignp[x] = ALOP;
                        int c_strictT,c_equalT, sLT, sRT;
                        int sT = specialLinCombQ(P[i].f,P[j].f,varSignp,LTOP,sLT,sRT,c_strictT,c_equalT,ptrPM);	  
                        if (sT == FALSE || ded != dedTableLE[P[j].relop][sLT+1][sRT+1][c_strictT][c_equalT])
                          interesting = false;
                      }
                  }
                if (!interesting && verbose) cout << "Stupid deduction! ";

                int sLast = s2;
                if (verbose) {
                  cout << "Ded 2: " << "P := "; P[i].f->write(*ptrPM); 
                  cout << " Q := "; P[j].f->write(*ptrPM); cout << " " << flush; }
                VarSet usedM = P[i].f->svars + P[j].f->svars, usedF;
                VarKeyedMap<int> varSignp(varSign);
                for(VarSet::iterator itr = usedM.begin(); itr != usedM.end(); ++itr)
                  {
                    int tmp = varSignp[*itr];
                    varSignp[*itr] = ALOP;
                    int c_strictT,c_equalT, sLT, sRT;
                    int sT = specialLinCombQ(P[i].f,P[j].f,varSignp,LTOP,sLT,sRT,c_strictT,c_equalT,ptrPM);

                    if (sT == FALSE || ded != dedTableLE[P[j].relop][sLT+1][sRT+1][c_strictT][c_equalT])
                      { 
                        varSignp[*itr] = tmp; 
                        usedF = usedF + *itr; 
                      }	
                    else
                      sLast = sT;
                  }
                if (verbose) {
                  SWRITE("Used: ");
                  for(VarSet::iterator itr = usedF.begin(); itr != usedF.end(); ++itr)
                    {	SWRITE(ptrPM->getName(*itr).c_str()); SWRITE(" "); }
                  OWRITE(sLast);
                  SWRITE("\n");
                }
                //>>>



                if (ded != ALOP /*&& sL*sR == 1*/) {/* if [sL,sR] contains zero, it's actually a level 3 deduction! NOT ALWAYS */
                  if (verbose) {
                    cout << "Proved:2 that ";  P[j].f->write(*ptrPM); cout << " " << relopString(P[j].relop) << " 0 implies ";
                    P[i].f->write(*ptrPM); cout << " " << relopString(ded) <<  " 0 in ";
                    Cinit->write();
                    cout << ": sL2 = " << sL2 << " sR2 = " << sR2 << " .implied = " << P[i].implied 
                         << " relop = " << relopString(P[i].l3dedRelop);
                    cout << endl; }
                  P[i].l4dedRelop &= ded;
                  if ((P[i].relop & P[i].l4dedRelop) == 0) { 
                    if (verbose) { cout << "This is a conflict!!!!" << endl; }
                    return new TConstObj(FALSE); }
                  G.addEdge(j,i,ded);
                }
              }
          }
      }
 
    if (l4flags & nf_ded_implications)
      G.findAndRecordImplications();

    return NULL;
  }



  /************************************************************************
   * Set implied info
   ************************************************************************/
  const int missing=0, waiting=1, found=2;
  class ImpGraphNode
  {
  public:
    int index; // i.e. the index in P
    int relop; // i.e. the P[i].relop value
    int mark;

    void notifyEdgeTo(int i, int relop);
    void notifyEdgeFrom(int j, int relop);
    int andOfIncomingRelops(bool onlyMarkedFoundFlag, vector<ImpGraphNode> &G);
    void enqueueMissingOutNbrs(queue<int> &Q, vector<ImpGraphNode> &G);

  private:
    // first is index, second is relop
    vector< pair<int,int> > IncomingEdges;
    vector< pair<int,int> > OutgoingEdges;
  };




  void ImpGraphNode::notifyEdgeTo(int i, int relop)
  {
    unsigned int k = 0;
    while(k < OutgoingEdges.size() && OutgoingEdges[k].first != i) ++k;
    if (k < OutgoingEdges.size())
      OutgoingEdges[k].second &= relop;
    else
      OutgoingEdges.push_back(pair<int,int>(i,relop));
  }

  void ImpGraphNode::notifyEdgeFrom(int j, int relop)
  {
    unsigned int k = 0;
    while(k < IncomingEdges.size() && IncomingEdges[k].first != j) ++k;
    if (k < IncomingEdges.size())
      IncomingEdges[k].second &= relop;
    else
      IncomingEdges.push_back(pair<int,int>(j,relop));
  }

  int ImpGraphNode::andOfIncomingRelops(bool onlyMarkedFoundFlag, vector<ImpGraphNode> &G)
  {
    int relop = ALOP;
    for(unsigned int k = 0; k < IncomingEdges.size(); ++k)
      if (!onlyMarkedFoundFlag || G[IncomingEdges[k].first].mark == found)
        relop &= IncomingEdges[k].second;
    return relop;
  }

  void ImpGraphNode::enqueueMissingOutNbrs(queue<int> &Q, vector<ImpGraphNode> &G)
  {
    for(unsigned int k = 0; k < OutgoingEdges.size(); ++k)
      if (G[OutgoingEdges[k].first].mark == missing)
        {
          Q.push(OutgoingEdges[k].first);
          G[OutgoingEdges[k].first].mark = waiting;
        }
  }



  ImpGraph::ImpGraph(vector<NormPoly> &P) : N(P.size()), G(N), ptrP(&P)
  {
    for(int i = 0; i < N; ++i)
      {
        G[i].index = i;
        G[i].relop = P[i].relop;
        G[i].mark = missing;
      }
  }

  void ImpGraph::addEdge(int j, int i, int relop)
  {
    G[i].notifyEdgeFrom(j,relop);
    G[j].notifyEdgeTo(i,relop);
  }

  void ImpGraph::findAndRecordImplications()
  {
    //  cout << "ImpGraph::N = " << N << endl;

    vector<NormPoly> &P = *ptrP;

    // Initialize Q, waitQ
    queue<int> waitQ; // Queue of vertices waiting to be found
    for(int i = 0; i < N; ++i)
      {
        if (P[i].implied || P[i].mayProvideSignInfo || strictlyStronger(G[i].relop,G[i].andOfIncomingRelops(false,G)))
          G[i].mark = found;
        else
          { waitQ.push(i); G[i].mark = waiting; }
      }

    // Construct S_Q and for each G[i] in S_Q - Q, set P[i].implied = true
    while(!waitQ.empty())
      {
        int next = waitQ.front(); waitQ.pop();
        int deducedRelop = G[next].andOfIncomingRelops(true,G);
        if (sameOrStronger(deducedRelop,G[next].relop))
          {
            P[next].implied = true;
            G[next].mark = found;
            G[next].enqueueMissingOutNbrs(waitQ,G);
          }
        else
          G[next].mark = missing;
      }

    return;
  }
}//end namespace tarski
