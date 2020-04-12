#include "writeForQE.h"
#include "formmanip.h"
#include "normalize.h"

using namespace std;

namespace tarski {
// Factors a conjunction into chunks that are disjoint
// with respect to the given set of variables.
// Returns vector<TFormRef> res with the property that
// 1) each element is an atom or conjunction of atoms
// 2) the conjunction of all elements of res is exactly F
// 3) if i != j, then res[i] and res[j] do not share any variable in S
// The upshot of this is that if Q is a subset of S,
// ex Q [ F ] <==> ex Q[res[0]] /\ ... /\ ex Q[res[k-1]] and
// all Q [ F ] <==> all Q[res[0]] /\ ... /\ all Q[res[k-1]] and
vector<TFormRef> factorDisjointVars(TAndRef F, VarSet S)
{
  vector<TFormRef> res;

  // A[i] is ith conjunct in F
  // Sf[i] is the set of elements of S occuring in A[i]
  vector<TFormRef> A;
  vector<VarSet> Sf;
  for(set<TFormRef>::iterator itr = F->conjuncts.begin(); itr != F->conjuncts.end(); ++itr)
  {
    A.push_back(*itr);
    Sf.push_back(((*itr)->getVars()) & S);
  }

  // Find connected components.
  unsigned int s = 0, e = 0; /// A[s],...,A[e] will grow to include an entire componant
  do {
    unsigned int k = e + 1;
    VarSet curr = Sf[s];
    while(k < A.size())
    {
      if ((Sf[k] & curr).any())
      {
	curr |= Sf[k];
	swap(Sf[e+1],Sf[k]);
	swap(A[e+1],A[k]);
	++e;
	k = e+1;
      }
      else
	++k;
    }
    
    // Set C to the "and" of each conjunct of F in the current componant
    TAndRef C = new TAndObj();
    for(unsigned int i = s; i <= e; ++i)
      C->AND(A[i]);
    res.push_back(C);

    s = e + 1;
    e = s;
  } while(s < A.size());
  return res;  
}

// Returns a set of references to atoms in F that contain
// the unique occurence of an element of S. Requires an AND!!!!
map<TFormRef,VarSet> atomsContainingUniqueVar(TAndRef F, VarSet S)
{
  map<TFormRef,VarSet> res;
  // A[i] is ith conjunct in F
  vector<TFormRef> A;
  for(set<TFormRef>::iterator itr = F->conjuncts.begin(); itr != F->conjuncts.end(); ++itr)
    A.push_back(*itr);

  // Collect info about vars occuring in only one conjunct
  VarKeyedMap<int> M(-1);
  VarSet U; // Used in more than one atom
  VarSet I; // In formula F and set S
  VarSet all; all.flip();
  for(unsigned int i = 0; i < A.size(); ++i)
  {
    VarSet N = ((A[i]->getVars() & S) & (all ^ U));
    if (!N.none())
    {
      I |= N;
      for(VarSet::iterator vitr = N.begin(); vitr != N.end(); ++vitr)
	if (M[*vitr] == -1)
	  M[*vitr] = i;
	else
	{
	  M[*vitr] = -2;
	  U |= *vitr;
	}
    }
  }

  // Construct result to return
  VarSet G = I & U.flip();
  for(VarSet::iterator vitr = G.begin(); vitr != G.end(); ++vitr)
    res[A[M[*vitr]]] |= *vitr;
  return res;
}


/***************************************************************
 *
 ***************************************************************/
string naiveButCompleteWriteForQepcad(TFormRef Fin,
				      TFormRef &introducedAssumptions,
				      bool endWithQuit,
				      bool trackUnsatCore,
				      char solFormType,
				      VarOrderRef ord
				      )
{
  try { return writeForQEPCADB(Fin,introducedAssumptions,endWithQuit,trackUnsatCore,
			       solFormType,ord); }
  catch(TarskiException e) { }

  // Set F to a prenex equivalent to Fin
  TFormRef F = Fin;
  if (!isPrenex(Fin)) F =  makePrenex(Fin);

  // determine which variables from F appear in non-strict inequalities (i.e. =,<=,>=)
  VarSet Vns = appearsInNonStrict(F);
  
  // Set Fqff to the quantifier-free part of F, 
  // Vall to the set of all variables appearing in F (both bound and quantified),
  // and blocks to the vector s.t. blocks[i] is the set of variables from the ith
  // quantifier-block (0 being innermost) and the last element of blocks being the
  // set of free variables.
  TFormRef Fqff;
  VarSet Vall = F->getVars(), Vq;
  vector<VarSet> blocks;
  TQBRef Q = dynamic_cast<TQBObj*>(&*F);
  if (!Q.is_null())
  {
    Fqff = Q->getFormulaPart();
    for(int i = 0; i < Q->numBlocks(); i++)
    {
      VarSet Vnext = Q->blockVars(i);
      blocks.push_back(Vnext);
      Vq = Vq + Vnext;
    } 
  }
  else
    Fqff = F;
  blocks.push_back(Vall - Vq);

  // If Fqff has no variables, we need to handle this separately.
  if (Fqff->getVars().isEmpty())
  {
    Level1 normalizer;
    RawNormalizer R(normalizer);
    R(F);
    int t = R.getRes()->constValue();
    if (t == TRUE) { return "true"; }
    else if (t == FALSE) { return "false"; }
    else throw TarskiException("Error in naiveButCompleteWriteForQepcad!");
  }

  // map s.t. for var v, varToBlockIndex[v] is the index i s.t. v is in blocks[i]
  VarKeyedMap<int> varToBlockIndex;
  for(unsigned int i = 0; i < blocks.size(); i++)
    for(VarSet::iterator itr = blocks[i].begin(); itr != blocks[i].end(); ++itr)
      varToBlockIndex[*itr] = i;
  
  // set blockOrder[i] to the vector of variable from the ith block in brown heuristic order
  vector<VarSet> V = getBrownVariableOrder(Fqff);
  if (!ord.is_null()) // override with manually specified order
  {
    ord->sort(V);
    std::reverse(V.begin(),V.end());
  }
  vector< vector<VarSet> > blockOrders(blocks.size());
  for(unsigned int i = 0; i < V.size(); ++i)
    blockOrders[varToBlockIndex[V[i]]].push_back(V[i]);
  
  PolyManager &PM = *(F->getPolyManagerPtr());
  // cout << "preferred variable order: ";
  // for(int i = 0; i < V.size(); ++i)
  //   cout << PM.getName(V[i]) << ' ';
  // cout << endl;

  // cout << "adapted to quantifier matrix: ";
  // for(int k = blockOrders.size() - 1; k >= 0; --k)
  // {
  //   for(int j = 0; j < blockOrders[k].size(); ++j)
  //     cout << PM.getName(blockOrders[k][j]) << ' ';
  // }
  // cout << endl;

  ostringstream sout;
  sout << "[]\n(";
  for(int k = blockOrders.size() - 1, count = 0; k >= 0; --k)
  {
    for(unsigned int j = 0; j < blockOrders[k].size(); ++j)
      sout << (count++ > 0 ? "," : "") << PM.getName(blockOrders[k][j]);
  }
  sout << ")" << endl;
  sout << (Vall.numElements() - Vq.numElements()) << endl;

  // Set countNonStrict to the number of quantified variables *in the current order* that
  // must be treated as non-strict.  This means that the the first countNonStrict variables
  // must be treated as non-strict, but the remaining variables can be treated as strict.
  bool allStrictSoFar = true;
  int countNonStrict = 0;
  for(int k = 0; k < blockOrders.size() - 1; ++k)
    for(int j = blockOrders[k].size() - 1; j >= 0; --j)
    {
      VarSet x = blockOrders[k][j];
      allStrictSoFar = allStrictSoFar && !(Vns & x).any();
      if (!allStrictSoFar)
	countNonStrict++;
    }

  // Write the quantifier block
  //-- NOTE: I NEED TO FIX THIS!  1. universal requires =/<=/>= only, 2. what the heck
  //         happens if there are alternations!  I think it all breaks down!
  for(int k = blockOrders.size() - 2; k >= 0; --k)
  {
    for(unsigned int j = 0; j < blockOrders[k].size(); ++j, --countNonStrict)
    {
      sout << "(" << (Q->blockType(k) == EXIST
		      ? (countNonStrict > 0 ? "E" : "F")
		      : (countNonStrict > 0 ? "A" : "A") 
		      ) << " " << PM.getName(blockOrders[k][j]) << ")";
    }
  }
  sout << "[";
  PushOutputContext(sout);
  Fqff->write();
  PopOutputContext();
  sout << "].\n";
  //  sout << "cell-choice-bound (SR,LD,HL)\n"; // T E S T ! ! !
  sout << "go\ngo\ngo\n";
  if (solFormType != 0)
    sout << "sol " << solFormType << "\n";


  if (endWithQuit) { sout << "quit" << endl; }
  return sout.str();
}
}//end namespace tarski
