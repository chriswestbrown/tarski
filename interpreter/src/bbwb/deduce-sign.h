#ifndef DEDUCE_SIGN_H
#define DEDUCE_SIGN_H

#include "../../GC_System/GC_System.h"
#include "../../poly/variable.h"
#include "../../poly/gcword.h"
#include <vector>
#include <utility>

namespace tarski {

  class FernPolyIter;
  class IntPolyObj;
  class PolyManager;
  class VarSet;
  typedef GC_Hand<IntPolyObj> IntPolyRef;

  const int REALS =         9994;
  const int RMINUS =        9995;
  const int RPLUS =         9996;
  const int INCLUSIVE =     9997;
  const int EXCLUSIVE =     9998;
  const int INFTY =         9999;

  const int  YEA =  true;
  const int  NAY =  false;



  class Interval
  {
  private:
    static bool T_valid[8][8];
    static short T_interval[2][8][8];
    static short T_restrict[8];
    static short T_weak[2][8][8];
    Word left;
    short leftType;
    Word right;
    short rightType;
    short sign;

    static std::vector<Interval> equalsCase(FernPolyIter& F1, FernPolyIter& F2);
    static std::vector<Interval> qOnlyCase(FernPolyIter& F);
    static std::vector<Interval> pOnlyCase(FernPolyIter& F);
    static std::vector<Interval> combine(const std::vector<Interval>& old, const std::vector<Interval>& nu);
    static int intervalScoreFun(const VarKeyedMap<int> &M, const VarSet &S);


  public:
    Word getLeft() const;
    short getLeftType() const;
    Word getRight() const;
    short getRightType() const;
    short getSign() const;
    void write() const;


    //AN interesting c++ learning experience:
    //Before I added leftType and rightType to the 1st constructor, it would always
    //call the first one because ints, shorts, and words are indistinguishable
    Interval(int lNum, int lDen, short leftType, int rNum, int rDen, short rightType, short sign);
    Interval(Word left, short leftType, Word right, short rightType,  short sign);
    static std::vector<Interval> findIntervals2(FernPolyIter &F1, FernPolyIter &F2);

    //Returns a tuple consisting of deduced signs (in the VKM), a VarSet of all the variables whose signs were deduced, and a short indicating the sign deduced on p
    static tuple<VarKeyedMap<int>, VarSet, short, Word> findIntervalsExplain(const VarKeyedMap<int> &varMap, PolyManager * PM, const IntPolyRef &formOne, const IntPolyRef &formTwo);
    static tuple<VarKeyedMap<int>, VarSet, short>  dedSignExp(const VarKeyedMap<int> &varMap, PolyManager * PM, const IntPolyRef &formOne, const IntPolyRef &formTwo, short formTwoSign);



    static VarKeyedMap<int> findIntervalsExplain(const VarKeyedMap<int> &varMap, PolyManager * PM, IntPolyRef &formOne, IntPolyRef &formTwo, const short &targetSign, const short &range, bool &success);

    static tuple<VarKeyedMap<int>, VarSet, short> deduceSign2(const VarKeyedMap<int> &varMap, PolyManager * PM, const IntPolyRef &formOne, const IntPolyRef &formTwo, short formOneSign, short formTwoSign) ;
  };

}//end namespace
#endif
