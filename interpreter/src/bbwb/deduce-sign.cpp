#include "deduce-sign.h"
#include "mono-explain.h"
#include "poly-explain.h"
#include "fern-poly-iter.h"

namespace tarski {
  /*
    The first is what the user is asking for an explanation for.
    The second is what the interval's sign is.
    The output is whether or not the interval is valid.
    This could also be called T_implies.
    Ex: If the user wants LEOP, but the interval proves LTOP, LTOP implies LEOP so its valid.
    If the user wants LTOP, but the interval proves LEOP, then the interval is useless
  */
  bool Interval::T_valid[8][8] = {
    //______|ZERO_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
    /*ZERO*/{NAY, NAY, NAY, NAY, NAY, NAY, NAY, YEA},
    /*LTOP*/{NAY, YEA, NAY, YEA, NAY, YEA, NAY, YEA},
    /*EQOP*/{NAY, NAY, YEA, YEA, NAY, NAY, YEA, YEA},
    /*LEOP*/{NAY, NAY, NAY, YEA, NAY, NAY, NAY, YEA},
    /*GTOP*/{NAY, NAY, NAY, NAY, YEA, YEA, YEA, YEA},
    /*NEOP*/{NAY, NAY, NAY, NAY, NAY, YEA, NAY, YEA},
    /*GEOP*/{NAY, NAY, NAY, NAY, NAY, NAY, YEA, YEA},
    /*ALOP*/{NAY, NAY, NAY, NAY, NAY, NAY, NAY, YEA}
  };


  /*
    Indicates that NOOP is a better restriction then LTOP/GTOP/EQOP, and so forth
    Useful for determining which interval is the best pick.
    I.E., if p.sign is LTOP, and deducedsign & p.sign is NOOP, you want to learn NOOP
  */
  short Interval::T_restrict[8] = {
    //ZERO_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
    0,   1,   1,   2,   1,   2,   2,   3
  };


  /*
    First value - whether t is negative (0) or positive (1)
    Second value - sign of q
    Third value - sign of p + tq
    Output - value of p
    Note: this can be generated / verified automatically with 
          QE.  For example, if T_interval[0][LEOP][LTOP] = LTOP is valid if
          for all t,q,p it holds that 

          [t < 0 /\ q <= 0 /\ p + t*q < 0 ] ==> [ p < 0 ] ] ])

          In general, we should have T_interval[ts][sig_q][sig_sum] = sig_p
          iff sig_p is the strongest relop s.t. 

          all t,q,p [ [ts * t > 0 /\ q sig_q 0 /\ p + t*q sig_sum 0 ] ==> p sig_p 0 ]        
  */
  short Interval::T_interval[2][8][8] = {
    {
      //______|NOOP_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
      /*NOOP*/{NOOP,NOOP,NOOP,NOOP,NOOP,NOOP,NOOP,NOOP},
      /*LTOP*/{NOOP,LTOP,LTOP,LTOP,ALOP,ALOP,ALOP,ALOP},
      /*EQOP*/{NOOP,LTOP,EQOP,LEOP,GTOP,NEOP,GEOP,ALOP},
      /*LEOP*/{NOOP,LTOP,LEOP,LEOP,ALOP,ALOP,ALOP,ALOP},
      /*GTOP*/{NOOP,ALOP,GTOP,ALOP,GTOP,ALOP,GTOP,ALOP},
      /*NEOP*/{NOOP,ALOP,NEOP,ALOP,ALOP,ALOP,ALOP,ALOP},
      /*GEOP*/{NOOP,ALOP,GEOP,ALOP,GTOP,ALOP,GEOP,ALOP},
      /*ALOP*/{NOOP,ALOP,ALOP,ALOP,ALOP,ALOP,ALOP,ALOP}
    },
    {
      //______|NOOP_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
      /*NOOP*/{NOOP,NOOP,NOOP,NOOP,NOOP,NOOP,NOOP,NOOP},
      /*LTOP*/{NOOP,ALOP,GTOP,ALOP,GTOP,ALOP,GTOP,ALOP},
      /*EQOP*/{NOOP,LTOP,EQOP,LEOP,GTOP,NEOP,GEOP,ALOP},
      /*LEOP*/{NOOP,ALOP,GEOP,ALOP,GTOP,ALOP,GEOP,ALOP},
      /*GTOP*/{NOOP,LTOP,LTOP,LTOP,ALOP,ALOP,ALOP,ALOP},
      /*NEOP*/{NOOP,ALOP,NEOP,ALOP,ALOP,ALOP,ALOP,ALOP},
      /*GEOP*/{NOOP,LTOP,LEOP,LEOP,ALOP,ALOP,ALOP,ALOP},
      /*ALOP*/{NOOP,ALOP,ALOP,ALOP,ALOP,ALOP,ALOP,ALOP}
    }
  };


