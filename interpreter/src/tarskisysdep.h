#ifndef _TARSKI_SYSDEP_H_
#define _TARSKI_SYSDEP_H_
#include "string.h"
#include <string>
namespace tarski {
static const char * pathToQepcad = "/home/fernando/Research/qesource/bin/qepcad";
static const char * pathToMaple = "";
static const char * pathToSingular = "";

if (strcmp(pathToQepcad, "") == 0) throw TarskiException("Invalid location for QEPCAD")
} // end namespace tarski
#endif
