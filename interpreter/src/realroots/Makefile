CXX += -std=c++0x
CC := $(CXX)
CPPFLAGS += $(extIncludesSACLIB) -I../formula
CFLAGS += -Wall -Wno-unused-variable -Wno-parentheses -Wno-unused-label
CXXFLAGS += -Wall -Wno-unused-variable -Wno-parentheses -Wno-unused-label
ARFLAGS := crvU

LIB = realroots.a
LIB_OBJS = \
 $(LIB)(realroots.o) \
 $(LIB)(RealAlgNumType.o) \
 $(LIB)(IPIIS.o) \
 $(LIB)(CSSP.o)

.PHONY: deb clean

deb:	$(LIB)

$(LIB): $(LIB_OBJS)

clean:	
	@- rm -f $(LIB) *.o



