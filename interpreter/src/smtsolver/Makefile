CXX += -std=c++0x
CPPFLAGS += $(extIncludesSACLIB) -I../formula 
CXXFLAGS += -Wall -Wno-unused-variable -Wno-parentheses -Wno-unused-label -O3
ARFLAGS := crvU

LIB = smtsolver.a
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp %.o $(SRC))

LIB_OBJS = \
 $(LIB) $( $OBJ  )
#dont forget to add back in miwtbasis.o!

bbwb.a: $OBJ
	rm -f *.o


.PHONY: lib deb clean



lib:
	$(LIB)

deb:
	$(LIB)


clean:
	rm -f $(LIB)
	rm -f *.o

$(LIB): $(LIB_OBJS)

