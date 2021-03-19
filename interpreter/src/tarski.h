#ifndef _TARSKI_HEADER_
#define _TARSKI_HEADER_

#include <string>

namespace tarski {

static const char * tarskiVersion     = "1.29";
static const char * tarskiVersionDate = "Fri Mar 19 16:06:57 EDT 2021";

/*******************************************************************
 * TarskiException - This is the basic exception class for all
 * exceptions thrown by this system.
 *******************************************************************/
class TarskiException : public std::exception
{
private:
  std::string msg;
public:
  TarskiException(const std::string &msg) throw() : msg(msg) { }
  ~TarskiException() throw() { }
  virtual const char* what() const throw() { return msg.c_str(); }
};

} // end namespace tarski
#endif
