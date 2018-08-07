#ifndef DEDUCTION_H
#define DEDUCTION_H

#include <set>
#include <utility>
#include <forward_list>
#include <list>
#include <queue>
#include <vector>
#include <algorithm>
#include <string>
#include "../tarski.h"
#include "../poly/poly.h"
#include "../formrepconventions.h"
#include "fern-poly-iter.h"


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
  protected:

    TAtomRef deduction; //Exists if this is a learned sign on some factref
    std::vector<TAtomRef> deps; //The atoms we needed to make a deduction
    std::vector<std::vector<int> > alt;
    //An alternate explanation for a deduction
    bool unsat; //true if this is just the general deduction that some atoms are incompatible
    bool given; //given or not
    std::string name;
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
    void write();
    inline bool isGiven() {return given;}
    inline std::string getName() { return name;}
    inline bool isUnsat() const { return unsat; }
    const std::vector<TAtomRef>& getDeps() const {return deps;}
    TAtomRef getDed() const { return deduction; }
    inline void addCycle(const vector<int>& v)
    { alt.emplace_back(v.begin(), v.end()); }
    virtual ~Deduction() {}
  Deduction(TAtomRef t, const std::vector<TAtomRef>& atoms, std::string s) : deps(atoms), unsat(false),  given(false), name(s)   {
      deduction = t;
    }
  Deduction(TAtomRef t, std::string s) : unsat(false), given(true), name(s) {
      deduction = t;
    }
  Deduction(const std::vector<TAtomRef>& atoms, std::string s) : deps(atoms), unsat(true), given(false), name(s) {}
    Deduction() {}
  };


  class Simplification : public Deduction {
  public:
  Simplification(TAtomRef t, const std::vector<TAtomRef>& atoms)
    : Deduction(t, atoms, "simplification") { } 
  };

  class BBDed : public Deduction {
  public:
  BBDed(TAtomRef t, const std::vector<TAtomRef>& atoms)
    : Deduction(t, atoms, "bb ded") { }
  BBDed(const std::vector<TAtomRef>& atoms) : Deduction(atoms, "bb unsat") { }
    
  };

  class MinWtDed : public Deduction {
  public:
  MinWtDed(TAtomRef t, const std::vector<TAtomRef>& atoms)
    : Deduction(t, atoms, "minwt ded") { }
  };

  class WBDed: public Deduction {

  public:
  WBDed(TAtomRef t, const std::vector<TAtomRef>& atoms, int code)
    : Deduction(t, atoms, "") {
      if (code == PSGN) name = "poly sign";
      else if (code == DEDSGN) name = "deduce sign";
      else throw TarskiException("UNKNOWN CODE FOR WBDED!");
    }

  };

  class Given : public Deduction {
  private:
  public:
    Given(TAtomRef t)
      : Deduction(t, "given") { }
  };

  class SignCombo : public Deduction {
  public:
    SignCombo(TAtomRef t, const std::vector<TAtomRef>& atoms)
      : Deduction(t, atoms, "sign combo") { }
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
  private:

    struct ManagerComp {
      bool operator()(const TAtomRef &A, const TAtomRef &B);
    };
    struct DedScore {
      int score;
      const Deduction * d;
      DedScore(Deduction * d) : score(0) {this->d = d; scoreDed();}
      void scoreDed();
    };
    struct SimpleComp {
      bool operator()(const DedScore& a, const DedScore& b);
    };

    PolyManager * PM;
    bool unsat;
    std::vector<Deduction *> deds; //The deductions themselves
    std::vector<std::vector<int> > depIdxs; //The indexes of all the atoms a deduction is dependent on
    VarKeyedMap<int> varSigns; //A fast mapping for variables, which is needed for WB algorithms
    //The index of the last deduction on an atom
    std::map<TAtomRef, int, ManagerComp> atomToDed;
    int givenSize; //The size of the part of the deds vector which is all given
    short getSgn(TAtomRef t);
    void writeDeps(Deduction *);
    void updateVarSigns(TAtomRef t);
    inline void updateVarSigns(Deduction * d) { updateVarSigns(d->getDed()); }

    //CONSTRUCTOR METHODS
    void addGCombo(TAtomRef t);
    void processGiven(TAtomRef t);
    //END CONSTRUCTOR
    vector<int> getDepIdxs(Deduction * d);
    void addDed(Deduction * d);
    void addCycle(Deduction * d);
    void addCombo(Deduction * d);


    TAndRef simplify();
    vector<int> expSimplify();

  public:

    void addGiven(TAtomRef t);
    short getSign(IntPolyRef p);
    inline bool isUnsat() { return unsat; }
    bool processDeduction(Deduction * d);
    short processDeductions(vector<Deduction *> v);
    inline Deduction * getLast() { return deds.back(); }
    inline VarKeyedMap<int>& getVars() { return varSigns; }
    inline Result traceBack() { return traceBack(deds.size()-1); }
    Result traceBack(int idx);
    void writeProof(int i);
    inline void writeProof() { writeProof(deds.size()-1); }
    void writeAll();
    inline int size() {return deds.size();}
    typedef vector<Deduction *>::const_iterator dedItr;
    inline void getItrs(int idx, dedItr& itr, dedItr& end)
    { itr = deds.begin()+idx; end = deds.end(); }
    DedManager(const std::vector<TAtomRef>&);
    DedManager(TAndRef t);
    ~DedManager();
    int searchMap(TAtomRef A);
    bool isEquiv(TAtomRef A, TAtomRef B);
    TAndRef getSimplifiedFormula();
  };


}//end namespace
#endif
