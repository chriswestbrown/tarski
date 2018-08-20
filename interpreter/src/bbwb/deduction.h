#ifndef DEDUCTION_H
#define DEDUCTION_H

#include <set>
#include <unordered_set>
#include <utility>
#include <forward_list>
#include <list>
#include <queue>
#include <vector>
#include <algorithm>
#include <string>
#include <numeric>
#include "Solver.h"
#include "SolverTypes.h"
#include "../tarski.h"
#include "../poly/poly.h"
#include "../formrepconventions.h"
#include "fern-poly-iter.h"


//TODO: Change Deducitons so that they use forward_lists of deductions
//      rather than vectors
//TODO:Redesign deductions s.t.
//1. A deduction is just a string and an atom
//2. The explanation is a separate data structure
//3. Givens are identified by Deduction Manager rather
//than by deductions themselves
//4. The DedM deductions vector members are on the
//stack rather than on the heap
//TODO: Simplification, and the data structures which support it,
//should be moved to a friend class of dedM
namespace tarski {

  const int PSGN =  0;
  const int DEDSGN = 1;
  /*
    Using Rounds to determine UNSAT:
    Rounds store a list of dependencies and a VarSet
    VarSets are the explanation in terms of the original variable relops
    the dependencies are a list of integers indicating a deduction was made during another round
    By following the deductions, you can trace back to the explanation in terms of the original relops

    All Deduction classes assume that the first sign and the first polynomial contains the actual deduction.
    Everything else is the explanation!
  */

  class DedManager;

  class Deduction {
    friend class DedManager;
  private:

    const static string names[7];
    TAtomRef deduction; //Exists if this is a learned sign on some factref
    bool unsat; //true if this is just the general deduction that some atoms are incompatible
    const std::string * name;
    //A safe write method for the deduction itself
    //will just write UNSAT if the deduction pointer is null
    void writeActual() {
      if (unsat) cout << "UNSAT\n";
      else {
        deduction->write();
        cout << endl;
      }
    }

  public:
    const static int GIVEN;
    const static int COMBO;
    const static int BBSTR;
    const static int MINWT;
    const static int BBCOM;
    const static int POLYS;
    const static int DEDUC;
    void write() const;
    inline const std::string& getName() { return *name; }
    inline bool isUnsat() const { return unsat; }
    inline TAtomRef getDed() const { return deduction; }
    virtual ~Deduction() {}
    Deduction(TAtomRef t, short code) : unsat(false) {
      name = &names[code];
      deduction = t;
    }
    Deduction(short code) : unsat(true) {
      name = &names[code];
    }
    Deduction(const Deduction& d) : unsat(d.unsat), name(d.name) {
      deduction = d.deduction;
    }
    Deduction() {}
  };



  struct DedExp {
    Deduction d;
    forward_list<TAtomRef> exp;
    DedExp(const Deduction& D, const forward_list<TAtomRef>& EXP)
      : d(D), exp(EXP) {};
    DedExp(TAtomRef t, int code, const forward_list<TAtomRef>& EXP)
      : d(t, code), exp(EXP) {}
    DedExp(int code, const forward_list<TAtomRef>& EXP)
      : d(code), exp(EXP) {}
    DedExp() {};
  };

  struct Result {
  public:
    std::vector<TAtomRef> atoms;
    /*
      Returns #of polys, vars and atoms for this result
    */
    inline int count() { return atoms.size(); }
    inline Result(const vector<TAtomRef>& a) : atoms(a) {}
    inline Result() : atoms(0) {}


    inline void write() {
      bool notFirst = false;
      cout << "size is " << atoms.size() << endl;
      for (std::vector<TAtomRef>::iterator itr = atoms.begin(); itr != atoms.end(); ++itr) {
        if (notFirst) std::cout << " /\\ "; 
        else notFirst = true;
        (*itr)->write();
      }
    }

  };

  

  class DedManager{

  public:
    typedef forward_list<TAtomRef> dedList;
  private:
    
