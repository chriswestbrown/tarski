/***************************************************************
 * This file MAPLE.h and MAPLE.cpp define a CAServer based
 * on the Maple system.
 ***************************************************************/
#ifndef _MAPLE_
#define _MAPLE_

#include "unnamedpipe.h"
#include "CAServer.h"
#include <sstream>
#include <signal.h>


class MapleServer : public CAServer
{
public:
  UnnamedPipe intoMaple, outofMaple;
  
  MapleServer(std::string dirPath = "");
  ~MapleServer();
  
  void IPFAC(Word r, Word P, Word *s_, Word *c_, Word *L_);
  void IPFACRES(Word r, Word A, Word B, Word *s_, Word *c_, Word *L_);
  void IPFACDSCR(Word r, Word P, Word *s_, Word *c_, Word *L_);
  void IPFACLDCF(Word r, Word P, Word *s_, Word *c_, Word *L_);

  const std::string name() { return "Maple"; }
  int serverTime();
  void reportStats(std::ostream &out);
};


#endif