  /*
    Given a sign of t, q and p + tq in that order, returns
    an equivalent or weaker sign of p + tq which allows
    for the same deduction by T_interval.
  */
  short Interval::T_weak[2][8][8] = {
    {
      //______|NOOP_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
      /*NOOP*/{NOOP,NOOP,NOOP,NOOP,NOOP,NOOP,NOOP,NOOP},
      /*LTOP*/{NOOP,LEOP,LEOP,LEOP,GTOP,NEOP,GEOP,ALOP},
      /*EQOP*/{NOOP,LTOP,EQOP,LEOP,GTOP,NEOP,GEOP,ALOP},
      /*LEOP*/{NOOP,LTOP,LEOP,LEOP,GTOP,NEOP,GEOP,ALOP},
      /*GTOP*/{NOOP,LTOP,GEOP,LEOP,GEOP,NEOP,GEOP,ALOP},
      /*NEOP*/{NOOP,ALOP,NEOP,ALOP,ALOP,ALOP,ALOP,ALOP},
      /*GEOP*/{NOOP,LTOP,GEOP,LEOP,GTOP,NEOP,GEOP,ALOP},
      /*ALOP*/{NOOP,ALOP,ALOP,ALOP,ALOP,ALOP,ALOP,ALOP}
    },
    {
      //______|NOOP_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
      /*NOOP*/{NOOP,NOOP,NOOP,NOOP,NOOP,NOOP,NOOP,NOOP},
      /*LTOP*/{NOOP,LTOP,GEOP,LEOP,GEOP,NEOP,GTOP,ALOP},
      /*EQOP*/{NOOP,LTOP,EQOP,LEOP,GTOP,NEOP,GEOP,ALOP},
      /*LEOP*/{NOOP,LTOP,GEOP,LEOP,GTOP,NEOP,GEOP,ALOP},
      /*GTOP*/{NOOP,LEOP,LEOP,LEOP,GTOP,NEOP,GEOP,ALOP},
      /*NEOP*/{NOOP,LTOP,NEOP,LEOP,GTOP,NEOP,GEOP,ALOP},
      /*GEOP*/{NOOP,LTOP,LEOP,LEOP,GTOP,NEOP,GEOP,ALOP},
      /*ALOP*/{NOOP,ALOP,ALOP,ALOP,ALOP,ALOP,ALOP,ALOP}
    }

  };


  /*
    Simple getter methods below
  */

  Word Interval::getLeft() const {
    return left;
  }
  Word Interval::getRight() const {
    return right;
  }
  short Interval::getSign() const {
    return sign;
  }
  short Interval::getLeftType() const {
    return leftType;
  }
  short Interval::getRightType() const {
    return rightType;
  }

  void Interval::write() const{
    std::cout << "{";
    if (leftType == INCLUSIVE) std::cout <<"[";
    else std::cout << "(";
    if (leftType == INFTY) std::cout << "-INFTY";
    else RNWRITE(left);
    if (leftType != INCLUSIVE) std::cout << ", ";
    if (rightType == INFTY) std::cout << "+INFTY";
    else if (rightType == INCLUSIVE) ; //do nothing
    else RNWRITE(right);
    if (rightType == INCLUSIVE) std::cout << "], ";
    else std::cout << "), ";
    std::cout <<  FernPolyIter::numToRelop(sign) + "}";
  }


  /*
    A manual constructor
  */
  Interval::Interval(int lNum, int lDen, short leftType, int rNum, int rDen, short rightType, short sign) {
    this->left = RNRED(lNum, lDen);
    this->leftType = leftType;
    this->right = RNRED(rNum, rDen);
    this->rightType = rightType;
    this->sign = sign;
  }

