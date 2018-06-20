#include "Tester.h"

using namespace Minisat;

void TestSolver::loadLits(vec<Lit>& lits, bool& conf, const vec<lbool>& assigns){
  for (vector<vector<Lit> >::iterator iter = toGive.begin(); iter != toGive.end(); ++iter) {
    vector<Lit> oldLits = *iter;
    if (isConflict(oldLits, assigns)) {
      for (int i = 0; i < oldLits.size(); i++) {
        lits.push(oldLits[i]);
      }
      currIdx++;
      conf = true;
      return;
    }
  }
  conf = false;
  return;
}



bool TestSolver::isConflict(vector<Lit>& lits, const vec<lbool>& assigns) {
  /*for (int i = 0; i < lits.size(); i++) {
    if (assigns[var(lits[i])] != l_False) return false;
  }
  */
  return true;
}


void TestSolver::getClause(const vec<Lit>& trail, const vec<int>& trail_lim, const vec<lbool>& assigns, int qhead, bool& conf, vec<Lit>& lits) {
  if (calledOnce){
    loadLits(lits, conf, assigns);
    return;
  }
  else {
    calledOnce = true;
    loadFromFile();
    loadLits(lits, conf, assigns);
    return;
  }
}

void TestSolver::loadFromFile() {

  std::string fname;
  cout << "Enter the file name\n";
  std::cin >> fname;
  std::ifstream infile;
  infile.open(fname.c_str());
  std::string in;
  while (1) {
    vector<Lit> lits;
    while (infile >> in){
      if (in == "0") {
        toGive.push_back(lits);
        break;
      }
      int parsed_lit = atoi(in.c_str());
      Var var = abs(parsed_lit)-1;
      lits.push_back( (parsed_lit > 0) ? mkLit(var) : ~mkLit(var) );
    }
    if (infile.eof()) break;
  }
}

