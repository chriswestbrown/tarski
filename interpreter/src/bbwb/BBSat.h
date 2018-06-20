#ifndef _BBSAT_H_
#define _BBSAT_H_

#include "bbwb.h"
#include "../shell/einterpreter.h"
#include "../formula/monomialinequality.h"
#include "../formula/formula.h"
#include <vector>
#include <set>
#include <queue>
#include <map>
#include <ctime>
#include <string>
#include "bbded.h"



namespace tarski {
  class BBSatComm : public BBWBComm
  {


  public:
  BBSatComm(NewEInterpreter* ptr) : BBWBComm(ptr) { }
    SRef execute(SRef input, vector<SRef> &args);
    string testArgs(vector<SRef> &args) { return ""; }
    string doc() { return "This command evaluates a formula and determines whether or not it is BlackBox Sat. If it is, it returns true. Each deduction which can be made is accompanied in a TarObj by the explanation such that the first element is the deduction and the remainder are the dependencies. Otherwise, it returns an explanation of why it is false according to BlackBox Satisfiability."; }
    string usage() { return "(bbsat <Formula>)"; }
    string name() { return "bbsat"; }

  };


} //end namespace

#endif
