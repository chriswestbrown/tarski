#ifndef _TARSKI_SHELL_UTILS_
#define _TARSKI_SHELL_UTILS_

#include <iostream>
#include <fstream>

namespace tarski {

// These two functions write from an input stream to an
// outputstream/string in a fixed number of columns.
void writeToFit(std::istream& in, std::ostream& out, int N, int lineWrapIndent = 0);
void writeToFit(const std::string& in, std::ostream& out, int N, int lineWrapIndent = 0);

// This gets the number of columns wide the terminal is currently at
int getTermWidth();

}//end namespace tarski

#endif
