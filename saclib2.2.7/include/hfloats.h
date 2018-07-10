/*===============================================
                 hfloats.h

  Header file for hardware floats routines.
===============================================*/

/* ----------------------------------------*/
/*            Include                      */
/* ----------------------------------------*/
#include <math.h>

#include "sysdep.h"

#ifdef _SPARC_SOLARIS_
#include <ieeefp.h>
#endif

#ifdef _X86_LINUX_
#include <ieee754.h>
#include <fenv.h>
#endif

#ifdef _MAC_OSX_
#include <fenv.h>
#endif

#include "sysdep.h"

/* ----------------------------------------*/
/*            Hardware Floats              */
/* ----------------------------------------*/

#ifdef _LITTLE_ENDIAN_
typedef struct {
  unsigned mant_l  : 32;
  unsigned mant_h  : 20;
  unsigned exp     : 11;
  unsigned sign    : 1;
  } ieee_rep;
#endif

#ifdef _BIG_ENDIAN_
/* SPARCstations: */
typedef struct {
  unsigned sign    : 1;
  unsigned exp     : 11;
  unsigned mant_h  : 20;
  unsigned mant_l  : 32;
  } ieee_rep;
#endif

typedef union {
  double num;
  ieee_rep rep;
  }  ieee;

#define BIAS 1023
#define MAXEXP 1023
#define MAXEXP1 1024
#define MINEXP -1022
#define MINEXP1 -1023
#define PREC 53
#define PREC1 52

/* ----------------------------------------*/
/*            Rounding modes               */
/* ----------------------------------------*/

/* Sun SPARCs: see also man ieee_flags and /usr/include/ieeefp.h */
#ifdef _SPARC_SOLARIS_
#define rounddown() fpsetround(FP_RM)
#define roundup() fpsetround(FP_RP)
#endif

/* Linux */
#ifdef _X86_LINUX_
#define rounddown() fesetround(FE_DOWNWARD)
#define roundup() fesetround(FE_UPWARD)
#endif

/* MAC OSX */
#ifdef _MAC_OSX_
#define rounddown() fesetround(FE_DOWNWARD)
#define roundup() fesetround(FE_UPWARD)
#endif

/* ----------------------------------------*/
/*            Hardware intervals           */
/* ----------------------------------------*/

typedef struct {
  double left, right;
} interval;

/*
static interval izero = { 0.0, 0.0 };
static interval ione  = { 1.0, 1.0 };
*/
