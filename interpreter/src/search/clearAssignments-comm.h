#ifndef _CLEAR_ASSIGNENTS_COMM_H_
#define _CLEAR_ASSIGNENTS_COMM_H_


#include "clearAssignments.h"
#include "../shell/einterpreter.h"
#include "../poly/variable.h"
#include "../poly/poly.h"
#include "../formula/formula.h"

#include <map>
#include <vector>

namespace tarski {

  /*******************************************************************
   ** Clear Assignments:  In a conjunction, what we might mean by 
   ** "solving" can be a little ambiguous, and what moves we want to
   ** make to work towards a solution may involve many decisions. 
   ** However, one kind of move that requires no decisions is the move
   ** to clear "assignments".  Here we consider only assignments of
   ** the form a x = b and a x = b y.  These are assignments of constants
   ** to variables or of one variable to a constant multiple of another.
   ** 
   ** Clear-assignments efficiently detects and makes such assignments.
   ** We note, however, that making the assignments may create new
   ** opportunities to make more assignments ... though to take advantage
   ** of them, clear-assignments would have to be called again.
   ** 
   ** This command works by creating a graph of such assignments (see
   ** comments in the code) and finding connected components in that
   ** graph, each of which represents a std::set of variables that are all
   ** constant multiples of one another (or are assigned constant values).
   *******************************************************************/

  
  TFormRef makeAssignments(TFormRef F,
                           VarKeyedMap<GCWord> &constants, 
                           const pair<GCWord,VarSet> &nada, 
                           VarKeyedMap< pair<GCWord,VarSet> > &multiples);


  class ClearAssignmentsComm : public EICommand
  {

  public:
  ClearAssignmentsComm(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, std::vector<SRef> &args);
    std::string testArgs(std::vector<SRef> &args) { return require(args,_tar); }
    std::string doc() { return "This command detects assignments in the input formula, and simplifies the formula by making those assignements.  Thus, \n\n   (clear-assignments [ x = y /\\ t = 3 /\\ x + y + t < 1]) \n\ngives y + 1 < 0 as a result.  Note that no attempt is made to detect are take advantage of new assignments that are created by these substitutions.  That would require calling clear-assignments a second time."; }
    std::string usage() { return "(clear-assignments <tar>)"; }
    std::string name() { return "clear-assignments"; }
  };





}//end namespace tarski

#endif

