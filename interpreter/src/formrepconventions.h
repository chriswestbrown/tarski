#ifndef _FORMREPCONVENTIONS_
#define _FORMREPCONVENTIONS_
#include <string>
using namespace std;

namespace tarski {
// Truth values: UNDET means "undetermined"
const int TRUE  = 1;
const int FALSE = 0;
const int UNDET = 2;

const int ZERO = 0;

const int NOOP = 0;
const int LTOP = 1;
const int EQOP = 2;
const int LEOP = 3;
const int GTOP = 4;
const int NEOP = 5;
const int GEOP = 6;
const int ALOP = 7;

const int UNIVER =  20;
const int EXIST  =  21;

const int ANDOP   =  11;
const int OROP    =  12;
const int NOTOP   =  13;
const int RIGHTOP =  14;
const int LEFTOP  =  15;
const int EQUIOP  =  16;

inline int logicalNegation(int value) 
{ return value == TRUE ? FALSE : (value == FALSE ? TRUE : value); }


inline int quantifierNegation(int value) { return (value == UNIVER) ? EXIST : UNIVER; }

// Reverses relop ... e.g. for multipying both sides of an inequality by -1
inline int reverseRelop(int r) { static int T[] = {0,4,2,6,1,5,3,7}; return T[r]; }
inline int negateRelop(int r) { return ALOP ^ r; }

inline int relopIsNonStrict(int r) { return EQOP & r; }

/*********************************************************************
 ** These tables do sign deductions. e.g. if we know "p sigma_1 0"
 ** and "q sigma_2 0", then we know "p*q T_prod[sigma_1][sigma_2] 0".
 *********************************************************************/
static
//___________________NOOP_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
short T_square[8] = {NOOP,GTOP,EQOP,GEOP,GTOP,GTOP,GEOP,GEOP};
static
short T_prod[8][8] = {
  //______|NOOP_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
  /*NOOP*/{NOOP,NOOP,NOOP,NOOP,NOOP,NOOP,NOOP,NOOP},
  /*LTOP*/{NOOP,GTOP,EQOP,GEOP,LTOP,NEOP,LEOP,ALOP},
  /*EQOP*/{NOOP,EQOP,EQOP,EQOP,EQOP,EQOP,EQOP,EQOP},
  /*LEOP*/{NOOP,GEOP,EQOP,GEOP,LEOP,ALOP,LEOP,ALOP},
  /*GTOP*/{NOOP,LTOP,EQOP,LEOP,GTOP,NEOP,GEOP,ALOP},
  /*NEOP*/{NOOP,NEOP,EQOP,ALOP,NEOP,NEOP,ALOP,ALOP},
  /*GEOP*/{NOOP,LEOP,EQOP,LEOP,GEOP,ALOP,GEOP,ALOP},
  /*ALOP*/{NOOP,ALOP,EQOP,ALOP,ALOP,ALOP,ALOP,ALOP}
};
static
short T_sum[8][8] = {
  //______|NOOP_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
  /*NOOP*/{NOOP,NOOP,NOOP,NOOP,NOOP,NOOP,NOOP,NOOP},
  /*LTOP*/{NOOP,LTOP,LTOP,LTOP,ALOP,ALOP,ALOP,ALOP},
  /*EQOP*/{NOOP,LTOP,EQOP,LEOP,GTOP,NEOP,GEOP,ALOP},
  /*LEOP*/{NOOP,LTOP,LEOP,LEOP,ALOP,ALOP,ALOP,ALOP},
  /*GTOP*/{NOOP,ALOP,GTOP,ALOP,GTOP,ALOP,GTOP,ALOP},
  /*NEOP*/{NOOP,ALOP,NEOP,ALOP,ALOP,ALOP,ALOP,ALOP},
  /*GEOP*/{NOOP,ALOP,GEOP,ALOP,GTOP,ALOP,GEOP,ALOP},
  /*ALOP*/{NOOP,ALOP,ALOP,ALOP,ALOP,ALOP,ALOP,ALOP}
};

inline int signToSigma(int s) { return s < 0 ? LTOP : (s == 0 ? EQOP : GTOP); }


// Array gives strings for each of the relational operators
static const char *relopStrings[8] = { "ERR", "<", "=", "<=", ">", "/=", ">=", "??" };
inline string relopString(int r) { return relopStrings[r]; }

