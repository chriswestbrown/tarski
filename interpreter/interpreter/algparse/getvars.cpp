#include <set>
#include "getvars.h"

namespace tarski {
void GetVars::operator()(Var* p)           
{ 
  if (boundVars.find(p->value) == boundVars.end())
  {  freeVars.insert(p->value); vars.insert(p->value); }
}
void GetVars::operator()(QuantifierBlock *p) { 
  for(unsigned int i = 0; i < p->vars.size(); ++i) { 
    std::string s = dynamic_cast<Var*>(&(*(p->vars[i])))->value;
    boundVars.insert(s); 
    vars.insert(s); 
  } 
  p->arg->apply(*this);
}

void RenameVar::operator()(Var* p)
{ 
  if (p->value == oldName) 
    p->value = newName;
}
void RenameVar::operator()(QuantifierBlock *p) 
{ 
  unsigned int i;
  for(i = 0; i < p->vars.size() && oldName != varName(p->vars[i]); ++i);
  if (i ==  p->vars.size())
    p->arg->apply(*this);
}

std::string distinctName(std::set<std::string> &S, std::string x)
{
  std::string n = x + "0";
  int i = x.length();
  while(S.find(n) != S.end())
  {
    if (n[i] == '9') { n += '0'; ++i; }
    else ++n[i];
  }
  return n;
}

// Modifying version!
void DistinctVarNames::operator()(Var *p) 
{
  if (GV.boundVars.find(p->value) == GV.boundVars.end())
  {  GV.freeVars.insert(p->value); GV.vars.insert(p->value); }
}

void DistinctVarNames::operator()(QuantifierBlock *p)
{ 
  GetVars GVp(GV);
  
  // Rename variables that have appeared elsewhere!
  for(unsigned int i = 0; i < p->vars.size(); ++i) { 
    Var* v = dynamic_cast<Var*>(&(*(p->vars[i])));
    std::string s = v->value;
    if (GV.vars.find(s) != GV.vars.end())
    {
      std::string sn = distinctName(GV.vars,s);
      RenameVar RV(s,sn);
      p->arg->apply(RV);
      v->value = sn;
    }
  }     
  
  // Add new variables to GV
  for(unsigned int i = 0; i < p->vars.size(); ++i) { 
    Var* v = dynamic_cast<Var*>(&(*(p->vars[i])));
    std::string s = v->value; 
    GV.vars.insert(s);
    GV.boundVars.insert(s);      
  }
  
  p->arg->apply(*this);
}  
}//end namespace tarski
