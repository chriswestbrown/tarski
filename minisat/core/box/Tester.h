#ifndef TESTER_H
#define TESTER_H

#include "CallBack.h"
#include <iostream>
#include <fstream>
#include <vector>


#define vector std::vector
namespace Minisat {


  //Intended usage:
  //Call getClause once
  //On the first call, it loads from a file all the clauses it will teach MiniSAT
  //On each call to getClause, it will select another clause
  //A major problem with desigining this class is that vec has the = operator set to private 
  class TestSolver : public TSolver {
  private:
    vector<vector<Lit> > toGive; //The list of cluases this solver will automatically teach
    int currIdx;

    bool calledOnce; //On init set to false, sets true on first call to getClause
    inline bool isConflict(vector<Lit>& lits, const vec<lbool>& assigns);

    inline void loadLits(vec<Lit>&, bool&, const vec<lbool>& assigns);

    void loadFromFile();

  public:
    void getClause(const vec<Lit>& trail, const vec<int>& trail_lim, const vec<lbool>& assigns, int qhead, bool& conf, vec<Lit>& lits);


    void getAddition(bool& conf, vec<Lit>& lits) {conf = false; }
  TestSolver() : currIdx(0) {}

  };





}


#endif
