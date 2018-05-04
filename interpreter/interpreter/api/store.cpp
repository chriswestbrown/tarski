#include "store.h"

using namespace tarskiapi;

static const char *relopStringsSMTLIB[8] = { "ERR", "<", "=", "<=", ">", "!=", ">=", "??" };
string Store::relopStringSMTLIB(int r) { return relopStringsSMTLIB[r]; }
int Store::SMTLIBRelopInt(const string& str)
{
  int sigma = 0; while(sigma < 7 && relopStringSMTLIB(++sigma) != str);
  if (sigma == 7) { throw tarski::TarskiException("unkown relop: " + str); }
  return sigma;
}

void foo(tarski::Normalizer &NORM, tarski::PolyManager &PM, const string& s)
{
  using namespace tarski;
  istringstream sin(s);
  ostringstream sout;
  readSMTRetTarskiString(sin,sout);
  TFormRef F = processExpFormulaClearDenominators(sout.str(),&PM);
  TFormRef G = NORM.normalize(F);
  writeSMTLIB(G,cout);
  cout << endl;
}

tarski::TFormRef readAndNormalize(tarski::Normalizer &NORM, tarski::PolyManager &PM, const string& s)
{
  using namespace tarski;
  istringstream sin(s);
  ostringstream sout;
  readSMTRetTarskiString(sin,sout);
  TFormRef F = processExpFormulaClearDenominators(sout.str(),&PM);
  TFormRef G = NORM.normalize(F);
  return G;
}


std::string Store::getString(const std::vector<int> &q)
{
  using namespace tarski;
  TAndRef C = new TAndObj();
  for(int i = 0; i < q.size(); i+=2)
    C->AND(getAtom(q[i],q[i+1]));
  ostringstream sout;
  PushOutputContext(sout);
  C->write();
  PopOutputContext();
  return sout.str();
}

int Store::isSAT(const std::vector<int> &q, string &witness)
{
  using namespace tarski;
  TAndRef C = new TAndObj();
  for(int i = 0; i < q.size(); i+=2)
    C->AND(getAtom(q[i],q[i+1]));

  TFormRef F;
  VarSet FVars = getFreeVars(C);
  if (FVars.none()) 
    F = C;
  else
    F = new TQBObj(FVars, EXIST, C, getPolyManagerPtr());
  
  QepcadConnection qconn;
  TFormRef res;
  res = qconn.basicQepcadCall(F,TFormRef(new TConstObj(TRUE)),true);
  witness = qconn.samplePointPrettyPrint(getPolyManagerPtr());
  return res->constValue();
}

int Store::isSATUC(const std::vector<int> &q, std::string &witness, std::vector<int> &unsatCore)
{
  using namespace tarski;
  // Set up M as a mirror to the conjunction, that way I can get TFormRef's by index
  vector<TFormRef> M;
  TAndRef C = new TAndObj();
  for(int i = 0; i < q.size(); i+=2)
  {
    TFormRef a = getAtom(q[i],q[i+1]);
    M.push_back(a);
    C->AND(a);
  }
  TFormRef F;
  VarSet FVars = getFreeVars(C);
  if (FVars.none()) 
    F = C;
  else
    F = new TQBObj(FVars, EXIST, C, getPolyManagerPtr());
  
  QepcadConnection qconn;
  TFormRef res;
  res = qconn.basicQepcadCall(F,TFormRef(new TConstObj(TRUE)),true);
  int out = res->constValue();
  witness = qconn.samplePointPrettyPrint(getPolyManagerPtr());
  if (out == 0)
  {
    set<TFormRef> S; // S will be the set of atoms in the unsat core    
    vector<int> T = qconn.getUnsatCore();
    for(int i = 0; i < T.size(); i++)
      S.insert(M[T[i]]);

    vector<int> properlyIndexedCore;
    for(int i = 0; i < M.size(); i++)
      if (S.find(M[i]) != S.end())
	properlyIndexedCore.push_back(i);
    unsatCore = properlyIndexedCore;
  }
  return out;
}


std::vector<int> Store::registerAtom(const string &smtlibPoly, int sigma)
{
  using namespace tarski;
  vector<int> res;
  ostringstream sout;
  sout << "(assert (" << relopStringSMTLIB(sigma) << " " << smtlibPoly << " 0))";
  //  cout << sout.str() << endl;
  TFormRef F = readAndNormalize(*NORM,*PM,sout.str());
  int t = F->constValue();
  if (t == TRUE || t == FALSE) { res.push_back(t); return res; }
  TAndRef c = new TAndObj(); c->AND(F);
		
  for(TAndObj::conjunct_iterator itr = c->conjuncts.begin(); itr != c->conjuncts.end(); itr++)
  {
    TFormRef next = *itr;
    TAtomRef A = TFasa<TAtomObj>(next);
    if (A.is_null()) { throw TarskiException("Expected Atom in registerAtom!"); }
    int idx = FactorTpIdx[A->getFactors()];
    if (idx == 0) {
      idx = IdxToFactor.size();
      IdxToFactor.push_back(A->getFactors());
      FactorTpIdx[A->getFactors()] = idx;
    }
    res.push_back(idx); res.push_back(A->getRelop());
  }
  
  return res;
}

tarski::TAtomRef Store::getAtom(int id, int sigma)
{
  using namespace tarski;
  FactRef F = IdxToFactor[id];
  TAtomRef A = new TAtomObj(F,sigma);
  return A;
}
