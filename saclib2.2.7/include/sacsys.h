/******************************************************************\
  FILE
    sacsys.h
 
  Header file containing system dependent definitions.
\******************************************************************/

#ifndef SACSYS_H
#define SACSYS_H

/* SACLIB 'Word' is a C 'int'. */
typedef int     Word;

/* Words are 32 bits. */
#define BETALENGTH 29 /* BETA is 29 bits */
#define BETA    0x20000000 
#define BETA1   0x1fffffff   /* BETA - 1 */
#define BETA2   0x10000000   /* BETA / 2 */

/* Default values of several global variables. */
#define NU_       1000000  /* 4 MB list space - 500000 list cells. */
#define NUp_      20000
#define NSPRIME_  500
#define NMPRIME_  2000
#define NLPRIME_  2000
#define NPFDS_    5
#define RHO_      10
#define NPTR1_    2001

/* LARGE = dimension of fixed size arrays used in IUPBEIN and
 *         IUPBRESN to hold the result of the polynomial evaluation.
 * SMALL = dimension of fixed size array used in IUPBEIN and
 *         IUPBRESN to hold the evaluation point.
 */
#define LARGE 2000
#define SMALL 50


/* Does the compiler support function prototypes? */
#ifdef __STDC__
#define P__(A)  A
#else
#define P__(A)  ()
#endif

/* Indicate that a function does not return. */
#ifndef __noreturn
# ifdef __GNUC__
#  define __noreturn __attribute__ ((noreturn))
# else
#  define __noreturn
# endif
#endif



#endif
