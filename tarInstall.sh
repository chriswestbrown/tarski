#!/bin/bash

trap "exit 1" TERM
export TOP_PID=$$

function check(){
    $1 > /dev/null
    if [ $? -ne 0 ]
    then
        echo "Failed on" $1
        kill -s TERM $TOP_PID
    fi
}

cd $saclib
echo "Making SACLIB..."
check bin/sconf
check bin/mkproto
check bin/mkmake
check "bin/mklib all"
check
echo "Saclib done"
cd $qe
echo "Making QEPCAD..."
check make
echo "QEPCAD done"
cd $TMROOT/core
echo "Making Minisat..."
check "make libr"
echo "Minisat Done"
cd ../../interpreter
echo "Making tarski..."
check make
echo "Tarski done. If you did not add $qe/bin to your path variable, add it now and call make in tarski/interpreter to properly set the location of QEPCAD."


