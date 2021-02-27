/*======================================================================
                         PRINTCAD2DBANNER()
======================================================================*/
#include "cad2d.h"

/* $Format: "static const char* version = \"$ProjectVersion$\";"$ */
static const char* version = "1.74";

/* $Format: "static const char* versdate = \"$ProjectDate$\";"$ */
static const char* versdate = "Thu Feb 25 14:48:53 EST 2021";

static void setversline() {
  int j;
  SWRITE("QEPCAD Version B ");
  SWRITE(version);
  SWRITE(", ");
  for(j = 4; j < 10; j++)
    CWRITE(versdate[j]);
  while(versdate[++j] != '\0');
  while(versdate[--j] != ' ');
  SWRITE(versdate + j);
}

void PRINTCAD2DBANNER()
{
Step1: /* Identification. */
       SWRITE("=======================================================\n");
       SWRITE("     CAD2D - A Program for producing CADs of R^2       \n");
       SWRITE("                                                       \n");
       SWRITE("Based on ");setversline(); SWRITE(", by Hoon Hong\n");
       SWRITE("with contributions by: Christopher W. Brown, George E. \n");
       SWRITE("Collins, Mark J. Encarnacion, Jeremy R. Johnson        \n");
       SWRITE("Werner Krandick, Richard Liska, Scott McCallum,        \n");
       SWRITE("Nicolas Robiduex, and Stanly Steinberg                 \n");
       SWRITE("=======================================================\n");

Return: /* Prepare for return. */
       return;
}
   
   
