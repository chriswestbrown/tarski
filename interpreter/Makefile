##################################################################
#
# Master Makefile for Tarski programs.
#
#                             Chris Brown, 3/28/2008
##################################################################

CXX:=g++

MROOT=${CURDIR}/minisat
extIncludesSACLIB:=-I${saclib}/include
extLibSACLIB:=${saclib}/lib/saclibo.a
extLibMINISAT:=${MROOT}/core/lib.a
extIncludeMINISAT:=${MROOT}/core
extIncludesMD5:=
extLibMD5:=-lssl  -lcrypto

extIncludesREADLINE:=
extLibREADLINE:=-lreadline

CFLAGS:=-O4
CXXFLAGS:=-O4

export

tarski:	tarskisysdep.h
	cd algparse; make
	cd poly; make
	cd formula; make
	cd onecell; make
	cd realroots; make
	cd search; make
	cd smtlib; make
	cd minisat/core; make libr
	cd fernando; make
#	cd nucad; make
	cd shell; make


clean:
	cd algparse; make clean
	cd poly; make clean
	cd formula; make clean
	cd onecell; make clean
	cd search; make clean
	cd realroots; make clean
	cd smtlib; make clean
	cd fernando; make clean
	cd nucad; make clean
	cd shell; make clean
	rm -f bin/tarski


tarskisysdep.h:	mksysdep.sh
	./mksysdep.sh
