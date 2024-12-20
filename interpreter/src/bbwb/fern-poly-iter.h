#ifndef _FERN_POLY_ITER_H
#define _FERN_POLY_ITER_H
#include "../../poly/gcword.h"
#include "../../GC_System/GC_System.h"
#include "../../formula/formula.h"
#include <stack>
#include <vector>

namespace tarski {
  class FernPolyIter;
  class IntPolyObj;
  class PolyManager;
  class VarSet;
  template<class T> class VarKeyedMap;
  typedef GC_Hand<IntPolyObj> IntPolyRef;

  class FernPolyIter
  {
  private:
    int size; //-- The number of variables in the original polynomial
    int currDepth;
    GCWord A;
    bool null;
    vector<short> aSigns;
    vector<short> exponents;  //The exponents on every variable in the current stack
    vector<Variable> currVars; //the variables which appear in the current monomial
    vector<char> varLevels; //the level of each variable - so we know whether or not to pop the current variable when we go back up the stack
    vector<Variable> allVars; //All variables in the polynomial
    stack<short> sSigns; //The sign of the term for the current monomial is the top of this
    short finSign; //The sign on the monomial taking into account the constant factor
    stack<GCWord> mono; //Let's us know what we need to hit next
    GCWord coefficient; //The coefficient. Whoohoo!
    
    Word begin();
    Word dive();
    void init(Word A, const VarSet &S, const VarKeyedMap<int> &varSign);
    static string shortToRelop[8];
    
  public:

    //Overload postfix ++
    FernPolyIter operator++(int);
    FernPolyIter& operator++();
    bool operator <(const FernPolyIter& F);
    int compareTo(const FernPolyIter &F);
    FernPolyIter(Word A, const VarSet &S, const VarKeyedMap<int> &varSign);
    FernPolyIter(IntPolyRef ref, const VarKeyedMap<int> &varSign);
    //FernPolyIter(const FernPolyIter& F);
    FernPolyIter() {}
    Word next();
    bool isNull() const;
    int end() { return 0; }
    int numVars() const;
    Word getCoefficient() const;
    const vector<Variable>& getVars() const;
    const vector<Variable>& getAllVars() const;
    vector<short> getExponents() const;
    short getTermSign() const;
    short getMonoSign() const;
    void write();
    void write(PolyManager& PM);
    void writeAll(PolyManager& PM);
    static string numToRelop(short num);
    static int compare(FernPolyIter F1, FernPolyIter F2);
    void debug(PolyManager& PM)
    {
      cout << "allVars.size() = " << allVars.size() << "  aSigns.size() = " << aSigns.size() << endl;
      for(int i = 0; i < allVars.size(); ++i)
	cout << PM.getName(allVars[i]) << " = " << shortToRelop[aSigns[i]] << endl;
    }
  };

}//end namespace tarski
#endif


