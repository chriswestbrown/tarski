class UnsatCore
{
private:

  class FactorInfo
  {
  public:
    int r;
    GCWord Q; // the saclib poly rep of this factor
    int exp; // the exponent associated with this factor
    int i, j; // ProjFac (i,j)
    int tag; // temporary assignment of relop denoting known sign info
    FactorInfo(int r, Word Q, int exp, int i, int j) : r(r), Q(Q), exp(exp), i(i), j(j) { }
    void WRITEFULL(Word V);
    void clearTag() { tag = 7 /* ALOP */; }
  };
  
  class OrigAtom
  {
  public:
    GCWord P; // original un-normalized polynomial
    int relop, r;
    int sign;
    GCWord content;
    std::vector<FactorInfo> factors;
    OrigAtom(Word r, Word P, Word sigma) : P(P), relop(sigma), r(r) { }
    void WRITE(Word V);
    void clearTags();
    bool isKnownFalse();
  };

  std::vector<OrigAtom> conjuncts;

  // appearsIn[(i,j)] is a vector of all indices (a,b) s.t. conjunct[a].factors[b].Q is proj. fac. (i,j)
  std::map<std::pair<int,int>, std::vector< std::pair<int,int> > > appearsIn;

  GCWord MHSP, HITSETSOL;
  
  QepcadCls* Qptr;

public:
 UnsatCore(QepcadCls &Q) : Qptr(&Q), MHSP(NIL), HITSETSOL(0) { } 

  /*======================================================================
    returns TRUE if the input formula in Qptr is one for which UnsatCore
    applies, and FALSE otherwise.  UnsatCore requires a closed formula
    containing only E and F quantifiers, and no ExtendedTarskiAtoms.
    ======================================================================*/
  Word isApplicable();
  
  /*======================================================================
    ======================================================================*/
  void prepareForLift();

  /*======================================================================
    C is a cell that by trial evaluation is found to be false.
    ======================================================================*/
  void record(Word C);

  /*======================================================================
    ======================================================================*/
  void findUnsatCore(bool indexOnly = false);
};


