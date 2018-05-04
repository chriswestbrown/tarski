EXEC = mhstest
SRC_FILES = hitset.cpp

CFLAGS:=-O4
CXXFLAGS:=-O4
CPPFLAGS:=-I.

LDFLAGS         = -L.

LDLIBS          = 

O_FILES         = $(SRC_FILES:%.cpp=%.o)


.PHONY: ${EXEC} depend light-clean clean randgen

${EXEC}: ${O_FILES}
	g++ ${CXXFLAGS} -o ${EXEC} main.cpp ${O_FILES} \
	${LDFLAGS} ${LDLIBS}

randgen:
	g++ -o randgen randgen.cpp

depend:
	makedepend -- $(CXXFLAGS) -- -Y $(SRC_FILES)

light-clean:
	$(RM) $(O_FILES) core *.rpo

clean: light-clean
	$(RM) $(EXEC) randgen


# DO NOT DELETE

hitset.o: hitset.h