 static const char *logopStrings[17] = { "ERR","ERR","ERR","ERR","ERR","ERR","ERR","ERR","ERR","ERR","ERR",
				  "/\\", "\\/", "~", "==>", "<==", "<==>" };

// returns true if relop a is consistent with relop b, i.e. x a 0 ==> x b 0
inline bool consistentWith(int a, int b)
{
static
short consistencyTable[8][8] = {
  //______|ZERO_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
  /*ZERO*/{ 0  , 0,   0,   0,   0,   0,   0,   0 },
  /*LTOP*/{ 0,   1,   0,   1,   0,   1,   0,   1 },
  /*EQOP*/{ 0,   0,   1,   1,   0,   0,   1,   1 },
  /*LEOP*/{ 0,   0,   0,   1,   0,   0,   0,   1 },
  /*GTOP*/{ 0,   0,   0,   0,   1,   1,   1,   1 },
  /*NEOP*/{ 0,   0,   0,   0,   0,   1,   0,   1 },
  /*GEOP*/{ 0,   0,   0,   0,   0,   0,   1,   1 },
  /*ALOP*/{ 0,   0,   0,   0,   0,   0,   0,   1 }
};
 return consistencyTable[a][b]; 
};

// input: s \in {-1,0,1}, simga \in {ZERO_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP},
// returns true iff "s sigma 0" is true
inline bool signSatSigma(int s, int sigma) { return consistentWith(signToSigma(s),sigma); }

// returns true if relop a is strictly stronger than relop b
inline bool strictlyStronger(int a, int b)
{
static
const short strictlyStrongerTable[8][8] = {
  //______|ZERO_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
  /*ZERO*/{ 0  , 1,   1,   1,   1,   1,   1,   1 },
  /*LTOP*/{ 0,   0,   0,   1,   0,   1,   0,   1 },
  /*EQOP*/{ 0,   0,   0,   1,   0,   0,   1,   1 },
  /*LEOP*/{ 0,   0,   0,   0,   0,   0,   0,   1 },
  /*GTOP*/{ 0,   0,   0,   0,   0,   1,   1,   1 },
  /*NEOP*/{ 0,   0,   0,   0,   0,   0,   0,   1 },
  /*GEOP*/{ 0,   0,   0,   0,   0,   0,   0,   1 },
  /*ALOP*/{ 0,   0,   0,   0,   0,   0,   0,   0 }
};
 return strictlyStrongerTable[a][b]; 
}

// returns true if relop a is strictly stronger than relop b
// always returns false for NOOP
inline bool fernStrictlyStronger(int a, int b)
{
  static
    const short strictlyStrongerTable[8][8] = {
    //______|ZERO_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
    /*ZERO*/{ 0,   0,   0,   0,   0,   0,   0,   0 },
    /*LTOP*/{ 1,   0,   0,   1,   0,   1,   0,   1 },
    /*EQOP*/{ 1,   0,   0,   1,   0,   0,   1,   1 },
    /*LEOP*/{ 1,   0,   0,   0,   0,   0,   0,   1 },
    /*GTOP*/{ 1,   0,   0,   0,   0,   1,   1,   1 },
    /*NEOP*/{ 1,   0,   0,   0,   0,   0,   0,   1 },
    /*GEOP*/{ 1,   0,   0,   0,   0,   0,   0,   1 },
    /*ALOP*/{ 1,   0,   0,   0,   0,   0,   0,   0 }
  };
  return strictlyStrongerTable[a][b]; 
}

// returns true if relop a is as strong or stronger than relop b
inline bool sameOrStronger(int a, int b) { return a == b || strictlyStronger(a,b); }

 inline const std::string numToRelop(short num) {
   static const std::string
    shortToRelop[] = {"NOOP", "LTOP", "EQOP", "LEOP", "GTOP", "NEOP", "GEOP", "ALOP"};

  return shortToRelop[num];
}

static const short T_combine[8][8] = {
  //______|ZERO_LTOP_EQOP_LEOP_GTOP_NEOP_GEOP_ALOP
  /*ZERO*/{ZERO,LTOP,EQOP,LEOP,GTOP,NEOP,GEOP,ALOP},
  /*LTOP*/{LTOP,LTOP,ZERO,LTOP,ZERO,LTOP,ZERO,LTOP},
  /*EQOP*/{EQOP,ZERO,EQOP,EQOP,ZERO,ZERO,EQOP,EQOP},
  /*LEOP*/{LEOP,LTOP,EQOP,ZERO,ZERO,ZERO,ZERO,LEOP},
  /*GTOP*/{GTOP,ZERO,ZERO,ZERO,GTOP,GTOP,GTOP,GTOP},
  /*NEOP*/{NEOP,LTOP,ZERO,ZERO,GTOP,NEOP,ZERO,NEOP},
  /*GEOP*/{GEOP,ZERO,EQOP,ZERO,GTOP,ZERO,GEOP,GEOP},
  /*ALOP*/{ALOP,LTOP,EQOP,LEOP,GTOP,NEOP,GEOP,ALOP}
};

inline short sigma_combine(int a, int b)
{
  return T_combine[a][b];
}

} // end namespace tarski
#endif



