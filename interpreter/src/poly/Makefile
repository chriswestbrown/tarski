CXX += -std=c++0x
CPPFLAGS += $(extIncludesSACLIB) $(extIncludesMD5)
CXXFLAGS += -Wall -Wno-unused-variable -Wno-parentheses -Wno-unused-label
ARFLAGS := crvU
EXTLIBS += $(extLibSACLIB) $(extLibMD5)

LIB_OBJS = \
 poly.a(gcword.o) \
 poly.a(GCSI.o) \
 poly.a(CREAD.o) \
 poly.a(CWRITE.o) \
 poly.a(sacMod.o) \
 poly.a(sacpolyutils.o) \
 poly.a(poly.o) \
 poly.a(specialDeduction.o) \
 poly.a(polymanager.o) \
 poly.a(factor.o) \
 poly.a(variable.o) \
 poly.a(md5digest.o) \
 poly.a(tracker.o) \
 poly.a(caserver/MAPLE.o) \
 poly.a(caserver/MATHEMATICA.o) \
 poly.a(caserver/SINGULAR.o) \
 poly.a(caserver/COCOA.o)

.PHONY: deb clean


deb:	poly.a
	ar -xU poly.a CREAD.o CWRITE.o GCSI.o
	${CXX} -o go -I${saclib}/include \
		test.cpp GCSI.o CREAD.o CWRITE.o \
	poly.a ${EXTLIBS}
	rm -f *.o

tstf:	poly.a testf.cpp
	ar -xU poly.a CREAD.o CWRITE.o GCSI.o
	${CXX} -o tstf -I${saclib}/include \
		testf.cpp GCSI.o CREAD.o CWRITE.o \
		poly.a ${saclib}/lib/saclibd.a
	rm -f *.o

poly.a: ${LIB_OBJS}

clean:
	@- rm -f poly.a go tstf
	rm -f *.o

#${LIB}(md5digest.o):	md5digest.cpp md5digest.h
#	${CXX} ${CFLAGS} -c $<
#	ar -rU ${LIB} ${<:.cpp=.o}
#	rm -f ${<:.cpp=.o}

