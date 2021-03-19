#include "termbase.h"

using namespace tarski;

void TermBase::topologicalSort(std::vector<TermId> &inTerms)
{
  std::vector<TermId> res;
  const int LWHITE = 0, LGRAY = 1, LBLACK = 2;
  std::unordered_map<TermId,int> color;
  for(auto itr = inTerms.begin(); itr != inTerms.end(); ++itr)
    color[*itr] = LWHITE;

  std::stack<
    std::pair< TermId,
	       std::pair<std::vector<TermId>::iterator, std::vector<TermId>::iterator> >
    > S;
  for(auto itr = inTerms.begin(); itr != inTerms.end(); ++itr)
  {
    switch(color[*itr])
    {
    case LWHITE: {
      auto pX = std::make_pair(dedfBegin(*itr),dedfEnd(*itr));
      S.push(
	     std::make_pair(*itr,pX)
	     );
      while(!S.empty())
      {
	auto& next = S.top();
	auto citr = next.second.first;
	auto cEnd = next.second.second;
	if (citr == cEnd) { S.pop(); color[next.first] = LBLACK; res.push_back(next.first); continue; }
	else { next.second.first++; } // increment child iterator
	TermId childTerm = *citr;
	auto findRes = color.find(childTerm);
	switch(findRes == color.end() ? LBLACK : findRes->second)
	{
	case LWHITE: {
	  color[childTerm] = LGRAY;
	  S.push( std::make_pair(childTerm,
				 std::make_pair(dedfBegin(childTerm),dedfEnd(childTerm))) );
	} break;
	case LGRAY: { throw TarskiException("Cycle detected in topologicalSort!"); } break;
	case LBLACK: break;
	}
      }
    } break;
    case LGRAY: { throw TarskiException("Cycle detected in topologicalSort!"); } break;
    case LBLACK: break;
    }
  }
  swap(res,inTerms);
}

void TermBase::dump(PolyManager* pPM, TermContextRef TC)
{
  std::vector<TermId> contents;
  fillWithTerms(contents);
  topologicalSort(contents);
  for(int i = contents.size() - 1; i >= 0; --i)
  {
    std::cout << "table[" << contents[i] << "] = "
	      << TC->get(contents[i])->toString(pPM) << " "
	      << getReason(contents[i]);
    for(auto ditr = dedfBegin(contents[i]); ditr != dedfEnd(contents[i]); ++ditr)
      std::cout << " " << *ditr;

    std::cout << std::endl;
  }
}
