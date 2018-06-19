#include "fern-poly-iter.h"
#include "../../formrepconventions.h"
#include "../../formula/monomialinequality.h"
namespace tarski {

//TODO: Give a FernPolyIter a constant, and return the proper sign!y


string FernPolyIter::shortToRelop[] = {"NOOP", "LTOP", "EQOP", "LEOP", "GTOP", "NEOP", "GEOP", "ALOP"};

/*
  Dives from the current level all the way into a monomial, while tracking signs, and variables
 */
Word FernPolyIter::dive(){
  Word Ap = A;

  //Loop as long as the second is a list and we are not actually looking at the description of a fraction
  while (currDepth < size-1 && (SECOND(Ap) > BETA || SECOND(Ap) < BETA) ){
    
    Word e = FIRST(Ap); // x_r^e - e is the exponent
    mono.push(RED2(Ap)); //Note that we need to look at the reductum of this 

    if (e != 0) { //var <= 0 and the exponent is not even, makae decisions on the sign and mark this level as relevant

      int trueSign = aSigns[currDepth]; //the sign of the current variable under examination

      if (e % 2 == 0)
        trueSign = T_square[trueSign];
      if (sSigns.size() != 0) {
        sSigns.push( T_prod[sSigns.top()][trueSign]); 
      }
      else {
        sSigns.push(trueSign);
      }

      //Save variable and exponent to currVars, mark that this level is relevant
      exponents.push_back(e);
      currVars.push_back(allVars.at(size-(1+currDepth)));
      varLevels.push_back(currDepth);

    }

    currDepth++;
    Ap = SECOND(Ap); //We dive deeper here
  }


  if (size == 0) {
    coefficient = Ap;
    return Ap;
  }
  else {
    //Here, we repeat the loop for the final monomial and the constant. Why? Because the above loop terminates at Second(Ap) <= Beta, which is the case when we've dived all the way and the Second(Ap) is the constant in a term
    Word e = FIRST(Ap);
    mono.push(RED2(Ap)); //Push the reductum in case there are multiple terms with different constants
    if (e != 0) { //var <= 0 and the exponent is not even
      int trueSign = aSigns[currDepth];

      if (e % 2 == 0)
        trueSign = T_square[trueSign]; //Even power - square the sign (LTOP becomes GTOP, etc.)
      if (sSigns.size() != 0) { //If there are previous signs in the stack, incorporate them
        sSigns.push( T_prod[sSigns.top()][trueSign] );
      }
      else {//The stack becomes just this sign
        sSigns.push(trueSign);
      }

      exponents.push_back(e);
      currVars.push_back(allVars[size-(1+currDepth)]);
      varLevels.push_back(currDepth);
    }

    if (currDepth == size-1){
      if (sSigns.size() != 0){
        int currSign = sSigns.top();

        int constSign = signToSigma(ISIGNF(SECOND(Ap)));
        finSign = T_prod[currSign][constSign];  
      }
      else finSign = signToSigma(ISIGNF(SECOND(Ap)));
    }
    coefficient = SECOND(Ap);
    return Ap;
  }
}


/*
  Returns the next monomial. Comes out of the current level by following the stack mono and seeing which value isn't garbage.
  Then dives into that level
  If A = 0, does nothing (AKA, we are at the end)
 */
Word FernPolyIter::next(){
  if (null) return 0;
  if (size == 0) {
    null = true;
    return 0;
  }
  if (A < BETA && A > -BETA) {
    null = true;
    return 0;
  }
  Word Ap = mono.top();
  //Get to the next monomial
  while (Ap == BETA) { //Since we have always pushed RED2 to the stack, we must always pop as well
    //cerr << "next loop \n";
    mono.pop();
    if (mono.size() == 0) { //Ap is beta, but there is nothing left in mono
      null = true;
      return 0;
    }
    if (varLevels.size() != 0 && varLevels.back() == currDepth){ //If this variable was stored and we are going outside it, remove relevant info
      exponents.pop_back();
      currVars.pop_back();
      varLevels.pop_back();
      sSigns.pop();
    }

    currDepth--;
    Ap = mono.top();


  }

  mono.pop();
  if (varLevels.back() == currDepth) {
    exponents.pop_back();
    currVars.pop_back();
    varLevels.pop_back();
    sSigns.pop();
  }
  
  A = Ap;
  return dive();
}

/*
  Called by the constructor to find the first monomial, if one exists. Calls the dive function
 */
Word FernPolyIter::begin(){
  if (A == 0) { null = true; return 0;}
  //A is just a single number
  if (A < BETA && A > -BETA) {
    finSign = signToSigma(ISIGNF(A));
    coefficient = A;
    return A;
  }
  //theres nothing there
  //This loop goes all the way to the innermost element of a list
  //It logs every exponent we hit on the way and puts it in exponents
  //When we stop, we have the first monomial of the polynomial!
  A = dive();
  return A;
}


/*
  Constructor. Initializes the iterator
  A - The polynomial to dissect
  S - The variables in the polynomial
  varSign - the map from variable to integer which gives the varriaible relops
  Note that since this version does not include the # of variables in the inequality,
  we must use std::reverse
  NOTE: If your polynomial is 0, this breaks! 
 */
FernPolyIter::FernPolyIter(Word A, const VarSet &S, const VarKeyedMap<int> &varSign){
  null = false;
  this->A = A;
  int i = 0;
  currDepth = 0;
  coefficient = 0;
  for(VarSet::iterator itr = S.begin(); itr != S.end(); ++itr) {
    allVars.push_back(*itr);
    aSigns.push_back(varSign.get(*itr));

    i++;

  }
  std::reverse(aSigns.begin(), aSigns.end());
  size = i;
  if (size == 0) {
    coefficient = A;
    //cerr << "coefficient is " << A;
    return;
  }
  begin();
}

FernPolyIter::FernPolyIter(IntPolyRef ref, const VarKeyedMap<int> &varSign){
  null = false;
  this->A = ref->getSaclibPoly();
  const VarSet& S = ref->getVars();
  currDepth = 0;
  coefficient = 0;
  int i = 0;
  for(VarSet::iterator itr = S.begin(); itr != S.end(); ++itr) {
    allVars.push_back(*itr);
    aSigns.push_back(varSign.get(*itr));
    i++;

  }
  size = i;
  if (size == 0) {
    coefficient = A;
    //cerr << "coefficient is " << A;
    return;
  }
  else
    begin();
}

