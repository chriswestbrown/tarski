/*======================================================================
                         PRINTBANNER()

Prints the qepcad banner!
======================================================================*/
#include "qepcad.h"

/* $Format: "static const char* version = \"$ProjectVersion$\";"$ */
static const char* version = "1.71";

/* $Format: "static const char* versdate = \"$ProjectDate$\";"$ */
static const char* versdate = "Tue Jan 30 16:04:07 EST 2018";

void setversline() {
  int j;
  SWRITE("   Version B ");
  SWRITE(version);
  SWRITE(", ");
  SWRITE(versdate);
  /* for(j = 5; j < 16; j++) */
  /*   CWRITE(versdate[j]); */
}

const char* QEPCADBVersion() { return version; }

void PRINTBANNER()
{

Step1: /* Identification. */
       SWRITE("=======================================================\n");
       SWRITE("                Quantifier Elimination                 \n");
       SWRITE("                          in                           \n");
       SWRITE("            Elementary Algebra and Geometry            \n");
       SWRITE("                          by                           \n");
       SWRITE("      Partial Cylindrical Algebraic Decomposition      \n");
       SWRITE("                                                       \n");
       SWRITE("   ");setversline(); SWRITE("\n");
       SWRITE("                                                       \n");
       SWRITE("                          by                           \n");
       SWRITE("                       Hoon Hong                       \n");
       SWRITE("                  (hhong@math.ncsu.edu)                \n");
       SWRITE("                                                       \n");
       SWRITE("With contributions by: Christopher W. Brown, George E. \n");
       SWRITE("Collins, Mark J. Encarnacion, Jeremy R. Johnson        \n");
       SWRITE("Werner Krandick, Richard Liska, Scott McCallum,        \n");
       SWRITE("Nicolas Robidoux, and Stanly Steinberg                 \n");
       SWRITE("=======================================================\n");

Return: /* Prepare for return. */
       return;
}
