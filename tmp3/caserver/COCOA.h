/***************************************************************
 * This file COCOA.h and COCOA.cpp define a CAServer based
 * on the Cocoa system.
 ***************************************************************/
#ifndef _COCOA_
#define _COCOA_

#include "unnamedpipe.h"
#include "CAServer.h"
#include <sstream>
#include <signal.h>


class CocoaServer : public CAServer
{
public:
  UnnamedPipe intoCocoa, outofCocoa;
  
  CocoaServer(std::string dirPath="");
  ~CocoaServer();
  
  void IPFAC(Word r, Word P, Word *s_, Word *c_, Word *L_);
  Word IPRES(Word r, Word A, Word B);
  Word IPDSCR(Word r, Word A);
  Word IPFACTGB(Word r, Word I, Word N);
  Word CONSTORDTEST(Word r, Word A, Word L);

  const std::string name() { return "Cocoa"; }
  int serverTime();
  void reportStats(std::ostream &out);
};


#endif
