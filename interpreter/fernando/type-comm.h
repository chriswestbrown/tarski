#ifndef _TYPE_COMM_H_
#define _TYPE_COMM_H_

#include "../shell/einterpreter.h"


namespace tarski {
  const int STRICT =    0;
  const int NONSTRICT = 1;
  const int MIXED =     2;
  const int NOT_APP =  10;


  /*
    TODO: DETECT MULTIFACTOR ATOMS FROM FACTREFS
  */
  class TypeComm : public EICommand
  {
  private:
    bool isConjunction(TFormRef F);
    bool isDisjunction(TFormRef F);
    short isStrict(TFormRef F);
    short isNonStrict(TFormRef F);
    bool hasQuantifiers(TFormRef F);
    bool hasMultiFactor(TFormRef F);
    short determineSignStrict(short input);
  

  public:
  TypeComm(NewEInterpreter* ptr) : EICommand(ptr) { }
    SRef execute(SRef input, std::vector<SRef> &args);
    std::string testArgs(std::vector<SRef> &args) { return ""; }
    std::string doc() { return "(type-comm [switch] <formula>). Used to evaluate tarski formulas based on characteristics such as pure conjunction, pure disjunction, purely strict, purely nonstrict, and if it contains multifactor atoms. Enter the option ['program] to make the output easier to parse programatically."; }
    std::string usage() { return "(type-comm <arg>)"; }
    std::string name() { return "type-comm"; }
  };

}
#endif
