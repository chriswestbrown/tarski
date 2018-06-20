#ifndef _READ_SMT_RET_FORMULA_
#define _READ_SMT_RET_FORMULA_
#include "../formula/formula.h"

namespace tarski {
  
// Read SMTLIB from input stream
// and at the first "assert" spit out the formula
// as a TFormRef that we're supposed to check the 
// satisfiability of. 
TFormRef readSMTRetFormula(std::istream& in, PolyManager* PM);

void readSMTRetTarskiString(std::istream& in, std::ostream& out);

TFormRef processExpFormula(const std::string& instr, PolyManager* PM);

TFormRef processExpFormulaClearDenominators(const std::string& instr, PolyManager* PM);

void writeSMTLIB(TFormRef F, std::ostream& out);

}//end namespace tarski
#endif

