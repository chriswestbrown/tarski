#include "CallBack.h"
#include <string>
#include "../mtl/Vec.h"
#include <iostream>
#include <vector>

using namespace Minisat;

void UserSolver::getClause(vec<Lit>& lits, bool& conf) {
  std::cout << "Stack:"; printStack(); std::cout << "\n";
  std::cout << "Enter a new clause with vars, terminated by a 0\n";
  std::cout << "Skip by entering N\n";
  std::string in;
  while (std::cin >> in) {
    if (in == "0") {
      break;
    }
    if (in == "N") {
      conf = false;
      return;
    }
    else {
      int parsed_lit = atoi(in.c_str());
      Var var = abs(parsed_lit)-1;
      lits.push( (parsed_lit > 0) ? mkLit(var) : ~mkLit(var) );
    }
  }
  conf = true;

  //


}

void UserSolver::getAddition(vec<Lit>& lits, bool& conf) {
  conf = false;
}


void TSolver::printStack() {
  const vec<Lit>& trail = getTrail();
  const vec<int>& trail_lim = getTrailLim();
  const vec<lbool>& assigns = getAssigns();
  int qhead = getQhead();
  int tlidx = 0;
  for (int i = 0; i < qhead; i++) {
    if (assigns[var(trail[i])] == l_False) std::cout << "-"; else if (assigns[var(trail[i])] == l_True) std::cout << "+";
    std::cout <<  var(trail[i]);
    if (tlidx < trail_lim.size() && i == trail_lim[tlidx]) {std::cout << "d"; tlidx++;}
    std::cout << " ";
  }
}



