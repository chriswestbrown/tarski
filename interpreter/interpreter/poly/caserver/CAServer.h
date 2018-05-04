/***************************************************************
 * This file defines the class CAServer (Computer Algebra Server).
 * The fundamental idea is that some standard calculations, like
 * resultants, discriminants, factorization and groebner basis
 * calculation could be provided by different systems.  The
 * CAServer provides an interface to a system.
 ***************************************************************/
#ifndef _CA_SERVER_
#define _CA_SERVER_
extern "C" {
#include "saclib.h"
}
#include <string>
#include <iostream>
#include <sys/types.h>
#include <signal.h>

class CAServer
{
protected:
  pid_t childpid;

public:
  CAServer() : childpid(0) { }
  virtual ~CAServer() { }

  virtual void IPFAC(Word r, Word P, Word *s_, Word *c_, Word *L_)
  {
    std::cerr << "IPFAC not implemented by this CAServer!" << std::endl;
    exit(1);
  }

  virtual Word IPRES(Word r, Word A, Word B)
  {
    std::cerr << "IPRES not implemented by this CAServer!" << std::endl;
    exit(1);
  }
 
  virtual void IPFACRES(Word r, Word A, Word B, Word *s_, Word *c_, Word *L_)
  {
    Word R = IPRES(r,A,B);
    if (R == 0) { *s_ = 0; *c_ = 0; *L_ = NIL;}
    else { IPFAC(r-1,R,s_,c_,L_); }      
  }

  virtual Word IPDSCR(Word r, Word A)
  {
    std::cerr << "IPDSCR not implemented by this CAServer!" << std::endl;
    exit(1);
  }

  virtual void IPFACDSCR(Word r, Word P, Word *s_, Word *c_, Word *L_)
  {
    Word D = IPDSCR(r,P);
    if (D == 0) { *s_ = 0; *c_ = 0; *L_ = NIL;}
    else { IPFAC(r-1,D,s_,c_,L_); }          
  }

  virtual Word IPLDCF(Word r, Word A)
  {
    std::cerr << "IPLDCF not implemented by this CAServer!" << std::endl;
    exit(1);
  }

  virtual void IPFACLDCF(Word r, Word P, Word *s_, Word *c_, Word *L_)
  {
    Word D = IPLDCF(r,P);
    if (D == 0) { *s_ = 0; *c_ = 0; *L_ = NIL;}
    else { IPFAC(r-1,D,s_,c_,L_); }          
  }

  /*
    Integer Polynomial Factorized Groebner Basis: 
    Inputs:
     r = number of variables
     I = a list of r-variate polynomials that generate the ideal
     N = a list of negated equations, i.e. non-zero constraints
    Output:
     F = a factorized Groebner Basis given as a list of GB's, each
         GB being a list of r-variate integral polynomials.  The intersection
	 of these ideals has the same zero std::set as the original ideal,
	 assuming the given non-zero constraints.  Each polynomial
	 will be integral.	 
   */
  virtual Word IPFACTGB(Word r, Word I, Word N)
  {
    std::cerr << "IPFACTGB not implemented by this CAServer!" << std::endl;
    exit(1);
  }
  virtual Word CONSTORDTEST(Word r, Word A, Word L)
  {
    std::cerr << "CONSTORDTEST not implemented by this CAServer!" << std::endl;
    exit(1);
  }


  // Should return the CPU time (in ms) used by the server up to that point.
  virtual int serverTime() { return -1; }
  virtual void reportStats(std::ostream &out) 
  { 
    out << "Stats for CAServer " << name() << " not available." << std::endl; 
  }
  virtual const std::string name() { return "Empty"; }
  virtual int kill() // Should kill the server, ungracefully if need be!
  {
    if (childpid != 0)
      ::kill(childpid,SIGKILL);
    return 0;
  }
};


#endif