  /*
    A constructor based on words
  */
  Interval::Interval(Word left, short leftType, Word right, short rightType, short sign) {
    this->left = left;
    this->leftType = leftType;
    this->right = right;
    this->rightType = rightType;
    this->sign = sign;
  }

  /*
    FindIntervals2 algorithm, where the two iterators are pointing to the same monomial. Advances both iterators. Returns a std::vector of 3 elements.
  */
  std::vector<Interval> Interval::equalsCase(FernPolyIter& F1, FernPolyIter& F2) {
    std::vector<Interval> l;
    //std::cerr << "equals case\n";
    //std::cerr << "BETA IS " << BETA << std::endl;
    Word bi = F2.getCoefficient();
    Word ai = F1.getCoefficient();
    short ms = F2.getMonoSign();
    //cout << "ms in equalsCase is " << numToRelop(ms) << endl;
    //cout << "bi in equalsCase is " << bi << endl;
    short sl = LTOP;
    short sm = EQOP;
    short sr = GTOP;
    if (ISIGNF(bi) < 0) {
      sl = GTOP;
      sr = LTOP;
    }
    Interval i1(RNRED(1, 1), INFTY, RNNEG(RNRED(ai, bi)), EXCLUSIVE,  T_prod[sl][ms]);
    //std::cout << "i1 = "; i1.write(); std::cout << std::endl;
    Interval i2(RNNEG(RNRED(ai, bi)), INCLUSIVE, RNNEG(RNRED(ai, bi)), INCLUSIVE,  EQOP);
    //std::cout << "i2 = "; i2.write(); std::cout << std::endl;
    Interval i3(RNNEG(RNRED(ai, bi)), EXCLUSIVE, RNNEG(RNRED(1, 1)), INFTY,  T_prod[sr][ms]);
    //std::cout << "i3 = "; i3.write(); std::cout << std::endl << std::endl;

    l.push_back(i1);
    l.push_back(i2);
    l.push_back(i3);
    F2.next();
    F1.next();
    return l;
  }

  /*
    FindIntervals2, the step where q only has the term, and p has the term with coefficient 0. Advances the iterator. Returns a std::vector of 3 elements.
  */
  std::vector<Interval> Interval::qOnlyCase(FernPolyIter& F) {
    std::vector<Interval> l;
    //std::cerr << "qonly case\n";
    //std::cerr << "F2 is "; F.write(); std::cerr << std::endl << std::endl;
    int bi = ISIGNF(F.getCoefficient());
    int ai = 0;

    short ms = F.getMonoSign();
    //cout << "ms in qCase is " << numToRelop(ms) << endl;
    short sl = LTOP;
    short sm = EQOP;
    short sr = GTOP;

    if (bi < 0) {
      sl = GTOP;
      sr = LTOP;
    }

    Interval i1(RNRED(1, 1), INFTY, RNNEG(RNRED(ai, bi)), EXCLUSIVE,  T_prod[sl][ms]);
    //std::cout << "i1 = "; i1.write(); std::cout << std::endl;
    Interval i2(RNNEG(RNRED(ai, bi)), INCLUSIVE, RNNEG(RNRED(ai, bi)), INCLUSIVE,  EQOP);
    //std::cout << "i2 = "; i2.write(); std::cout << std::endl;
    Interval i3(RNNEG(RNRED(ai, bi)), EXCLUSIVE, RNNEG(RNRED(1, 1)), INFTY,  T_prod[sr][ms]);
    //std::cout << "i3 = "; i3.write(); std::cout << std::endl << std::endl;

    l.push_back(i1);
    l.push_back(i2);
    l.push_back(i3);

    F.next();
    return l;
  }

  /*
    FindIntervals2 algorithm, the case where p only has the monomial and q has it with coefficient 0. Advances the iterator. Returns a std::vector of 1 element.
  */
  std::vector<Interval> Interval::pOnlyCase(FernPolyIter& F) {
    std::vector<Interval> l;
    //cout << "pOnlyCase\n";
    short finSign = F.getTermSign();
    Interval i(RNRED(1, 1), INFTY,  RNRED(1, 1), INFTY,  finSign);
    //std::cout << "i: "; i.write(); std::cout << std::endl << std::endl;
    l.push_back(i);
    F.next();
    return l;
  }



