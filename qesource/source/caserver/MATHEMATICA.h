/***************************************************************
 * This file MATHEMATICA.h and MATHEMATICA.cpp define a CAServer based
 * on the Mathematica system.
 ***************************************************************/
#ifndef _MATHEMATICA_
#define _MATHEMATICA_

#include "unnamedpipe.h"
#include "CAServer.h"
#include <sstream>
#include <signal.h>


class MathematicaServer : public CAServer
{
public:
  UnnamedPipe intoMathematica, outofMathematica;
  
  MathematicaServer(std::string dirPath="");
  ~MathematicaServer();
  
  void IPFAC(Word r, Word P, Word *s_, Word *c_, Word *L_);

  const std::string name() { return "Mathematica"; }
  int serverTime();
  void reportStats(std::ostream &out);
};


#endif
