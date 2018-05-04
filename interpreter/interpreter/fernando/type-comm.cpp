#include "type-comm.h"
#include "../formrepconventions.h"


namespace tarski {

  /*
    Some notes to self
    - TFORM objects are in formula.h in the formula folder

    Suggestion to Dr. Brown: Make And and Or extend one virtual superclass.
    Otherwise, if we don't care about the Andness or Orness of the data,
    and need to access the atoms contained within, we have to write duplicate
    code to handle both cases
    Do the same for ATOM and EXTATOM as well
  */

  SRef TypeComm::execute(SRef input, std::vector<SRef> &args)
  {
    // must have arguments
    int N = args.size();
    if (N < 1) { return new ErrObj("type-comm requires 1 argument."); }

    //last argument must be the formula to evaluate
    TarRef tarRef = args[N-1]->tar();
    if (tarRef.is_null()) { return new StrObj("type-comm argument not a Tarski Formula."); }
    TFormRef tRef = tarRef->val;

    //See if there is a switch option for programming mode
    bool programMode = false;
    for(int i = 0; i < N-1; i++)
      {
        SymRef opt = args[i]->sym();
        if (!opt.is_null() && opt->val == "program")
          programMode = true;
        else
          return new ErrObj("smt-load optional argument '" + args[i]->toStr() +"' not understood.");
      }

    std::string toReturn = "";

    if (programMode) {
      if (hasQuantifiers(tRef))
        toReturn += "( Q1";
      else
        toReturn += "( Q0";
      if (isConjunction(tRef))
        toReturn += " C1";
      else
        toReturn += " C0";

      if (isDisjunction(tRef))
        toReturn += " D1";
      else
        toReturn += " D0";


      if (isStrict(tRef) == STRICT)
        toReturn += " S1";
      else
        toReturn += " S0";

      if (isNonStrict(tRef) == NONSTRICT)
        toReturn += " N1";
      else
        toReturn += " N0";

      if (hasMultiFactor(tRef))
        toReturn += " M1 )";
      else
        toReturn += " M0 )";


      return new StrObj(toReturn);
    }

    else{

      if (hasQuantifiers(tRef))
        toReturn += "Contains Quantifiers, ";
      else
        toReturn += "Does not contain Quantifiers, ";

      if (isConjunction(tRef))
        toReturn += "Is a Conjunction, ";
      else if (isDisjunction(tRef))
        toReturn += "Is a Disjunction, ";

      else
        toReturn += "Is neither a pure Conjunction nor pure Disjunction, ";

      if (isStrict(tRef) == STRICT)
        toReturn += "Is strict, ";
      else if (isNonStrict(tRef) == NONSTRICT)
        toReturn += "Is non-strict, ";
      else
        toReturn += "Contains both strict and non-strict operators, ";

      if (hasMultiFactor(tRef))
        toReturn += "Contains multi-factor atoms, ";
      else
        toReturn += "Does not contain multi-factor atoms, ";


      return new StrObj(toReturn);
    }
  }

  /*
    Returns true if the formula contains a Quantifier, and false otherwise.
  */
  bool TypeComm::hasQuantifiers(TFormRef F){
    switch (F->getTFType())
      {
      case TF_QB:{ return true; break;}
      case TF_ATOM: {return false; break;}
      case TF_EXTATOM:{ return false; break;}
      case TF_CONST:{ return false; break;}
      case TF_AND:{
        TAndRef C = asa<TAndObj>(F);
        for(std::set<TFormRef, ConjunctOrder>::iterator it = C->begin(); it != C->end(); ++it ) {
          if (TypeComm::hasQuantifiers(*it)) return true;
        }
        return false;
        break;
      }

      case TF_OR: {
        TOrRef C = asa<TOrObj>(F);
        for(std::set<TFormRef, ConjunctOrder>::iterator it = C->begin(); it != C->end(); ++it ) {
          if (TypeComm::hasQuantifiers(*it)) return true;
        }
        return false;
        break;
      }
        return false;
        break;
      }
    return false;
  }
  /*
    returns true if there are multifactor objects, and false otherwise
  */
  bool TypeComm::hasMultiFactor(TFormRef F){

    switch (F->getTFType())
      {
        //Pick it apart and see whats inside
      case TF_AND: {
        TAndRef C = asa<TAndObj>(F);
        for(std::set<TFormRef, ConjunctOrder>::iterator it = C->begin();
            it != C->end(); ++it )
          {
            bool res = TypeComm::hasMultiFactor(*it);
            if (res)
              return true;
          }
        return false;
      }
        //Pick it apart and see whats inside
      case TF_OR: {
        TOrRef C = asa<TOrObj>(F);
        for(std::set<TFormRef, ConjunctOrder>::iterator it = C->begin();
            it != C->end(); ++it )
          {
            bool res = TypeComm::hasMultiFactor(*it);
            if (res)
              return true;
          }
        return false;

      }
      case TF_EXTATOM: {
        TExtAtomRef E = asa<TExtAtomObj>(F);
        FactRef F = E->F;
        if (F->numFactors() > 1 || (F->numFactors() == 1 && F->factorBegin()->second > 1) )
          return true;
        else
          return false;
      }
      case TF_ATOM: {
        TAtomRef A = asa<TAtomObj>(F);
        FactRef F = A->F;
        if (F->numFactors() > 1 || (F->numFactors() == 1 && F->factorBegin()->second > 1))
          return true;
        else
          return false;
      }

      case TF_CONST: {
        return false;
      }
      case TF_QB: {
        TQBRef Q = asa<TQBObj>(F);
        TFormRef T = Q->getFormulaPart();
        return TypeComm::isStrict(T);

      }
        return false;
      }
    return false;
  }

