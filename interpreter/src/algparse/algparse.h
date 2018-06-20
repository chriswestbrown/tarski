#ifndef _ALGPARSE_
#define _ALGPARSE_

#include "alglex.h"

/**************************************************************
 * Parses from LC and stores parse tree in result.
 * NOTE: algparse is *outside* the tarski namespace.  Unless and
 *       until I can convince bison to put it in the tarski
 *       namespace, that's just how it's going to have to be!
 **************************************************************/
int algparse (tarski::LexContext *LC, tarski::TarskiRef &result);

namespace tarski {
/**************************************************************
 * clearDenominators(T)
 * Given parse tree T for a formula that has non-constant
 * denominators, so that T is not actually a true tarski formula,
 * clearDenominators(T) returns the tarski formula Tf that is
 * equivalent to T UNDER THE ASSUMPTION that the formula T 
 * properly guards the denominators it uses.
 **************************************************************/
TarskiRef clearDenominators(TarskiRef T);

}//end namespace tarski
#endif
