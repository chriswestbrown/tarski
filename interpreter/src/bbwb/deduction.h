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
    void write() {
      if (unsat) cout << "UNSAT";
      else {
        cout << name <<  ": ";
        if (!unsat) deduction->write();
      }
      if (given) cout << endl;
      else {
        std::cout << " from  [ ";
        for (unsigned int i = 0; i < deps.size(); i++) {
          if (deps[i]->getRelop() != ALOP) {
            deps[i]->write();
            if (i != deps.size()-1) std::cout << " /\\ ";
          }
        }
        cout << " ]\n";

      }
    }
  public:
    inline bool isGiven() {return given;}
    inline std::string getName() { return name;}
    inline bool isUnsat() const { return unsat; }
    const std::vector<TAtomRef>& getDeps() const {return deps;}
    TAtomRef getDed() const { return deduction; }
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

  class MnWtDed : public Deduction {
  public:
  MnWtDed(TAtomRef t, const std::vector<TAtomRef>& atoms)
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

  struct managerComp {
    bool operator()(const TAtomRef &A, const TAtomRef &B) {
      if (A->getFactors()->numFactors() < B->getFactors()->numFactors()) return true;
      if (A->getFactors()->numFactors() > B->getFactors()->numFactors()) return false;
      int t = OCOMP(A->getFactors()->getContent(),B->getFactors()->getContent());
      if (t != 0) {
	if (t < 0) return -1;
        return 1;
      }
      FactObj::factorIterator itrA= A->getFactors()->factorBegin();
      FactObj::factorIterator itrB= B->getFactors()->factorBegin();
      while(itrA != A->getFactors()->factorEnd())
        {
          if (itrA->second < itrB->second) return true;
          if (itrA->second > itrB->second) return false;
          if (itrA->first < itrB->first) return true;
          if (itrB->first < itrA->first) return false;
          ++itrA;
          ++itrB;
        }
      return false;
    }
  };

  class DedManager{
  private:

    PolyManager * PM;
    bool unsat;
    std::vector<Deduction *> deds; //The deductions themselves
    std::vector<std::vector<int> > depIdxs; //The indexes of all the atoms a deduction is dependent on
    VarKeyedMap<int> varSigns; //A fast mapping for variables, which is needed for WB algorithms
    //The index of the last deduction on an atom
    std::map<TAtomRef, int, managerComp> atomToDed; 
    inline short getSgn(TAtomRef t) {
      return (atomToDed.find(t) == atomToDed.end())
        ? ALOP : deds[atomToDed[t]]->getDed()->relop;
    }
    void writeDeps(Deduction *);

    inline void updateVarSigns(TAtomRef t) {
      if (t->F->numFactors() == 1 && t->factorsBegin()->first->isVariable().any() &&
          t->getRelop() != ALOP) {
        VarSet v = t->getVars();
        varSigns[v] = varSigns[v] & t->getRelop();
      }

    }
    void updateVarSigns(Deduction * d) {
      updateVarSigns(d->getDed());
    }

    //CONSTRUCTOR METHODS
    
    void addGCombo(TAtomRef t);
    void processGiven(TAtomRef t);
    //END CONSTRUCTOR
    vector<int> getDepIdxs(Deduction * d);
    void addDed(Deduction * d);
    void addCombo(Deduction * d);

    short processFirstBB(BBDed *, IntPolyRef, short);


  public:

    void addGiven(TAtomRef t);
    inline short getSign(IntPolyRef p) {
      FactRef F = new FactObj(PM);
      F->addFactor(p, 1);
      TAtomRef t = new TAtomObj(F, ALOP);
      return getSgn(t);
    }
    inline bool isUnsat() {return unsat;}
    bool processDeduction(Deduction * d);
    short processDeductions(vector<Deduction *> v);
    inline Deduction * getLast() {return deds.back();}
    VarKeyedMap<int>& getVars() {return varSigns;}
    Result traceBack();
    Result traceBack(int idx);
    void writeProof();
    void writeProof(int idx);
    void writeAll();
    inline int size() {return deds.size();}
    inline void getItrs(int idx, vector<Deduction *>::const_iterator& itr, vector<Deduction *>::const_iterator& end) {
      itr = deds.begin()+idx;
      end = deds.end();
    }
    DedManager(const std::vector<TAtomRef>&);
    DedManager(TAndRef t);
    ~DedManager();
    int searchMap(TAtomRef A) {
      int ret = -1;
      for (map<TAtomRef, int, managerComp>::iterator itr = atomToDed.begin(); itr != atomToDed.end(); ++itr) {
	if (isEquiv(A, itr->first)) {
	  ret = itr->second;
	  break;
	}
      }
      return ret;
    }
    bool isEquiv(TAtomRef A, TAtomRef B) {
      if (A->getFactors()->numFactors() != B->getFactors()->numFactors()) return false;
      int t = OCOMP(A->getFactors()->getContent(),B->getFactors()->getContent());
      if (t != 0) { return false; }
      FactObj::factorIterator itrA= A->getFactors()->factorBegin();
      FactObj::factorIterator itrB= B->getFactors()->factorBegin();
      while(itrA != A->getFactors()->factorEnd())
	{
	  if (itrA->second != itrB->second) return false;
	  if (itrA->first < itrB->first || itrB->first < itrA->first) return false;
	  ++itrA;
	  ++itrB;
	}
      return true;
    }
  };


}//end namespace
#endif