  /*
    Combinetwo intervals - a surprising amount of code based on the way I decided to implement Exclusive vs Inclusive vs Infinity
  */
  std::vector<Interval> Interval::combine(const std::vector<Interval>& old, const std::vector<Interval>&  nu)  {
    std::vector<Interval> out;


    for (std::vector<Interval>::const_iterator iter = old.begin(); iter != old.end(); ++iter) {
      for (std::vector<Interval>::const_iterator iter2 = nu.begin(); iter2 != nu.end(); ++iter2) {
        //std::cerr << "Trying Q: "; iter->write(); std::cerr << " L: "; iter2->write(); std::cerr << std::endl;

        Word newLeft = RNRED(1, 1);
        Word newRight = RNRED(1, 1);
        short leftType = EXCLUSIVE;
        short rightType = EXCLUSIVE;
        short finSign = T_sum[iter->getSign()][iter2->getSign()];
        if (finSign == ALOP || finSign == NOOP) {
          //std::cerr << "interval rejected - bad sign" << std::endl;
          continue;
        }

        //Both sides are inclusive
        if (iter->getLeftType() == INCLUSIVE && iter2->getLeftType() == INCLUSIVE) {
          //std::cerr << "i1\n";
          Word l = iter->getLeft();
          Word l2 = iter2->getLeft();
          if (RNCOMP(l, l2) != 0 ){
            //std::cerr << "interval rejected - invalid range" << std::endl;
            continue;
          }
          leftType = INCLUSIVE;
          rightType = INCLUSIVE;
          newLeft = l;
          newRight = l;
        }
        else if (iter->getLeftType() == INCLUSIVE) {
          //std::cerr << "ia\n";
          Word l = iter->getLeft();
          Word l2 = iter2->getLeft();
          Word r2 = iter2->getRight();
          short l2Type = iter2->getLeftType();
          short r2Type = iter2->getRightType();
          //iter2 is all infinite, so no need to do comparisons
          if (l2Type == INFTY && r2Type == INFTY) {
            //std::cerr << "ia1\n";
            newLeft = l;
            newRight = l;
            leftType = INCLUSIVE;
            rightType = INCLUSIVE;
          }
          //iter2 is left infinite, so we only need to verify iter falls within the range of the right side of iter2
          else if (l2Type == INFTY) {
            //std::cerr << "ia2\n";
            if (RNCOMP(l, r2) != -1) {
              ////std::cerr << "interval rejected - invalid range" << std::endl;
              continue;
            }
            leftType = INCLUSIVE;
            rightType = INCLUSIVE;
            newLeft = l;
            newRight = l;
          }
          //The same as above, but for the other sides
          else if (r2Type == INFTY) {
            //std::cerr << "ia3\n";
            if (RNCOMP(l, l2) != 1) {
              ////std::cerr << "interval rejected - invalid range" << std::endl;
              continue;
            }
            leftType = INCLUSIVE;
            rightType = INCLUSIVE;
            newLeft = l;
            newRight = l;
          }
          //No infinite
          else {
            //std::cerr << "ia4\n";
            if (RNCOMP(l, l2) != 1 || RNCOMP(l, r2) != -1){
              ////std::cerr << "interval rejected - invalid range" << std::endl;
              continue;
            }
            leftType = INCLUSIVE;
            rightType = INCLUSIVE;
            newLeft = l;
            newRight = l;
            //Check if l > l2
            //Check if l < r2
            //If either fails, continue
            //Else, newLeft and newRight = l, type is INCLUSIVE on both sides
          }
        }
        else if (iter2->getLeftType() == INCLUSIVE) {
          //std::cerr << "ib\n";
          Word l = iter2->getLeft();
          Word l2 = iter->getLeft();
          Word r2 = iter->getRight();
          short l2Type = iter->getLeftType();
          short r2Type = iter->getRightType();
          if (l2Type == INFTY && r2Type == INFTY) {
            //std::cerr << "ib1\n";
            newLeft = l;
            newRight = l;
            leftType = INCLUSIVE;
            rightType = INCLUSIVE;
          }
          //iter2 is left infinite, so we only need to verify iter falls within the range of the right side of iter2
          else if (l2Type == INFTY) {
            //std::cerr << "ib2\n";
            if (RNCOMP(l, r2) != -1) {
              ////std::cerr << "interval rejected - invalid range" << std::endl;
              continue;
            }
            leftType = INCLUSIVE;
            rightType = INCLUSIVE;
            newLeft = l;
            newRight = l;
          }
          //The same as above, but for the other sides
          else if (r2Type == INFTY) {
            //std::cerr << "ib3\n";
            if (RNCOMP(l, l2) != 1) {
              ////std::cerr << "interval rejected - invalid range" << std::endl;
              continue;
            }
            leftType = INCLUSIVE;
            rightType = INCLUSIVE;
            newLeft = l;
            newRight = l;
          }
          //No infinity on either side of iter1, so must range check both sides
          else {
            //std::cerr << "ib4\n";
            if (RNCOMP(l, l2) != 1 || RNCOMP(l, r2) != -1){
              ////std::cerr << "interval rejected - invalid range" << std::endl;
              continue;
            }
            leftType = INCLUSIVE;
            rightType = INCLUSIVE;
            newLeft = l;
            newRight = l;
            //Check if l > l2
            //Check if l < r2
            //If either fails, continue
            //Else, newLeft and newRight = l, type is INCLUSIVE on both sides
          }
        }

        else {
          ////std::cerr << "In INFNITY DERIVING\n";
          //Left sides of both are infinity
          if (iter->getLeftType() == INFTY && iter2->getLeftType() == INFTY) {
            //std::cerr << "l1\n";
            leftType = INFTY;
          }
          //iter 1 has left infinity
          else if (iter->getLeftType() == INFTY) {
            //std::cerr << "l2\n";
            leftType = EXCLUSIVE;
            newLeft = iter2->getLeft();
          }
          //iter 2 has left infinity
          else if (iter2->getLeftType() == INFTY) {
            //std::cerr << "l3\n";
            leftType = EXCLUSIVE;
            newLeft = iter->getLeft();
          }
          //neither have left infinity
          else {
            //std::cerr << "l4\n";
            leftType = EXCLUSIVE;
            newLeft = RNMAX(iter->getLeft(), iter2->getLeft());
          }

          //Right sides of both are infinity
          if (iter->getRightType() == INFTY && iter2->getRightType() == INFTY) {
            rightType = INFTY;
            //std::cerr << "r1\n";
          }
          //iter 1 has right infinity
          else if (iter->getRightType() == INFTY) {
            rightType = EXCLUSIVE;
            newRight = iter2->getRight();
            //std::cerr << "r2\n";
          }
          //iter 2 has right infinity
          else if (iter2->getRightType() == INFTY) {
            rightType = EXCLUSIVE;
            newRight = iter->getRight();
            //std::cerr << "r3\n";
          }
          //neither have right infinity
          else {
            rightType = EXCLUSIVE;
            newRight = RNMIN(iter->getRight(), iter2->getRight());
            //std::cerr << "r4\n";
          }
        }

        //Check if left side is smaller than right side if both sides are expected to be rational numbers
        if (leftType == EXCLUSIVE && rightType == EXCLUSIVE && RNCOMP(newLeft, newRight) != -1) {
          continue;
        }

        //Construct a new interval
        Interval i(newLeft, leftType, newRight, rightType, finSign);
        out.push_back(i);
        //std::cerr << "Interval out: "; i.write(); std::cerr << std::endl;

      }
    }
    /*
    std::cerr << "survivors: \n";
    int i = 1;
    for (std::vector<Interval>::const_iterator iter = out.begin(); iter != out.end(); ++iter) {
      std::cerr << "j" << i << ": "; iter->write(); std::cerr << std::endl;
      i++;
    }
    std::cerr << std::endl;
    */
    return out;
  }

