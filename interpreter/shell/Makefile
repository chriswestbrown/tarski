CXX += -std=c++0x
CPPFLAGS += $(extIncludesSACLIB) -I../formula $(extIncludesMD5) $(extIncludesREADLINE) 
CXXFLAGS += -Wall -Wno-unused-variable -Wno-parentheses -Wno-unused-label -Wno-sign-compare
ARFLAGS := crvU
EXTLIBS += ../smtlib/smt-lib.a ../onecell/onecell.a ../realroots/realroots.a ../search/search.a \
           ../formula/formula.a ../poly/poly.a ../algparse/algparse.a \
	   ../fernando/fernando.a \
	   $(extLibSACLIB)  $(extLibMD5) $(extLibREADLINE)



LIB = shell.a
LIB_OBJS = \
 $(LIB)(schemish.o) \
 $(LIB)(einterpreter.o)	\
 $(LIB)(utils.o)

.PHONY: shell clean scheme

shell:	../formula/formmanip.h $(LIB)
	ar -x ../poly/poly.a CREAD.o CWRITE.o GCSI.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o tarski shell.cpp GCSI.o CREAD.o CWRITE.o \
		$(LIB) $(EXTLIBS) $(LIB) $(EXTLIBS) ../minisat/core/lib.a
	cp --preserve -f tarski ../bin/tarski 
	rm -f *.o

../minisat/core/lib.a:
	cd ../minisat/core; make libr

scheme:	../algparse/algparse.a ../poly/poly.a schemish.cpp ../formula/formmanip.h $(LIB)
	ar -x ../poly/poly.a CREAD.o CWRITE.o GCSI.o
	$(TCPP) $(CPPFLAGS) -o schemish -I$(saclib)/include schemish.cpp GCSI.o CREAD.o CWRITE.o \
		$(LIB) $(EXTLIBS) $(LIB)
	rm -f *.o

clean:
	rm -f shell.a tarski schemish

$(LIB):	$(LIB_OBJS)

$(LIB)(schemish.o):	schemish.cpp schemish.h utils.h

$(LIB)(einterpreter.o):	einterpreter.cpp einterpreter.h funcEvalFormAtRat.h

$(LIB)(utils.o):	utils.cpp utils.h

## Make sure the nesting of .h's is specified
einterpreter.h:	schemish.h utils.h
	touch einterpreter.h


