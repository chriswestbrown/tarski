#include <algorithm>
#include "mhs-gen.h"
#include <unordered_set>
#include <vector>
#include <cmath>
#include "minhitset/naive/hitset.h"
#include "../../../minisat/core/SolverTypes.h"
#include "../../../minisat/mtl/Vec.h"

using namespace std;
using namespace Minisat;
#define u_s unordered_set<int>

inline bool vecSortFunc (std::vector<int> A, std::vector<int> B) {
  return A.size() < B.size();
}

int MHSGenerator::intFromLit(Lit l) {
  int i = var(l);
  if (sign(l) == true) {
    i *= -1;
    i -= 1;
  }
  else i += 1;
  return i;
}

int MHSGenerator::absDec(int i) {
  if (i < 0)  i++;
  else i--;
  return i;
}

Lit MHSGenerator::toLit(int i) {
  if (i < 0) { i++; return mkLit(i, true); }
  else {i--; return mkLit(i, false); }
}

vector<Lit> MHSGenerator::genMHS(const vec<Lit>& trail) {
  u_s allTrail = genAllTrail(trail);
  vector<int> forceLearn;
  vector<vector<int> > toCalc = preProcess(allTrail, forceLearn);
  vector<int> res;
  if (toCalc.size() == 1) {
    res.push_back(toCalc[0][0]);
  }
  else if (toCalc.size() != 0) {
    HitProb H;
    H.fill(maxFormula, toCalc.size(), toCalc);
    naiveSolve(H, res);
  }
  res.insert(res.end(), forceLearn.begin(), forceLearn.end());
  res.insert(res.end(), required.begin(), required.end());
  vector<Lit> toRet;
  for (vector<int>::iterator itr = res.begin(), end = res.end(); itr != end; ++itr) {
    toRet.push_back(toLit(*itr));
  }

  
  cout << endl;
  cout << "TRAIL: ";
  for (int i = 0; i < trail.size(); i++) {
    write(trail[i]); cout << " ";
  }
  cout << endl;
  cout << "RES: ";
  for (int i = 0; i < toRet.size(); i++) {
  write(toRet[i]); cout << " ";
  }
  


  return toRet;
}

u_s MHSGenerator::genAllTrail(const vec<Lit>& trail) {
  u_s allTrail;
  for (int i = 0; i < trail.size(); i++)
    allTrail.insert(intFromLit(trail[i]));
  return allTrail;
}

//Note we need to preserve the 0 at the end
//Note we can guarantee a 0 at the end
void MHSGenerator::fastErase(vector<int>& A, int i) {
  A.back() = A[i];
  A.pop_back();
  A[i] = A.back();
  A.back() = 0;
}

void MHSGenerator::fastErase(vector<vector<int> >& A, int i) {
  vector<int> tmp = A.back();
  A.back() = A[i];
  A[i] = tmp;
  A.pop_back();
}



//Removes all clauses satisfied by a purely propositional variable
//Removes all clauses satisfied by only one variable and adds the variable to forceLearn
//Removes all clauses satsified by a required variable
//Removes all purely propositional variables
//Removes all negative theory variables
std::vector<std::vector <int> > MHSGenerator::preProcess (u_s& allTrail, vector<int>& forceLearn ){
  std::vector<std::vector<int> > toCalc(clauses);

  for (int i = toCalc.size()-1; i >= 0; i--) {
    bool isRemoved = false;
    /*
    cout << "Clause: ";
    for (int j = 0; j < toCalc[i].size(); j--) {
      cout << absDec(toCalc[i][j]) << " ";
    }
    cout << endl;
    */
    for (int j = toCalc[i].size()-2; j >= 0; j--) {
      if (abs(toCalc[i][j]) > maxFormula && allTrail.find(toCalc[i][j]) != allTrail.end() ) {
        //cout << "REMOVING CLAUSE 1\n";
        fastErase(toCalc, i);
        isRemoved = true;
        break;
      }
      else if (required.find(toCalc[i][j]) != required.end()) {
        //cout << "REMOVING CLAUSE 2\n";
        fastErase(toCalc, i);
        isRemoved = true;
        break;
      }
      else if (abs(toCalc[i][j]) > maxFormula){
        //cout << "REMOVING " << absDec(toCalc[i][j]) << endl;
        fastErase(toCalc[i], j);
      }
      else if (allTrail.find(toCalc[i][j]) == allTrail.end()) {
        //cout << "REMOVING " << absDec(toCalc[i][j]) << endl;
        fastErase(toCalc[i], j);
      }
    }
    if (!isRemoved && toCalc[i].size() <=2 ) {
      if (toCalc[i].size() == 2){
        forceLearn.push_back(toCalc[i][0]);
        //cout << "FLEARNING " << absDec(toCalc[i][0]) << endl;
      }
      fastErase(toCalc, i);
    }
  }
  
  cout << "toCalc: \n";
  for (int i = 0; i < toCalc.size(); i++) {
    for (int j = 0; j < toCalc[i].size(); j++) {
      cout << absDec(toCalc[i][j]) << " ";
    }
    cout << endl;
  }
  
  return toCalc;
}





MHSGenerator::MHSGenerator(std::vector<std::vector <Lit> >& form, int maxForm) : maxFormula(maxForm+1) {
  for (int i = 0; i < form.size(); i++) {
    //For clauses of size 1, that variable must always be true
    if (form[i].size() == 1) {
      int tmp = intFromLit(form[i][0]);
      if (abs(tmp) > maxFormula) continue;
      required.insert(tmp);
      continue;
    }
    vector<int> toPush(form[i].size()+1);
    int j;
    for (j = 0; j < form[i].size(); j++) {
      toPush[j] = intFromLit(form[i][j]);
    }
    toPush[j] = 0;
    clauses.push_back(toPush);
  }
  std::sort(clauses.begin(), clauses.end(), vecSortFunc);
}
