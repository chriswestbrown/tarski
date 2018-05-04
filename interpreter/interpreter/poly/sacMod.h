extern "C" {
#include "saclib.h"
};
#include "caserver/CAPolicy.h"
#include <iostream>
#include <sstream>

//-- I/O Context operations
extern void InputContextInit();
extern void PushInputContext(std::istream &in);
extern void PopInputContext();

extern void OutputContextInit();
extern void PushOutputContext(std::ostream &out);
std::ostream& TopOutputContext();
extern void PopOutputContext();

//-- Initialization & Cleanup
//-- NOTE: "topOfTheStack" should be a pointer that points to a value
//--       that is above (resp. below) any std::stack allocated garbage
//--       collected Saclib Word.  
extern void SacModInit(int argc, char **argv, int &ac, char** &av,
		       std::string policy, std::string server, std::string dirPath,
		       void* topOfTheStack);
extern void SacModEnd();


//-- Info
extern int CATime(); // total time in ms for all computer algebra processes
extern void CAStats(); 

extern CAPolicy* GVCAP;
