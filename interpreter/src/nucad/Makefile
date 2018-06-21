CXX += -std=c++0x
CC := $(CXX)
CPPFLAGS += $(extIncludesSACLIB) -I../formula
CFLAGS += -Wall -Wno-unused-variable -Wno-parentheses -Wno-unused-label
CXXFLAGS += -Wall -Wno-unused-variable -Wno-parentheses -Wno-unused-label
ARFLAGS := crvU

LIB = nucad.a
LIB_OBJS = \
 $(LIB)(property.o)

.PHONY: deb clean

deb:	$(LIB)

$(LIB): $(LIB_OBJS)

clean:	
	@- rm -f $(LIB) *.o



