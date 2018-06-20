#include "../../formula/monomialinequality.h"
#include "../../formula/DynamicBitVector.h"
#include "minwtbasis.h"
#include "deduction.h"
#include <vector>
#include <list>
#include <unordered_set>
#include <iostream>

#define u_s unordered_set
namespace tarski {
  using namespace std;

  inline void merge(u_s<int>& s1, const u_s<int>& s2) {
    for (u_s<int>::const_iterator itr = s2.begin(), end = s2.end(); itr != end; ++itr) {
      s1.insert(*itr);
    }

  }


  

  /* Matrix M, a reduced Matrix
     piv, the pivot columns vector
     traceMatrix, indicating which rows reduced eachother
     traceRow, a mapping from the rows in M to the rows in the MIR used to generate it
     (say, in the case you are only using the strict part of the matrix)

     Output
     v - the row to reduce
     explain - contains the rows in the MonoIneqRep used to generate M used to reduce v
    
  */
  DBV reduceExplain(Matrix &M, vector<int> &piv, vector<DBV>& traceMatrix, vector<int>& traceRow, const DBV &v, u_s<int>& explain) {
    DBV u;
    u = v;
    for(unsigned int i = 0; i < piv.size(); ++i){
      if (u.get(piv[i]) == true) {
        u += M[i];
        cerr << "i is " << i << endl;
        for (int idx = 0; idx < traceMatrix[i].size(); idx++) {
          if (traceMatrix[i].get(idx) != 0)
            explain.insert(idx);
        }
      }
    }

    return u;
  }

  DBV reduceExplain2(Matrix &M, vector<int> &piv, vector<DBV>& traceMatrix, vector<u_s<int> >& trace, const DBV &v, u_s<int>& explain) {
    DBV u;
    u = v;
    for(unsigned int i = 0; i < piv.size(); ++i){
      if (u.get(piv[i]) == true) {
        u += M[i];
        cerr << "i2 is " << i << endl;
        for (int idx = 0; idx < traceMatrix[i].size(); idx++) {
          if (traceMatrix[i].get(idx) != 0)
            merge(explain, trace[idx]);
        }
      }
    }
    return u;
  }




  //This method exists to see if we can deduce EQOP on a particular polynomial
  //If sigma is 1:
  //return -1 if there is no unique 2 value (except sigma)
  //Otherwise, returns the index of the unique 2 value
  //If sigma is 0:
  //return -1, since we can't deduce anything useful
  bool getDed(MIVectRep& mi, MonoIneqRep * MIR, TAtomRef& t, PolyManager * PM) {
    // If the strict part has only zeros in the polynomial positions and the non-strict
    // part has only even entries, we have an equality or TRUE.
    bool alleven = (mi.strictPart.weight(true) == 0);
    FactRef currF = new FactObj(PM);
    for(int j = 1; j <= MIR->numVars(); (alleven = alleven && (mi.get(j) != 3)) , ++j)
      if (mi.get(j) != 0)
        currF->addFactor(MIR->reverseMap[j]->first, (mi.get(j) == 2 ? 2 : 1));

    int op = (mi.get(0) != 0 ? LEOP : GEOP);
    if (!alleven) {
      t = new TAtomObj(currF, op);
      t->write();
      return true;
    }
    else if (op == LEOP) {
      t = new TAtomObj(currF, EQOP);
      t->write();
      return true;
      }
    return false;
  }


  vector<Deduction *> minWtBasisDeds(MonoIneqRep &MIR, vector<short> pureStrict) {
    vector<MIVectRep> Bf;
    vector<vector<TAtomRef> > finExp;
    vector<Deduction *> deds;
    minWtBasisExp(MIR, Bf, finExp);
    vector<vector<TAtomRef> >::iterator vItr = finExp.begin();
    for (vector<MIVectRep>::iterator itr = Bf.begin(), end = Bf.end(); itr != end; ++itr, ++vItr) {
      MIVectRep mi = *itr;
      TAtomRef t;
      vector<TAtomRef> exp = *vItr;
      if (exp.size() == 0) { cerr << "Terminate due to size 0" << endl; continue; }
      PolyManager * PM = exp[0]->getPolyManagerPtr();
      bool success = getDed(mi, &MIR, t,  PM);
      if (success)  {
        if (exp.size() != 0)
          deds.push_back(new MnWtDed(t, exp));
      }

    }

    return deds;
  }





