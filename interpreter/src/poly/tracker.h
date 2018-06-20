#ifndef _CWBTRACKER_
#define _CWBTRACKER_

#include "poly.h"

namespace tarski {
class Tracker
{
 public:
  class Hash 
  { 
  public: 
    unsigned char A[16]; 
    bool operator<(const Hash& H) const
    {   
      unsigned long long *K = (unsigned long long *)(void*)A, *J = (unsigned long long *)(void*)H.A;
      return K[0] < J[0] || (K[0] == J[0] && K[1] < J[1]);
    } 
  };
private:
  std::map< std::string, std::map<Hash,int> > M;

public:
  void recordUnordered(const std::string& category, Word X, Word Y)
  {
    record(category,(OCOMP(X,Y) > 0 ? LIST2(Y,X) : LIST2(X,Y)));
  }
  void record(const std::string& category, Word X);
  void recordSacPoly(const std::string& category, Word level, Word P)  { record(category,LIST2(level,P)); }
  void report();
  void clear() { M.clear(); }
  ~Tracker() { }
};

}//end namespace tarski
#endif
