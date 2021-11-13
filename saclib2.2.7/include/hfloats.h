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

#if defined(__MSYS__) || defined(__MINGW32__)
#include <fenv.h>
#endif

#ifdef _EMCC2_
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

/* Windows */
#if defined(__MSYS__) || defined(__MINGW32__)
#define rounddown() fesetround(FE_DOWNWARD)
#define roundup() fesetround(FE_UPWARD)
#endif

/* Emscripten */
#ifdef _EMCC2_

// See https://github.com/emscripten-core/emscripten/issues/13678
#define FE_INVALID    1
#define __FE_DENORM   2
#define FE_DIVBYZERO  4
#define FE_OVERFLOW   8
#define FE_UNDERFLOW  16
#define FE_INEXACT    32
// #define FE_ALL_EXCEPT 63
#define FE_TONEAREST  0
#define FE_DOWNWARD   0x400
#define FE_UPWARD     0x800
#define FE_TOWARDZERO 0xc00

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