  /* Input:
     MIR - the original monomial inequality
     M & piv - the reduced matrix and pivot permutation for the strict part of MIR

     Output:
     Bf - a vector of non-strict vectors which is equivalent to the non-strict part
     of MIR, and which has minimum weight amongst all such.
     finExp - An explanation for each member included in Bf as a vector of TAtomRef
  */
  void minWtBasisExp(MonoIneqRep &MIR,  vector<MIVectRep> &Bf, vector<vector<TAtomRef> >& finExp) {
    int ns = MIR.numStrict();
    int nn = MIR.numVars() - ns;

    vector<int> traceRow;
    Matrix M = MIR.genStrictMatrix(traceRow);
    std::vector<int> piv;
    std::vector<int> pivRow;
    std::vector<DBV>  george = idMatrix(M);
    reducedGaussExplain(M,george,piv,pivRow,true);

    // Get Bi
    list<MIVectRep> Bi;//List instead of vector because erases are cheaper
    list<u_s<int> > BiExp;
    for(unsigned int i = 0; i < MIR.B.size(); ++i) 
      {
        if (MIR.B[i].isStrict()) continue;
        cerr << "In loop " << i << "\n";
        // Reduce by strict part, and see if we've seen this before
        MIVectRep w = MIR.B[i];
        u_s<int> exp;
        exp.insert(i);
        /*
          Looks like we'll need a reduceExplain function here!
          Will need to tag all w's with the a minimal set of vectors used to reduce it (does not need to be optimal)
        */
        w.strictPart = reduceExplain(M,piv,george,traceRow,MIR.B[i].strictPart, exp);
        cout << "DEPS: ";
        for (u_s<int>::iterator itr = exp.begin(), end = exp.end(); itr != end; ++itr) {
          cout << *itr << " ";
        }
        cout << endl;
        if (isIn(w, Bi)) continue;
        Bi.push_back(w);
        BiExp.push_back(exp);
      
      }

    while(Bi.size() > 0)
      { 
        // Choose w
        int currInd = 0; 
        list<u_s<int> >::iterator biExpItr = BiExp.begin(), bestItr; //lets us track for BiExp;
        
        int mw = -1; list<MIVectRep>::iterator mitr;
        for(list<MIVectRep>::iterator itr = Bi.begin(); itr != Bi.end(); ++itr, ++biExpItr)
          {
            int t = wt(*itr);
            if (t > mw) { mw = t; mitr = itr; bestItr = biExpItr;}
            currInd++;
          }
        if (mw < 1) break;
        MIVectRep w = *mitr;
        u_s<int> exp = *bestItr;

        Bi.erase(mitr);
        BiExp.erase(bestItr);
        // Construct Blt and Beq
        vector<u_s<int>  > bltExp, beqExp; //See construction of Mle below
        vector<list<MIVectRep>::iterator> Blt, Beq;
        biExpItr = BiExp.begin();
        for(list<MIVectRep>::iterator itr = Bi.begin(); itr != Bi.end(); ++itr, ++biExpItr)
          {
            int t = sc(*itr,w);
            if (t == 1) {/* add to Blt */ Blt.push_back(itr); bltExp.push_back(*biExpItr); }
            else if (t == 0) { /* add to Beq */ Beq.push_back(itr); beqExp.push_back(*biExpItr); }

          }

        // Check whether Blt implies an equality

        //Step 6------------------------------------------------------------------------------------
        Matrix Mlt(Blt.size(),1+ns+nn);
        int i = 0;
        for(vector<list<MIVectRep>::iterator>::iterator itr = Blt.begin(); itr != Blt.end(); ++itr)
          Mlt[i++] = trans(**itr,ns,nn);
        // Gaussian Elimination!
        vector<int> pivlt;
        gaussianElimination(Mlt,pivlt,true);
     
        // Detect whether eq is implied
        bool implieseq = false;
        for(unsigned int i = 0; i < Blt.size() && !implieseq; ++i)
          implieseq = Mlt[i].get(0) && Mlt[i].weight(true) == 0;
        if (implieseq) continue;
        //End Step 6--------------------------------------------------------------------------------
        //Note no modifications because this is just an early terminate case
      
        // Construct Mle
        bltExp.insert(bltExp.end(), beqExp.begin(), beqExp.end()); //Now bltExp is the explanation for Mle
        int nr = Blt.size() + Beq.size(), j = 0;
        Matrix Mle(nr,1+ns+nn);
        for(vector<list<MIVectRep>::iterator>::iterator itr = Blt.begin(); itr != Blt.end(); ++itr)
          Mle[j++] = trans(**itr,ns,nn);
        for(vector<list<MIVectRep>::iterator>::iterator itr = Beq.begin(); itr != Beq.end(); ++itr)
          Mle[j++] = trans(**itr,ns,nn);
        
        // Gaussian Elimination!
        vector<int> pivle;
        vector<int> pivrow;
        vector<DBV> trace = idMatrix(Mle);
        reducedGaussExplain(Mle, trace, pivle, pivrow, true);
        u_s<int> addToExp;
        //So trace its the george for this matrix
        //So trace[i][j] means that row i required row j for the reduction
        //And row j requires bltExp[j]


        // Detect whether eq is implied
        implieseq = false;
        bool iszero = false;
        for(int i = 0; i < nr; ++i) {
          implieseq = Mle[i].get(0) && Mle[i].weight(true) == 0;
          if (implieseq) {
            for (int j = 0; j < nr; ++j) {
              if (trace[i][j] == 1) merge(addToExp, bltExp[j]);
            }
            break;
          }
        }
        if (!implieseq)
          {
            DBV wp = trans(w,ns,nn);
            wp = reduceExplain2(Mle,pivle,trace,bltExp,wp, addToExp);
            implieseq = wp.get(0) && wp.weight(true) == 0;
            iszero = wp.isZero();
          }
        if (implieseq)
          {
            vector<MIVectRep> w2r;
            for(vector<list<MIVectRep>::iterator>::iterator itr = Beq.begin(); itr != Beq.end(); ++itr)
              w2r.push_back(**itr);
            for(unsigned int i = 0; i < w2r.size(); ++i)
              eraseFromList(w2r[i], Bi);
            w.strictPart.set(0,1);
            for(int i = 1; i <= ns; ++i)
              w.strictPart.set(i,0);
            for(int i = 0; i < nn; ++i)
              w.nonstrictPart.set(i,(w.nonstrictPart.get(i) == 0 ? 0 : 2));
            Bf.push_back(w); //Something makes it to the final solution here!
            merge(exp, addToExp);
            addExp(exp, finExp, MIR);
            continue;
          } 
    
        if (!iszero) {
          Bf.push_back(w); //Something makes it to the final solution here!
          merge(exp, addToExp);
          addExp(exp, finExp, MIR);
        }
      }
  }

  bool isIn(MIVectRep mi, list<MIVectRep> mList) {
    for (list<MIVectRep>::iterator itr = mList.begin(), end = mList.end(); itr != end; ++itr) {
      if (mi == *itr) return true;
    }
    return false;
  }

  inline void eraseFromList(MIVectRep mi,  list<MIVectRep>& mList) {
    for (list<MIVectRep>::iterator itr = mList.begin(), end = mList.end(); itr != end; ++itr) {
      if (mi == *itr) {
        mList.erase(itr);
        return;
      }
    }
  }

  inline void addExp(u_s<int>& exp, vector<vector<TAtomRef > >& finExp, MonoIneqRep& MIR) {
    vector<TAtomRef> curr;
    for (u_s<int>::iterator itr = exp.begin(), end = exp.end(); itr != end; ++itr)
      curr.push_back(MIR.rowIndexToAtom(*itr));
    finExp.push_back(curr);

  }

}


