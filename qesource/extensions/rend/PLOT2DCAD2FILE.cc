/*======================================================================
                      PLOT2DCAD2FILE(D,P,J)

Inputs
  D : a CAD data-structure
  P : the projection factor set for D
  J : the projection polynomial set for D

Side Effects
First, this reads in arguments for plotting, the character D indicates
that default values are to be used for all parameters, while E indicates
that the user will type in explicit values.  The 2D CAD plotting 
programs are then called.
======================================================================*/
#include "rend.h"
#include <cstring>

void QepcadCls::PLOT2DCAD2FILE(Word D, Word P, Word J)
{

/********************************************************************/
Step0: /* Read user input */

  // Get basic info
  double x,X,y,Y,e;
  int Id1 = 600, Id2 = 600;
  char S[40];
  singlelinestream lin(qein(),singlelinestream::skipleadingws);
  if (!(lin >> x >> X >> y >> Y >> e >> S)) {
    SWRITE("xmin xmax ymin ymax stride filename, try 'whatis d-2d-cad'!\n");
    qein().putback('\n'); //QEPCAD's next command function looks for a newline first!
    return; }
  if (!( x < X && y < Y)) {
    SWRITE("Must have xin < xmax, ymin < ymax!\n");
    qein().putback('\n'); //QEPCAD's next command function looks for a newline first!
    return; }

  // Get additional flags
  bool 
    z = true, // show zero dimensional cell
    c = true, // make a plot in color
    s = false; // show the semi-algebraic set only, not the CAD artifacts
  signed char g;
  while((g = lin.get()) && g != EOF) {
    switch(g) {
    case 'z': z = false; break;
    case 'Z': z = true; break;
    case 'c': c = false; break;
    case 'C': c = true; break;
    case 's': s = false; break;
    case 'S': s = true; break;
    default: break;
    } }
  qein().putback('\n'); //QEPCAD's next command function looks for a newline first!

/********************************************************************/
Step1: /* Make sure at least the CAD of 1-space exists. Initialize. */
      if (LELTI(D,CHILD) == NIL) {
	SWRITE("No decomposition to plot!\n");
	return; }

/********************************************************************/
 Step2: /* Open the output file. */
  ofstream out;
  out.open(S);
  if (!out) {
    printf("File %s could not be opened.\n",S);
    return; }

/********************************************************************/
Step3:/* Get a nice initial plot.                                   */
  Word Ix = IEEELBRN(x),IX = IEEELBRN(X),Iy = IEEELBRN(y),
    IY = IEEELBRN(Y),E = IEEELBRN(e),L;
  Rend_Cell M;        /* M is the "mirror" CAD for plotting.    */
  CONMIRCAD(D,P,J,M,*this); /* This actually constructs M.            */
  if (s)
    ADDBOUNDARYINFO(M,*this);
  Rend_Win W(M,Id1,Id2,Ix,IX,Iy,IY);
  W.set_precis_faithfull();
  W.update_extents(M);
  L = W.get_lociva(M);
  FILL_2D(M,W,E,L,P);
  
/********************************************************************/
Step4: /* Produce plot! */
  int n = strlen(S);
  if (n > 4 && strcmp(S + (n - 4),".svg") == 0) {
    WRITE_SVG(M,W,L,E,P,out,c,z,s);
  }
  else
    WRITE_EPS(M,W,L,E,P,out,c,z);
  out.close();

  return;
}


void QepcadCls::PLOT2DTOOUTPUTSTREAM(int Id1, int Id2, double x, double X, double y, double Y, double e,
				     ostream& out,
				     bool c, bool z, bool s)
{
  Word D = GVPC, P = GVPF, J = GVPJ;
  Word Ix = IEEELBRN(x),IX = IEEELBRN(X),Iy = IEEELBRN(y),
    IY = IEEELBRN(Y),E = IEEELBRN(e),L;
  Rend_Cell M;        /* M is the "mirror" CAD for plotting.    */
  CONMIRCAD(D,P,J,M,*this); /* This actually constructs M.            */
  if (s)
    ADDBOUNDARYINFO(M,*this);
  Rend_Win W(M,Id1,Id2,Ix,IX,Iy,IY);
  W.set_precis_faithfull();
  W.update_extents(M);
  L = W.get_lociva(M);
  FILL_2D(M,W,E,L,P);
  WRITE_SVG(M,W,L,E,P,out,true,true,s);
}
