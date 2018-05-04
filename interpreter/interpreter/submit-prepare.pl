#!/usr/bin/perl
##############################################################
# This script prepares for a new submit of the tarski system.
##############################################################

## increment the minor version number & give new date stamp
`./submit-prepare-help.pl < tarski.h > _tmp_tarski.h_`;
`mv -f _tmp_tarski.h_ tarski.h`;
print("Version stamp complete!\n");

