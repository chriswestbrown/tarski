CXX += -std=c++0x
CXXFLAGS += -Wall -Wno-unused-variable -Wno-parentheses -Wno-unused-label
ARFLAGS := crvU
CPPFLAGS += $(extIncludesSACLIB) $(extIncludesMD5)

LIB = formula.a
LIB_OBJS = \
 $(LIB)(DynamicBitVector.o) \
 $(LIB)(formula.o) \
 $(LIB)(linearSubs.o) \
 $(LIB)(normalize.o) \
 $(LIB)(formmanip.o) \
 $(LIB)(monomialinequality.o) \
 $(LIB)(writeForQE.o) \
 $(LIB)(writeForQEPCADB.o) 

.PHONY: clean

$(LIB):	$(LIB_OBJS)

formula.o:	formula.cpp formula.h DynamicBitVector.o

normalize.o:	normalize.cpp normalize.h formula.o formmanip.o

linearSubs.o:	linearSubs.cpp linearSubs.h formula.o

formmanip.o:	formmanip.cpp formmanip.h formula.o #External deps not included 

monomialinequality.o:	monomialinequality.cpp monomialinequality.h DynamicBitVector.o normalize.o #External deps not included

writeForQE.o:	writeForQE.cpp writeForQE.h formula.o

writeForQEPCADB.o:	writeForQEPCADB.cpp formmanip.o normalize.o linearSubs.o writeForQE.o #External deps not included

DynamicBitVector.o:	DynamicBitVector.cpp DynamicBitVector.h

## Make sure the nesting of .h's is specified
normalize.h:	formula.h
	touch normalize.h
formmanip.h: formula.h
	touch formmanip.h # Also depends on poly & algparse
linearSubs.h:	formula.h
	touch linearSubs.h

clean:
	rm -f formula.a