  /*
    Takes in a TFormRef and determine if it is strict, nonstrict, or mixed
  */
  short TypeComm::isStrict(TFormRef F){

    bool atLeastOneStrict = false;

    switch (F->getTFType()) {
      //Pick it apart and see whats inside
    case TF_AND: {
      TAndRef C = asa<TAndObj>(F);
      for(std::set<TFormRef, ConjunctOrder>::iterator it = C->begin();
          it != C->end(); ++it ) {
        short res = TypeComm::isStrict(*it);
        if (res == STRICT) { atLeastOneStrict = true; continue; }
        if (res == NONSTRICT) return -1; //Fail
        if (res == NOT_APP) continue;
      }
      if (atLeastOneStrict) return STRICT;
      else return NOT_APP;
      break;
    }
      //Pick it apart and see whats inside
    case TF_OR: {
      TOrRef C = asa<TOrObj>(F);
      for(std::set<TFormRef, ConjunctOrder>::iterator it = C->begin();
          it != C->end(); ++it )
        {
          short res = TypeComm::isStrict(*it);
          if (res == STRICT) { atLeastOneStrict = true; continue; }
          if (res == NONSTRICT) return -1; //Fail
          if (res == NOT_APP) continue;
        }
      if (atLeastOneStrict) return STRICT;
      else return NOT_APP;
      break;
    }
    case TF_EXTATOM: {
      TExtAtomRef E = asa<TExtAtomObj>(F);
      int nuVal = E->relop;
      short res = TypeComm::determineSignStrict(nuVal);
      if (res == NOT_APP) return NOT_APP;
      if (res == STRICT) return STRICT;
      if (res == NONSTRICT) return -1; //fail
      break;
    }
    case TF_ATOM: {
      TAtomRef A = asa<TAtomObj>(F);
      int nuVal = A->relop;
      short res = TypeComm::determineSignStrict(nuVal);
      if (res == NOT_APP) return NOT_APP;
      if (res == STRICT) return STRICT;
      if (res == NONSTRICT) return -1; //fail
      break;
    }
    case TF_CONST: {return NOT_APP;}
    case TF_QB: {
      TQBRef Q = asa<TQBObj>(F);
      TFormRef T = Q->getFormulaPart();
      return TypeComm::isStrict(T);

    }
      return NOT_APP;
    }
    return NOT_APP;
  }

