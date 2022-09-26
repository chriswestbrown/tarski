#!/bin/bash

### To use an existing external Saclib and/or Qepcad
### replace the empty strings below with full paths.
externalSaclibRoot=""
externalQepcadRoot=""

### Use "TOOLCHAIN=emmake ./build.sh" if you intend to start a WebAssembly build.
### Use "STATIC=1 ./build.sh" if you want to compile the tarski executable statically.
### Use "./build.sh clean" to clean up and remove objects that are already built.
### Use "READLINE=0 ./build.sh" if you want to omit readline support.
### Use "MAKEOPT=-j4 ./build.sh" to use 4 cores on compilation (it has effect only for the interpreter, not for QEPCAD or Saclib).
### Use "CC=clang CXX=clang++ ./build.sh" when compiling via clang (important for Windows)

### Optimization issues
### -------------------
###
### By default, Tarski optimizied fully via the optimization option -O3.
### However, it may crash in certain cases. To avoid that, consider compiling the tool without certain optimizations.
### Tarski consists of three parts: Saclib (1), QEPCAD B (2), and the interpreter (3).
### These three parts may require special consideration.
###
### 1. Optimizing Saclib. You can safely optimize Saclib on the following platforms (with -O3):
###
### * Windows, MSYS2, clang32 (version 13.0.1)
### * Mac, clang (version LLVM 9.1.0, clang-902.0.39.2, tested on Mac 10.13.6)
###
### Safe optimizing is recommended via a careful configuration of optimization flags:
###
### * Ubuntu Linux 22.04 (amd64), gcc (vesion 11.2.0):
###   -O3 -fno-ipa-pure-const -fno-inline-small-functions -fno-inline-functions-called-once
### * Ubuntu Linux 18.04 (amd64), gcc (version 7.5.0):
###   -O3 -fno-ipa-pure-const -fno-inline-small-functions
### * Linux Mint 20.2 (amd64), gcc (version 9.4.0):
###   -O3 -fno-ipa-pure-const -fno-inline-small-functions
### * Raspberry Pi OS (based on Debian 11 Bullseye), gcc (version 10.2.1-6):
###   -O3 -fno-ipa-pure-const -fno-inline-small-functions -fno-inline-functions-called-once
###
### Example:
### sed -i s/-O3/"-fno-ipa-pure-const -fno-inline-small-functions -fno-inline-functions-called-once"/ saclib2.2.8/bin/mklib # recommended on Ubuntu Linux 22.04 Linux, gcc
###
### Optimizing is not recommended on the following platforms (expect various crashes):
###
### * Windows, MSYS2, clang64 (version 13.0.1)
### * Linux, clang (version 14.0.0)
### * Web, emscripten (version 3.1.22)
###
### Example:
### sed -i s/-O3// saclib2.2.8/bin/mklib # recommended on Windows, clang64
###
### 2. Optimizing QEPCAD B. You can safely optimize Saclib on the following platforms (with -O3):
###
### * Windows, MSYS2, clang32 (version 13.0.1)
### * Mac, clang (version LLVM 9.1.0, clang-902.0.39.2, tested on Mac 10.13.6)
### * Ubuntu Linux 22.04 (amd64), gcc (vesion 11.2.0)
### * Ubuntu Linux 18.04 (amd64), gcc (version 7.5.0)
### * Linux Mint 20.2 (amd64), gcc (version 9.4.0)
### * Raspberry Pi OS (based on Debian 11 Bullseye), gcc (version 10.2.1-6)
###
### Optimizing is not recommended on the following platforms (expect various crashes):
###
### * Linux, clang (version 14.0.0)
### * Windows, MSYS2, clang64 (version 13.0.1)
### * Web, emscripten (version 3.1.22)
###
### Example:
### sed -i s/-O3// qesource/Makefile # recommended on Windows, clang64
###
### 3. Optimizing the interpreter. You can safely optimize Saclib on the following platforms (with -O3):
###
### * Windows, MSYS2, clang32 (version 13.0.1)
### * Mac, clang (version LLVM 9.1.0, clang-902.0.39.2, tested on Mac 10.13.6)
### * Ubuntu Linux 22.04 (amd64), gcc (vesion 11.2.0)
### * Ubuntu Linux 18.04 (amd64), gcc (version 7.5.0)
### * Linux Mint 20.2 (amd64), gcc (version 9.4.0)
### * Raspberry Pi OS (based on Debian 11 Bullseye), gcc (version 10.2.1-6):
### * Web, emscripten (version 3.1.22)
###
### Optimizing is not recommended on the following platforms (expect various crashes):
###
### * Linux, clang (version 14.0.0)
###
### Example:
### sed -i s/-O3// interpreter/Makefile  # recommended on Linux, clang
###
### Safe optimizing is recommended via a careful configuration of optimization flags:
###
### * Windows, MSYS2, clang64 (version 13.0.1):
###   -O1
###
### Example:
### sed -i s/-O3/-O1/ interpreter/Makefile  # recommended on Windows, clang64

######################################################################################

if [ "$READLINE" = "" ]; then
 READLINE=1
 fi

export STATIC
export TOOLCHAIN
export READLINE

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
    saclibRoot="$tarskiRoot/saclib2.2.8"
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
 pushd saclib2.2.8
 rm -fr lib
 popd
 # Final cleanup
 find \( -name '*.o' -or -name '*.or' -or -name '*.a' -or -name '.exe' -or -name '*.wasm' -or -name '*.dll' -or -name '*.so' -or -name '*.jnilib' \) -delete
 rm -f interpreter/bin/tarski qesource/source/qepcad
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
check "$TOOLCHAIN make $MAKEOPT libr"
echo "Minisat Done"
popd

### TARSKI
pushd interpreter
echo "Making tarski..."
pushd ./src
check ./mksysdep.sh
popd

check "$TOOLCHAIN make $MAKEOPT"
popd

### LIBTARSKI
UNAME_S=`uname -s`

if [ "$UNAME_S" = "Darwin" ]; then
 if [ "$JAVA" = "" ]; then
  JAVA=`find /usr/local/Cellar/openjdk/*/ | sort | head -1`
  if [ "$JAVA" = "" ]; then
   echo "No Java found. Consider installing it via Homebrew (openjdk)."
   fi
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

if [[ "$UNAME_S" == *"MINGW64"* ]]; then
 JAVA=`find /c/Program\ Files\/OpenJDK/* | sort | grep jdk | head -1`
 if [ "$JAVA" = "" ]; then
  echo "No Java found. Consider installing it via choco (openjdk)."
  fi
 which swig > /dev/null || {
   echo "No swig found. Consider installing it via pacman (swig)."
   }
 fi

if [[ "$UNAME_S" == *"MINGW32"* ]]; then
 JAVA=`find /c/Program\ Files\ \(x86\)\/Java/* | sort | grep jdk | head -1`
 if [ "$JAVA" = "" ]; then
  echo "No Java found. Consider installing it via choco (jdk8 --x86)."
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
  check "make $MAKEOPT dll"
  timeout 60 make dlltest || echo "The Java Native Interface seems unstable."
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