  /*
    Performs the findIntervals2 algorithm.
    F1 and F2 must be fresh iterators at the first term of the polynomial.
  */
  std::vector<Interval> Interval::findIntervals2(FernPolyIter &F1, FernPolyIter &F2){
    std::vector<Interval> q;

    bool first = true;
    //std:cerr << "------------------------------------------------------------------------------------------------------\n";
    while (!F1.isNull() && !F2.isNull()){
      //std::cerr << "F1 is"; F1.write();
      //std::cerr << "F2 is"; F2.write();
      std::vector<Interval> l;
      int res = F1.compareTo(F2);
      //If res is 0, then both p and q have Mi
      //If res is less than 0, only p has Mi
      //If res is greater tan 0, only q has Mi
      if (res == 0) {
        //cerr << "Equals\n";
        l = equalsCase(F1, F2);
      }
      else if (res == 1) {
        //cerr << "Qonly\n";
        l = qOnlyCase(F2);
      }
      else  {
        //cerr << "Ponly\n";
        l = pOnlyCase(F1);
      }

      if (first){
        q = l;
        first = false;
      }
      else {
        q = combine(q, l);

      }
    }
    //Iterate through all remaining monomials
    while (!F1.isNull()) {
      q = combine(q, pOnlyCase(F1));
    }
    while (!F2.isNull()) {
      q = combine(q, qOnlyCase(F2));
    }
    return q;
  }