  /*
    Takes in a TFormRef and determine if it is  nonstrict
  */
  short TypeComm::isNonStrict(TFormRef F){

    bool atLeastOne = false;

    switch (F->getTFType()) {
      //Pick it apart and see whats inside
    case TF_AND: {
      TAndRef C = asa<TAndObj>(F);
      for(std::set<TFormRef, ConjunctOrder>::iterator it = C->begin();
          it != C->end(); ++it ) {
        short res = TypeComm::isStrict(*it);
        if (res == STRICT) { return -1; } // Fail
        if (res == NONSTRICT) {atLeastOne = true; continue; }
        if (res == NOT_APP) continue;
      }
      if (atLeastOne) return NONSTRICT;
      else return NOT_APP;
      break;
    }
      //Pick it apart and see whats inside
    case TF_OR: {
      TOrRef C = asa<TOrObj>(F);
      for(std::set<TFormRef, ConjunctOrder>::iterator it = C->begin();
          it != C->end(); ++it )      {
        short res = TypeComm::isStrict(*it);
        if (res == STRICT) { return -1;  } // Fail
        if (res == NONSTRICT) {atLeastOne = true; continue; }
        if (res == NOT_APP) continue;
      }
      if (atLeastOne) return NONSTRICT;
      else return NOT_APP;
      break;
    }
    case TF_EXTATOM: {
      TExtAtomRef E = asa<TExtAtomObj>(F);
      int nuVal = E->relop;
      short res = TypeComm::determineSignStrict(nuVal);
      if (res == NOT_APP) return NOT_APP;
      if (res == STRICT) return -1 ; //fail
      if (res == NONSTRICT) return NONSTRICT; 
      break;
    }
    case TF_ATOM: {
      TAtomRef A = asa<TAtomObj>(F);
      int nuVal = A->relop;
      short res = TypeComm::determineSignStrict(nuVal);
      if (res == NOT_APP) return NOT_APP;
      if (res == STRICT) return -1; //fail
      if (res == NONSTRICT) return NONSTRICT; 
      break;
    }
    case TF_CONST: {return NOT_APP;}
    case TF_QB: {
      TQBRef Q = asa<TQBObj>(F);
      TFormRef T = Q->getFormulaPart();
      return TypeComm::isNonStrict(T);

    }
      return NOT_APP;
    }
    return NOT_APP;
  }

  /*
    Determines whether a sign is of the strict type, nonstrict type, or is of a type that doesnt matter.
    Strict means LTOP, GTOP
    Nonstrict means LEOP, GEOP
    "Doesn't matter" means NOOP, EQOP, ALOP, NEOP.

  */
  short TypeComm::determineSignStrict(short input) {
    if (input == LTOP  || input == GTOP )// < or >
      return STRICT;
    else if (input == LEOP || input == GEOP)// <= or >=
      return NONSTRICT;
    else //ALOP, NOOP, 0, etc.
      return NOT_APP; //Does not apply
  }

  bool TypeComm::isConjunction(TFormRef F)
  {
    switch(F->getTFType())
      {
      case TF_ATOM: return true; break;
      case TF_EXTATOM: return true; break;
        /*For our purposes, it is useful to think of the statement TRUE by itself as a conjunction, and a statement FALSE by itself as a disjunction */
      case TF_CONST: return F->constValue() == TRUE ? true : false; break;
      case TF_OR: return false; break;
      case TF_QB: return false; break;
      case TF_AND:
        {
          TAndRef C = asa<TAndObj>(F);
          for(std::set<TFormRef, ConjunctOrder>::iterator it = C->begin(); it != C->end(); ++it )
            {
              if (!isConjunction(*it)) return false;
            }
          return true;
          break;
        }
        return true;
      }
  }

  bool TypeComm::isDisjunction(TFormRef F){
    switch (F->getTFType())
      {
      case TF_ATOM: return false; break;
      case TF_EXTATOM: return false; break;
        /*For our purposes, it is useful to think of the statement TRUE by itself as a conjunction, and a statement FALSE by itself as a disjunction */
      case TF_CONST: return F->constValue() == TRUE ? false : true; break;
      case TF_AND:
        {
          TAndRef C = asa<TAndObj>(F);
          for(std::set<TFormRef, ConjunctOrder>::iterator it = C->begin(); it != C->end(); ++it )
            {
              if (isDisjunction(*it)) return true;
            }
          return false;
          break;
        }
      case TF_QB: return false; break;
      case TF_OR: return true; break;
      }
    return true;
  }


  /* A SKELETON FOR MAKING NEW FUNCTIONS

     bool TypeComm::nuCommand(TFormRef F){
     switch (F->getTFType())
     {
     case TF_ATOM: {return true; break;}
     case TF_EXTATOM:{ return true; break;}
     case TF_CONST:{ return true; break;}
     case TF_AND:{
     TAndRef C = asa<TAndObj>(F);
     for(std::set<TFormRef, ConjunctOrder>::iterator it = C->begin(); it != C->end(); ++it ) {
     //do something
     }
     return true;
     break;
     }
     case TF_QB:{ return true; break;}
     case TF_OR: {
     TOrRef C = asa<TOrObj>(F);
     for(std::set<TFormRef, ConjunctOrder>::iterator it = C->begin(); it != C->end(); ++it ) {
     //do something
     }
     return true;
     break;
     }
     break;
     }
     return true;
     }
  */
}
