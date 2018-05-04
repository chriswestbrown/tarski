CXX += -std=c++0x
CPPFLAGS += $(extIncludesSACLIB) -I../formula
CXXFLAGS += -Wall -Wno-unused-variable -Wno-parentheses -Wno-unused-label
ARFLAGS := crvU
EXTLIBS = ../formula/formula.a  ../realroots/realroots.a ../poly/poly.a \
	../algparse/algparse.a  $(extLibSACLIB)  $(extLibMD5)

LIB = search.a
LIB_OBJS = \
 $(LIB)(grade.o) \
 $(LIB)(rewrite.o) \
 $(LIB)(search.o) \
 $(LIB)(graph.o) \
 $(LIB)(qfr.o)

.PHONY: opt clean

opt:	$(LIB)
	ar -x ../poly/poly.a CREAD.o CWRITE.o GCSI.o
	$(CXX) $(CXXFLAGS) -o qfr -I$(saclib)/include test.cpp GCSI.o CREAD.o CWRITE.o \
		$(LIB) $(EXTLIBS)
	rm -f *.o


$(LIB):	$(LIB_OBJS)

$(LIB)(grade.o):	grade.cpp grade.h

$(LIB)(search.o):	search.cpp search.h

$(LIB)(rewrite.o):	rewrite.cpp rewrite.h

$(LIB)(graph.o):	graph.cpp search.h

$(LIB)(qfr.o):	qfr.cpp search.h rewrite.h grade.h

## Make sure the nesting of .h's is specified
grade.h:	search.h
	touch grade.h
rewrite.h:	search.h
	touch rewrite.h

clean:
	rm -f qfr search.a
