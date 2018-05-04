#include "../formula/formula.h"

/*
The idea behind "clear-assignments" is that formulas often come with
"assignments" like x = 2 or y = 3 x.  In contrast with more general
linear equality constraints, there is no real decision to make about
what to do with these: if you see an assignment of a constant to
a variable, evaluate the formula with that variable assigned that
constant value.   If you see a constraint like 3 x = 5 y, choose one
variable, it matters not which, and substitute e.g. x <- 5/3 y throughout
the formula.  
*/


