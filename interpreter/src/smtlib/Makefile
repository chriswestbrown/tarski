CXX += -std=c++0x
CPPFLAGS += $(extIncludesSACLIB) -I../formula
CXXFLAGS += -Wall -Wno-unused-variable -Wno-parentheses -Wno-unused-label
ARFLAGS := crvU

LIB = smt-lib.a
LIB_OBJS = \
 $(LIB)(readSMTRetFormula.o)

.PHONY: lib deb clean

lib:	$(LIB)

deb:	$(LIB)
	ar -x ../poly/poly.a CREAD.o CWRITE.o GCSI.o
	$(CXX)  $(CXXFLAGS) -o go -I$(saclib)/include \
		tester.cpp GCSI.o CREAD.o CWRITE.o \
		 $(LIB) $(EXTLIBS)
	rm -f *.o

clean:	
	rm -f go $(LIB)

$(LIB):	$(LIB_OBJS)

