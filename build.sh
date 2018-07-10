#!/bin/bash

trap "exit 1" TERM
export TOP_PID=$$

function check(){
    $1 # > /dev/null
    if [ $? -ne 0 ]
    then
        echo "Failed on" $1
        kill -s TERM $TOP_PID
    fi
}

tarskiRoot=$(pwd)

### SACLIB
saclibRoot="$tarskiRoot/saclib2.2.7"
export saclib=$saclibRoot
pushd $saclib
echo "Making SACLIB..."
check bin/sconf
check bin/mkproto
check bin/mkmake
check "bin/mklib all"
check
echo "Saclib done"
popd

### QEPCAD
qepcadRoot="$tarskiRoot/qesource"
export qe=$qepcadRoot
pushd $qe
echo "Making QEPCAD..."
check make
echo "QEPCAD done"
popd

### HACKED MINISAT
minisatRoot="$tarskiRoot/minisat"
export TMROOT=$minisatRoot
pushd $TMROOT/core
echo "Making Minisat..."
check "make libr"
echo "Minisat Done"
popd

### TARSKI
pushd interpreter
echo "Making tarski..."
check make
popd

### FINAL MESSAGE
echo -e "\nTarski done!"
echo -e "######################################################"
echo -e "IMPORTANT!!!  PLEASE READ BELOW!!!"
echo -e "######################################################"
echo -e "There are several environment variables that should be"
echo -e "set in order to use or recompile Tarski.  So it is"
echo -e "strongly recommended that the lines:\n"
echo -e "export \$saclib=$saclibRoot"
echo -e "export \$qe=$qepcadRoot\n"
echo -e "are added to your .profile (or .bash_profile, depending"
echo -e "on which you use) or whichever the equivalent file is on"
echo -e "your system."
echo -e "######################################################"