  int Interval::intervalScoreFun(const VarKeyedMap<int> &M, const VarSet &S) {
    int score = 0;
    for(VarSet::iterator itr = S.begin(); itr != S.end(); ++itr) {
      score += signScores[M.get(*itr)];
    }
    return score;
  }


  tuple<VarKeyedMap<int>, VarSet, short>  Interval::deduceSign2(const VarKeyedMap<int> &varMap, PolyManager * PM,
								const IntPolyRef &formOne, const IntPolyRef &formTwo,
								short formOneSign, short formTwoSign)
  {
    FernPolyIter F1;
    FernPolyIter F2;
    bool v1 = false;
    VarKeyedMap<int> varMap2(varMap);
    if (formOne->isVar() && varMap.get(formOne->isVariable()) != ALOP) {
      varMap2[formOne->isVariable()] = ALOP;
      formTwoSign = formTwo->signDeduce(varMap2);
      F1 = FernPolyIter(formOne->getSaclibPoly(), formOne->getVars(), varMap2);
      F2 = FernPolyIter(formTwo->getSaclibPoly(), formTwo->getVars(), varMap2);
      v1 = true;
    }
    else  {
      F1 = FernPolyIter(formOne->getSaclibPoly(), formOne->getVars(), varMap);
      F2 = FernPolyIter(formTwo->getSaclibPoly(), formTwo->getVars(), varMap);
    }
    //Find the intervals of the interators
    std::vector<Interval> intervals = findIntervals2(F1, F2);


    VarKeyedMap<int> bestExplain;
    int minScore = -1;
    short bestSign = ALOP;
    short bestOut = ALOP;
    VarSet allVars;
    bool success = true;

    //Here, we generate the intPolyRef for td * p + tn * q, and then compute an explanation for its sign by using PolySign
    //Then, we score the explanation
    //The explanation with the minimum score wins
    int i = 0;
    //std::cerr << intervals.size() << " is the num of intervals" << std::endl;
    if (intervals.size() == 0) {
      //std::cerr << "EARLY RETURN";
      short x = ALOP;
      tuple<VarKeyedMap<int>, VarSet, short> toReturn(varMap, formOne->getVars(), x);
      return toReturn;
    }

    for (std::vector<Interval>::iterator iter = intervals.begin(); iter != intervals.end(); ++iter) {

      Interval inter = *iter;
      if (inter.getLeftType() == INCLUSIVE && inter.getLeft() == 0) {
        continue;
      }
      if (verbose && false) {
        std::cerr << "Examining interval " << i << ": ";
        inter.write();
        std::cerr << std::endl;
      }
      short sign = inter.getSign();
      if (sign == ALOP) continue;
      if (sign == GTOP)  {
        if ((GEOP & formOneSign) == (GTOP & formOneSign))
          sign = GEOP;
      }
      else if (sign == LTOP)  {
        if ((LEOP & formOneSign) == (LTOP & formOneSign))
          sign = LEOP;
      }

      Word t = 1; // td * p + tn * q
      Word td;
      Word tn;
      if (inter.getLeftType() == INCLUSIVE) {
        t = inter.getLeft();
        tn = RNNUM(t);
        td = RNDEN(t);
        //std::cerr <<  "FIE: C0\n";
      }
      else {
        Word left = 0;
        Word right = 0;
        //If one side is infinity, you can't really take an average
        //My stupid yet easy solution:
        //Both sides are infinity - set t to 0
        //Left side is infinity - set t the right side minus one
        //Right side is infinity - set to the left side plus one
        if (inter.getLeftType() == INFTY && inter.getRightType() == INFTY) {
          t = RNINT(0);
          tn = RNINT(0);
          td = RNINT(0);
        }
        else if (inter.getLeftType() == INFTY) {
          right = inter.getRight();
          t = RNSUM(right, RNINT(-4));
          //std::cerr << "RIGHT: "; RNWRITE(right); OWRITE(right); std::cerr << std::endl;
          td = RNDEN(t);
          tn = RNNUM(t);
        }
        else if (inter.getRightType() == INFTY) {
          left = inter.getLeft();
          t = RNSUM(left, RNINT(4));
          td = RNDEN(t);
          tn = RNNUM(t);
        }
        else {
          left = inter.getLeft();
          right = inter.getRight();
          t = RNSUM(left, right); //sum
          t = RNQ(t, RNINT(2)); //divide by 2, t is now the average
          td = RNDEN(t);
          tn = RNNUM(t);
          //std::cerr << "FIE: C4\n";
        }


      }
      short tSign = RNSIGN(t);
      if (verbose && false) {
        std::cerr << "t is ";
        RNWRITE(t);
        std::cerr << "RNSIGN(t) is " << RNSIGN(t) << std::endl;
      }
      



      if (tSign == 0) continue;
      else if (tSign == -1) tSign = 0;
      if (T_interval[tSign][formTwoSign][sign] == ALOP) continue;
      if (T_interval[tSign][formTwoSign][sign] == LEOP &&
          (formOneSign == LTOP || formOneSign == EQOP)) continue;
      if (T_interval[tSign][formTwoSign][sign] == GEOP &&
          (formOneSign == GTOP || formOneSign == EQOP)) continue;
      if (T_interval[tSign][formTwoSign][sign] == NEOP &&
          (formOneSign == LTOP || formOneSign == GTOP)) continue;
      short currSign = T_interval[tSign][formTwoSign][sign] & formOneSign;
      if (T_restrict[currSign] > T_restrict[bestSign])
        continue;
      //cout << "currSign is now " << numToRelop(currSign);
      IntPolyRef p = formOne->integerProduct(td);
      IntPolyRef q = formTwo->integerProduct(tn);
      IntPolyRef fin = PM->sum(p, q);

      if (verbose && false) {
        std::cerr << "p: "; p->write(*PM); std::cerr << std::endl;
        std::cerr << "q: "; q->write(*PM); std::cerr << std::endl;
        std::cerr << "fin: "; fin->write(*PM); std::cerr << std::endl;
        std::cerr << "Want to prove "  << numToRelop(sign) << std::endl;
      }
      FernPolyIter F;

      F = (v1) ? FernPolyIter(fin->getSaclibPoly(), fin->getVars(), varMap2)
        : FernPolyIter(fin->getSaclibPoly(), fin->getVars(), varMap);

      bool success = true;
      //Note: In case of breakage, change currSign to sign
      VarKeyedMap<int> explain = (v1) ? select(varMap2, F, sign, success)
        : select(varMap, F, sign, success);
      if (verbose) std::cout << " ";
      if (!success) {
        std::cerr << "\nThere is an issue - I wasn't able to prove the sign " + FernPolyIter::numToRelop(sign) << " on tn * p + td * q\n";
        std::cerr << "t is "; RNWRITE(t); std::cerr << std::endl;
        std::cerr << "tn is "; OWRITE(tn); std::cerr << std::endl;
        std::cerr << "td is "; OWRITE(td); std::cerr << std::endl;
        std::cerr << "p: "; p->write(*PM); std::cerr << " " + numToRelop(formOneSign) << " 0 " << std::endl;
        std::cerr << "q: "; q->write(*PM); std::cerr << " " + numToRelop(formTwoSign) << " 0 " << std::endl;
        std::cerr << "fin: "; fin->write(*PM); std::cerr << std::endl;
        throw TarskiException("ERROR - call to PolySign failed in DeduceSignExplain");
      }
      else {
        int score = intervalScoreFun(varMap, fin->getVars());
        if (T_restrict[currSign] < T_restrict[bestSign] || score < minScore || minScore == -1) {

          minScore = score;
          bestExplain = explain;
          allVars = fin->getVars();
          bestSign = currSign;
          bestOut = T_interval[tSign][formTwoSign][sign];
        }
        i++;
      }
    }
    tuple<VarKeyedMap<int>, VarSet, short> toReturn(bestExplain, allVars, bestOut);
    return toReturn;
  }
}//end namespace
