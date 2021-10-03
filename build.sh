#!/bin/bash

### To use an existing external Saclib and/or Qepcad
### replace the empty strings below with full paths.
externalSaclibRoot=""
externalQepcadRoot=""

### Use "TOOLCHAIN=emmake ./build.sh" if you intend to start a WebAssembly build.
### Use "STATIC=1 ./build.sh" if you want to compile the tarski executable statically.
### Use "./build.sh clean" to clean up and remove objects that are already built.

######################################################################################

export STATIC
export TOOLCHAIN

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
if [ "$externalSaclibRoot" = "" ]
then
    saclibRoot="$tarskiRoot/saclib2.2.7"
else
    saclibRoot=$externalSaclibRoot
fi

export saclib=$saclibRoot
if [ "$externalQepcadRoot" = "" ]
then
    qepcadRoot="$tarskiRoot/qesource"
else
    qepcadRoot=$externalQepcadRoot
fi

export qe=$qepcadRoot

if [ "$1" = "clean" ]; then
 pushd interpreter
 check "make clean"
 popd
 pushd saclib2.2.7
 rm -fr lib
 popd
 exit 0
 fi

pushd $saclib
echo "Making SACLIB..."

# Create the makefiles only when this is a fresh build.
# Otherwise avoid rebuilding the whole system from scratch.
# (Recreating the makefiles results in a full rebuild.)
if [ ! -e include/sacproto.h -o ! -e lib/objd/makefile -o ! -e lib/objo/makefile ]; then
    if [ "$TOOLCHAIN" = emmake ]; then
        check "bin/sconf wasm"
    else
        check bin/sconf
    fi
    check bin/mkproto
    check bin/mkmake
fi
check "bin/mklib all"
check
echo "Saclib done"
popd

### QEPCAD
export PATH=$qe/bin:$PATH
pushd $qe
echo "Making QEPCAD..."
check "$TOOLCHAIN make opt"
echo "QEPCAD done"
popd

### HACKED MINISAT
minisatRoot="$tarskiRoot/minisat"
export TMROOT=$minisatRoot
pushd $TMROOT/core
echo "Making Minisat..."
check "$TOOLCHAIN make libr"
echo "Minisat Done"
popd

### TARSKI
pushd interpreter
echo "Making tarski..."
pushd ./src
check ./mksysdep.sh
popd

check "$TOOLCHAIN make"
popd

### LIBTARSKI
UNAME_S=`uname -s`

if [ "$UNAME_S" = "Darwin" ]; then
 JAVA=`find /usr/local/Cellar/openjdk/*/ | sort | head -1`
 if [ "$JAVA" = "" ]; then
  echo "No Java found. Consider installing it via Homebrew (openjdk)."
  fi
 which swig > /dev/null || {
   echo "No swig found. Consider installing it via Homebrew (swig)."
 }
 fi

if [ "$UNAME_S" = "Linux" ]; then
 JAVA=`find /usr/lib/jvm/* | sort | head -1`
 if [ "$JAVA" = "" ]; then
  echo "No Java found. To compile libtarski you need a working JDK installation."
  fi
 which swig > /dev/null || {
   echo "No swig found. To compile libtarski you need it."
   }
 fi

if [[ "$UNAME_S" == *"MINGW"* ]]; then
 JAVA=`find /c/Program\ Files\/OpenJDK/* | sort | head -1`
 if [ "$JAVA" = "" ]; then
  echo "No Java found. Consider installing it via choco (openjdk)."
  fi
 which swig > /dev/null || {
   echo "No swig found. Consider installing it via pacman (swig)."
   }
 fi

if [ "$TOOLCHAIN" != emmake -a "$JAVA" != "" ]; then
 which swig > /dev/null && {
  pushd interpreter
  echo "Making libtarski..."
  export JAVA
  check "make dll"
  make dlltest || echo "The Java Native Interface seems unstable."
  popd
  }
 fi



### FINAL MESSAGE
echo -e "\nTarski done!"
echo -e "######################################################"
echo -e "IMPORTANT!!!  PLEASE READ BELOW!!!"
echo -e "######################################################"
echo -e "There are several environment variables that should be"
echo -e "set in order to use or recompile Tarski.  So it is"
echo -e "strongly recommended that the lines:\n"
echo -e "export saclib=$saclibRoot"
echo -e "export qe=$qepcadRoot\n"
echo -e "export PATH=\$PATH:$qepcadRoot/bin\n"
echo -e "are added to your .profile (or .bash_profile, depending"
echo -e "on which you use) or whichever the equivalent file is on"
echo -e "your system."
echo -e "######################################################"



