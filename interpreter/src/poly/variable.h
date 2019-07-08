/************************************************************************
 * variable.h
 * This file defines the data-structure used to represent variables and
 * sets of variables. Variables are unnamed, but a "Variable Context"
 * structure attaches names to variables.
 *
 * Variables and Variable Sets are represented by a std::bitset in which a 
 * 1 representes the presence and a 0 the absence of a variable.  In
 * reality, an object of type Variable is just a VarSet that has one 
 * element (though this restriction is unenforced!)
 *
 * This system limits the number of variables to varNumLimit.  If you
 * increase varNumLimit, you increase the storage size of every VarSet
 * and, potentially, increase the time required for various operations.
 ************************************************************************/
#ifndef _CWBVARIABLE_
#define _CWBVARIABLE_

#include <iostream>
#include <bitset>
#include <string>
#include <map>

#define _VAR_LIMIT_64_
#undef _VAR_LIMIT_32_


#ifdef _VAR_LIMIT_32_

namespace tarski {  
  const int varNumLimit = 32, 
           specialMod = 53; // must be prime > varNumLimit + 2, not equal to
                            // 7,17,23,31,41,43,47,71,73,79,89,97,103,109,113,127,
                            // 137,151,157,167191,199,223,229,...
                            // essentially I need to be sure that you have a prime 
                            // p s.t. for 0 <= i,j < p-1, 2^i = 2^j mod p ==> i = j
  inline int bit2index(const std::bitset<varNumLimit>& v) { return v.to_ulong() % (unsigned long)specialMod; }
}//end namespace tarski
#endif

#ifdef _VAR_LIMIT_64_
namespace tarski {  
const int varNumLimit = 64, 
          specialMod = 67; // see above for description
inline int bit2index(const std::bitset<varNumLimit>& v) { return v.to_ullong() % (unsigned long long)specialMod; }
}//end namespace tarski
#endif

namespace tarski {  


class VarContext;
class VarSet;
typedef std::bitset<varNumLimit> Variable;

inline bool operator<(const Variable &x, const Variable &y) { return x.to_ulong() < y.to_ulong(); }


class VarSet : public std::bitset<varNumLimit>
{
private:
  int nextIndex(int i) const {
    while(++i < (int)size() && !test(i))
      ;
    return i;
  }
  int prevIndex(int i) const {
    while(--i >= 0 && !test(i))
      ;
    return i;
  }
  
public:
 VarSet(int i = 0) : std::bitset<varNumLimit>(i) { } // initialized to all zeros
 VarSet(const std::bitset<varNumLimit> &i) : std::bitset<varNumLimit>(i) { } 

  class iterator
  {
  public:
    int index;
    VarSet const *parent;
    /* to debug */ Variable deref;
    Variable& operator*() { deref = 1; deref <<= index; return deref; } //Variable var = 1; var <<= index; return var; /*return VarSet(1) << index;*/ }
    iterator& operator++() { index = parent->nextIndex(index); return *this; }
    iterator& operator--() { index = parent->prevIndex(index); return *this; }
    bool operator==(const iterator &i) { return index == i.index; }
    bool operator!=(const iterator &i) { return index != i.index; }
    iterator operator-(int k)
    {
      iterator itrT = *this;
      for(int i = k; i > 0; --i)
	--itrT;
      return itrT;
    }
  };

 public:
  iterator begin() const { iterator itr; itr.index = -1; itr.parent = this; ++itr; return itr; }
  iterator end() const { iterator itr; itr.index = size(); itr.parent = this; return itr; }
  iterator rend() const { iterator itr; itr.index = -1; itr.parent = this; return itr; }
  iterator rbegin() const { iterator itr; itr = end(); --itr; return itr; }
  int numElements() const { return count(); }
  void write() const { std::cout << (*this); }
  int positionInOrder(VarSet x) const; // returns i where x is the ith element of std::set, starting at 0.
  bool isEmpty() const { return !any(); } 
  VarSet operator&(const VarSet& X) const
  {
    return VarSet(std::operator&(*this,X));
  }
  VarSet operator&(const Variable& X) const
  {
    return VarSet(std::operator&(*this,X));
  }
  VarSet operator|(const VarSet& X) const 
  {
    return VarSet(std::operator|(*this,X));
  }
  VarSet operator^(const VarSet& X) const 
  {
    return VarSet(std::operator^(*this,X));
  }
  VarSet operator~() const { return VarSet(this->std::bitset<varNumLimit>::operator~()); }
  VarSet flip() const {  std::bitset<varNumLimit> tmp(*this); tmp.flip(); return VarSet(tmp); }
};

inline VarSet operator+(const VarSet &A, const VarSet &B) { return VarSet(A | B); }
inline VarSet operator-(const VarSet &A, const VarSet &B) { return VarSet(A & ~B); }
inline bool operator<(const VarSet &x, const VarSet &y) { return x.to_ulong() < y.to_ulong(); }

class VarContext
{
private:
  std::string names[specialMod]; 
  std::map<std::string,Variable> table;
  int nextIndex;
public:
  VarContext() { nextIndex = 0; }
  Variable getVar(const std::string &name);
  const std::string& getName(const Variable &v) {
    return names[ bit2index(v) ];
  }
  const std::string& getName(const VarSet &v) {
    return names[ bit2index(v) ];
  }
  Variable addVar(const std::string &name);
  Variable newVar(VarSet V); // returns a "new" varialbe - i.e. one not already in V.  Empty VarsSet if limit is reached
  void printVariables()
  {
    for(std::map<std::string,Variable>::iterator itr = table.begin(); itr != table.end(); ++itr)
      std::cout << itr->first << ":" << itr->second << std::endl;
  }
};

// This data structure allows fast lookups for maps keyed by a Variable
template<class T>
class VarKeyedMap
{
private:
  T table[specialMod];
  T _nullValue;
public:
  void clear() { for(int i = 0; i < specialMod; ++i) table[i] = _nullValue; }
  T getNullValue() { return _nullValue; }
  VarKeyedMap(const T &nv = 0) : _nullValue(nv) { clear(); }
  VarKeyedMap(const VarKeyedMap<T> &M) { _nullValue = M._nullValue; for(int i = 0; i < specialMod; ++i) table[i] = M.table[i]; }
  T& operator[](const Variable &v) { return table[ v.to_ulong() % (unsigned long)specialMod ]; }
  const T& get(const Variable &v) const { return table[ v.to_ulong() % (unsigned long)specialMod ]; }
 };
}//end namespace tarski
#endif

