CXX += -std=c++0x
CXXFLAGS += -Wall -Wno-unused-variable -Wno-parentheses
ARFLAGS := crvU
LIB = algparse.a
LIB_OBJS = \
 algparse.a(grammar.tab.o) \
 algparse.a(alglex.o) \
 algparse.a(algparse.o) \
 algparse.a(utils.o) \
 algparse.a(treemanip.o) \
 algparse.a(getvars.o) \
 algparse.a(prettyprint.o)

.PHONY: clean

opt:	algparse.a main.cpp printGross.h utils.h
	$(CXX) $(CXXFLAGS) -o frewrite main.cpp algparse.a

algparse.a: $(LIB_OBJS)
	$(AR) $(ARFLAGS) $@ $^
	@- rm -f grammar.tab.o

algparse.a(grammar.tab.o): grammar.ypp alglex.h
	bison -d --verbose grammar.ypp
	$(CXX) $(CXXFLAGS) -c grammar.tab.cpp

alglex.h: cwbpt.h
	touch alglex.h

getvars.h prettyprint.h: treemanip.h
	touch getvars.h prettyprint.h

printGross.h: prettyprint.h utils.h
	touch printGross.h

clean:
	@- rm -f frewrite algparse.a grammar.output grammar.tab.cpp grammar.tab.hpp