    struct ManagerComp {
      bool operator()(const TAtomRef &A, const TAtomRef &B);
    };
    PolyManager * PM;
    bool unsat;
    std::vector<Deduction> deds; //The deductions themselves
    typedef
    std::vector<std::list<std::set<int> > > vecDep;
    vector<char> isGiven;
    vecDep depIdxs; //The indexes of all the atoms a deduction is dependent on
    std::vector<std::set<int> > origDep; //The indices of the first time a deduction was made. This is necessary because cycles can ruin tracebacks, but std::set doesn't give us a way to retrieve the first way a deduction was made
    VarKeyedMap<int> varSigns; //A fast mapping for variables, which is needed for WB algorithms
    //The index of the last deduction on an atom
    std::map<TAtomRef, int, ManagerComp> atomToDed;
    int givenSize; //The size of the part of the deds vector which is all given
    short getSgn(TAtomRef t);
    void writeDeps(Deduction&);
    void updateVarSigns(TAtomRef t);
    inline void updateVarSigns(const Deduction& d) { updateVarSigns(d.getDed()); }

    //CONSTRUCTOR METHODS
    void addGCombo(TAtomRef t);
    void processGiven(TAtomRef t);
    //END CONSTRUCTOR


    std::set<int> getDepIdxs(const dedList&);
    void addDed(const Deduction& d, const dedList&);
    void addCycle(const Deduction& d, const dedList&);
    void addCombo(const Deduction& d, const dedList&);
    void checkAdd(std::set<int>&, int idx);


    //SIMPLFICATION METHODS
    TAndRef simplify();
    int scoreDed(const Deduction& d);
    void writeIntermediate(vector<size_t>&, vector<size_t>&);
    template <typename T> vector<size_t> sort_indices(const vector<T>& v) {
      vector<size_t> idx(v.size());
      iota(idx.begin(),idx.end(), 0);
      sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) {
          return v[i1] < v[i2];
        });
      return idx;
    }
    typedef std::unique_ptr<Minisat::vec<Minisat::Lit>> vecPtr;
    typedef std::list<vecPtr> listVec;
    listVec genSatProblem(TAndRef& t, set<int>& skips, vector<size_t>&);
    void writeSatProblem(listVec& lv);
    void solveSAT(listVec& lv, TAndRef& t, set<int>& skips,
                  vector<size_t>& indices);
    //the indices of all the elements in the simplified formula
    vector<int> simpIdx;

    //END SIMPLIFICATION METHODS
  public:
    DedManager(const std::vector<TAtomRef>&);
    DedManager(TAndRef t);
    ~DedManager();

    TAndRef getInitConjunct();
    void addGiven(TAtomRef t);
    short getSign(IntPolyRef p);
    inline bool isUnsat() { return unsat; }
    bool processDeduction(const Deduction& d, const dedList&);
    inline const Deduction& getLast() { return deds.back(); }
    inline VarKeyedMap<int>& getVars() { return varSigns; }
    inline Result traceBack() { return traceBack(deds.size()-1); }
    Result traceBack(int idx);
    void writeProof(int i);
    inline void writeProof() { writeProof(deds.size()-1); }
    void writeAll();
    inline int size() {return deds.size();}
    typedef vector<Deduction>::const_iterator dedItr;
    inline void getItrs(int idx, dedItr& itr, dedItr& end)
    { itr = deds.begin()+idx; end = deds.end(); }
    inline void getOItrs(dedItr& itr, dedItr& end)
    { itr = deds.begin(); end = deds.begin()+givenSize;}
    int searchMap(TAtomRef A);
    bool isEquiv(TAtomRef A, TAtomRef B);
    TAndRef getSimplifiedFormula();
    Result explainAtom(TAtomRef t);
    //Explain the atoms required to produce an atom in the simplified formula
    //At idx i.
    //If there are no dependencies, the result consists of the atom itself
    inline Result explainSimp(size_t i) {
      return (!isGiven[i])
        ? traceBack(simpIdx[i]) : Result({deds[simpIdx[i]].getDed()});
    }
  };


}//end namespace
#endif