  FernPolyIter::FernPolyIter(const FernPolyIter& F) {
    this->size = F.size; 
    this->null = F.null;
    this->A = F.A;
    this->currDepth = F.currDepth;
    this->coefficient = F.coefficient;
    this->aSigns = F.aSigns;
    this->exponents = F.exponents;
    this->currVars = F.currVars; //the variables which appear in the current monomial
    this->varLevels = F. varLevels; //the level of each variable - so we know whether or not to pop the current variable when we go back up the stack
    this->allVars = F.allVars; //All variables in the polynomial
    this->sSigns = F.sSigns; //The sign of the term for the current monomial is the top of this
    this->finSign = F.finSign; //The sign on the monomial taking into account the constant factor
    this->mono = F.mono; //Let's us know what we need to hit next
    this->coefficient = F.coefficient; //The coefficient. Whoohoo!
  }


FernPolyIter FernPolyIter::operator++(int){
  FernPolyIter * tmp = this;
  next();
  return *tmp;
}

FernPolyIter& FernPolyIter::operator++(){
  next();
  return *this;
}

/*
  Less than operator
  Note that this is expensive to use twice to detect equality - an O(n + m) comparison where n and m are the number of variables in n and m
 */
bool FernPolyIter::operator <(const FernPolyIter& F){
  return compareTo(F);
}

/*
  Returns -1 if this is less than F
  Returns 0 if this is equivalent to F
  Returns 1 if this is greater than F

  Returns 1 if this has iterated through all monomials
  Returns -1 if F has iterated through all monomials
 */
int FernPolyIter::compareTo(FernPolyIter F){
  if (null && F.isNull()) return 0;
  else if (null) return 1;
  else if (F.isNull()) return -1;
  vector<Variable> vars1 = currVars;
  vector<Variable> vars2 = F.getVars();
  vector<short> exp1 = exponents;
  vector<short> exp2 = F.getExponents();
  vector<Variable>::iterator vIter1 = vars1.begin();
  vector<Variable>::iterator vIter2 = vars2.begin();
  vector<short>::iterator sIter1 = exp1.begin();
  vector<short>::iterator sIter2 = exp2.begin();

  while (vIter1 != vars1.end() && vIter2 != vars2.end()) {
    if (*vIter1 < *vIter2) {
      //cerr << "FPI: 0\n";
      return 1;
    }
    else if (*vIter2 < *vIter1) {
      //cerr << "FPI: 1\n";
      return -1;
    }
    else if (*sIter1 < *sIter2) {
      //cerr << "FPI: 2\n";
      return -1;
    }
    else if (*sIter2 < *sIter1) {
      //cerr << "FPI: 3\n";
      return 1;
    }
    ++vIter1;
    ++vIter2;
    ++sIter1;
    ++sIter2;
  }
  if (vIter1 == vars1.end() && vIter2 == vars2.end()) {
    //cerr << "FPI: 4\n";
    return 0;
  }
  else if (vIter1 == vars1.end()) {
    //cerr << "FPI: 5\n";
    return 1;
  }
  else {
    //cerr << "FPI: 6\n";
    return -1;
  }
}

int FernPolyIter::compare(FernPolyIter F1, FernPolyIter F2) {
  return F1.compareTo(F2);
}


/*
  Exactly what it says on the tin.
 */
void FernPolyIter::write(PolyManager&  PM){
  cout << endl;
  vector<short>::iterator itc = exponents.begin();
  for (vector<Variable>::iterator it = currVars.begin(); it != currVars.end(); ++it){
    cout << PM.getName(*it) << "^" << *itc << "*";

    ++itc;
  }
  cout << coefficient << ": " << shortToRelop[finSign] << endl;
}


/*
  Exactly what it says on the tin. Uses made up names
*/
void FernPolyIter::write(){
  cout << endl;
  int i =0;
  vector<short>::iterator itc = exponents.begin();
  for (vector<Variable>::iterator it = currVars.begin(); it != currVars.end(); ++it){
    cout << i << "^" << *itc << " * ";
    i++;
    ++itc;
  }
  cout << coefficient << ": " << shortToRelop[finSign] << endl;
}

void FernPolyIter::writeAll(PolyManager& PM){
  cout << endl;
  vector<short>::iterator itc = aSigns.begin();
  for (vector<Variable>::iterator it = allVars.begin(); it != allVars.end(); ++it){
    cout << PM.getName(*it) << "^" << *itc << "*";

    ++itc;
  }
}


/*
  Returns the sign of the term (monomial * coefficient)
 */
short FernPolyIter::getTermSign() const{
  return finSign;
}

/*
  Returns the sign of the monomial
 */
 short FernPolyIter::getMonoSign() const{
   if (sSigns.size() == 0) {
     int sgn = ISIGNF(coefficient);
     if (sgn == 0) return EQOP;
     else return GTOP;
   }
   return sSigns.top();
}

/*
  Exactly what it says on the tin
 */
 int FernPolyIter::getCoefficient() const{
  return coefficient;
}


const vector<Variable>& FernPolyIter::getVars() const{
  return currVars;
}

 const vector<Variable>& FernPolyIter::getAllVars() const{
  return allVars;
}

 vector<short> FernPolyIter::getExponents() const{
  return exponents;
}


string FernPolyIter::numToRelop(short num) {
  if (num >= 8 || num < 0) {
    throw new TarskiException("Index out of bounds - invalid relop in numToRelop! ");
      }
  return shortToRelop[num];
}


bool FernPolyIter::isNull(){
  return null;
}

}//end namespace tarski
