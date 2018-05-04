/***************************************************************
 * This file SINGULAR.h and SINGULAR.cpp define a CAServer based
 * on the Singular system.
 ***************************************************************/
#ifndef _SINGULAR_
#define _SINGULAR_

#include "unnamedpipe.h"
#include "CAServer.h"
#include <sstream>
#include <signal.h>


class SingularServer : public CAServer
{
public:
  UnnamedPipe intoSingular, outofSingular;  
  SingularServer(std::string dirPath="");
  ~SingularServer();
  
  void IPFAC(Word r, Word P, Word *s_, Word *c_, Word *L_);
  Word IPRES(Word r, Word A, Word B);
  Word IPDSCR(Word r, Word A);
  Word IPFACTGB(Word r, Word I, Word N);
  Word CONSTORDTEST(Word r, Word A, Word L);

  const std::string name() { return "Singular"; }
  int serverTime();
  void reportStats(std::ostream &out);
};


#endif
