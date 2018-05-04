CXX += -std=c++0x
CPPFLAGS += $(extIncludesSACLIB) -I../formula
CXXFLAGS += -Wall -Wno-unused-variable -Wno-parentheses -Wno-unused-label -Wno-sign-compare
ARFLAGS := crvU

LIB = onecell.a
LIB_OBJS = \
 ${LIB}(opennucad.o) \
 ${LIB}(builder.o) \
 ${LIB}(opencell.o) \
 ${LIB}(cellbound.o) \
 ${LIB}(varorder.o) \
 ${LIB}(OCBuilderType.o) \
 ${LIB}(OpenNuCADType.o)

.PHONY: lib deb clean

lib:	onecell.a

onecell.a: ${LIB_OBJS}

clean:	
	rm -f onecell.a


