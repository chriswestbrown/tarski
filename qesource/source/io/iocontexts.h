#include <iostream>


void PushInputContext(std::istream &in);
void PopInputContext();
void InputContextInit(std::istream& defaultin=std::cin);
Word setECHO(Word k);
Word setWASBKSP(Word k);
void PushOutputContext(std::ostream &out);
void PopOutputContext();
std::ostream& TopOutputContext();
void OutputContextInit(std::ostream &out=std::cout);
