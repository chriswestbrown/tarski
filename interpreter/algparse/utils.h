#ifndef _INTERPRETER_UTILS_
#define _INTERPRETER_UTILS_

#include <set>
#include <string>
#include <iostream>

namespace tarski {

/*********************************************************************
 * intersection function for sets of strings
 *********************************************************************/
std::set<std::string> intersect(const std::set<std::string>&A, const std::set<std::string>&B);

/*********************************************************************
 * print function for sets of strings
 *********************************************************************/
void print(std::ostream& out, const std::set<std::string>& A);

}// end namespace tarski
#endif
