#include "utils.h"

namespace tarski {
std::set<std::string> intersect(const std::set<std::string>&A, const std::set<std::string>&B)
{
  std::set<std::string> S;
  for(std::set<std::string>::iterator i = A.begin(); i != A.end(); ++i)
    if (B.find(*i) != B.end())
      S.insert(*i);
  return S;
}

void print(std::ostream& out, const std::set<std::string>& A) 
{ 
  std::set<std::string>::iterator i = A.begin(); 
  out << *i;
  for(++i;i != A.end(); ++i) { out << "," << *i; }
}
}
