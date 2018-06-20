
# Set to the full path to tarski/interpreter
TarskiRoot=..

# The saclib environment variable should be set
extIncludesSACLIB:=-I${saclib}/include
extLibSACLIB:=${saclib}/lib/saclibo.a
extIncludesMD5:=
extLibMD5:=-lssl  -lcrypto

extIncludesREADLINE:=
extLibREADLINE:=-lreadline

CFLAGS:=-g
CXXFLAGS:=-g

CXX += -std=c++0x
CPPFLAGS += $(extIncludesSACLIB) -I${TarskiRoot}/formula $(extIncludesMD5) $(extIncludesREADLINE)
CXXFLAGS += -Wall -Wno-unused-variable -Wno-parentheses -Wno-unused-label -Wno-sign-compare
ARFLAGS := crvU
EXTLIBS += ${TarskiRoot}/smtlib/smt-lib.a ${TarskiRoot}/onecell/onecell.a ${TarskiRoot}/realroots/realroots.a ${TarskiRoot}/search/search.a \
           ${TarskiRoot}/formula/formula.a ${TarskiRoot}/poly/poly.a ${TarskiRoot}/algparse/algparse.a \
	   ${TarskiRoot}/fernando/fernando.a \
	   $(extLibSACLIB)  $(extLibMD5) $(extLibREADLINE)

LIB = api.a
LIB_OBJS = $(LIB)(store.o)

.PHONY: base clean

base:	$(LIB)
	ar -x ${TarskiRoot}/poly/poly.a CREAD.o CWRITE.o GCSI.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o atest atest.cpp  \
		GCSI.o CREAD.o CWRITE.o \
		$(LIB) $(EXTLIBS) $(LIB) $(EXTLIBS) 
	rm -f *.o

$(LIB):	$(LIB_OBJS)

$(LIB)(store.o):	store.cpp store.h


clean:
	rm -f api.a atest


