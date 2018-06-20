#include "tracker.h"

#include "sacMod.h"
#include "md5digest.h"

using namespace std;

namespace tarski {

void Tracker::record(const string& category, Word X)
{
  Hash H;
  md5_digest_saclibObj(X,H.A);
  M[category][H]++;
}

void Tracker::report()
{
  for(map< string, map<Hash,int> >::iterator itr = M.begin(); itr != M.end(); ++itr)
  {
    int n_tot = 0, n_distinct = 0;
    for(map<Hash,int>::iterator itr2 = itr->second.begin(); itr2 != itr->second.end(); ++itr2)
    {
      ++n_distinct;
      n_tot += itr2->second;
    }
    cerr << "category: " << itr->first << " distinct: " << n_distinct << " ave: " << double(n_tot)/n_distinct << endl;
  }
}
}//end namespace tarski
