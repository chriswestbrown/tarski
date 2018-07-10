#!/bin/bash


{
echo '#ifndef _TARSKI_SYSDEP_H_'
echo '#define _TARSKI_SYSDEP_H_'
echo '#include "string.h"'
echo
echo 'namespace tarski {'
echo 'static const char * pathToQepcad = "'$(which qepcad | tr -d '\n')'";'
echo 'static const char * pathToMaple = "'$(which maple | tr -d '\n')'";'
echo 'static const char * pathToSingular = "'$(which Singular | tr -d '\n')'";'
echo '} // end namespace tarski'
echo '#endif'
} > tarskisysdep.h

