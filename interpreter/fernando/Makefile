CXX += -std=c++0x
CPPFLAGS += $(extIncludesSACLIB) -I../formula 
CXXFLAGS += -Wall -Wno-unused-variable -Wno-parentheses -Wno-unused-label -O3
ARFLAGS := crvU

LIB = fernando.a
LIB_OBJS = \
 $(LIB)(sample-comm.o type-comm.o BBSat.o fern-poly-iter.o sactester.o mono-explain.o poly-explain.o deduce-sign.o deduction.o wbsat.o wbsat-tester.o  formula-maker.o minhitset/naive/hitset.o mhs-gen.o minwtbasis.o bbded.o boxer.o bbwb.o box-solver.o box-solver-comm.o qep-solver.o qep-solver-comm.o)
#dont forget to add back in miwtbasis.o!

fernando.a: sample-comm.o type-comm.o BBSat.o fern-poly-iter.o sactester.o mono-explain.o poly-explain.o deduce-sign.o deduction.o wbsat.o wbsat-tester.o  formula-maker.o minhitset/naive/hitset.o mhs-gen.o minwtbasis.o bbded.o boxer.o bbwb.o box-solver.o box-solver-comm.o qep-solver.o qep-solver-comm.o

sample-comm.o: sample-comm.cpp sample-comm.h

type-comm.o: type-comm.cpp type-comm.h

BBSat.o: BBSat.cpp BBSat.h ../formula/formula.a

fern-poly-iter.o: fern-poly-iter.cpp fern-poly-iter.h ../formula/formula.a ../poly/poly.a ../GC_System/GC_System.h ../formrepconventions.h

mono-explain.o: mono-explain.h mono-explain.cpp fern-poly-iter.o

poly-explain.o: mono-explain.o poly-explain.cpp poly-explain.h  fern-poly-iter.o

deduce-sign.o: mono-explain.o poly-explain.o deduce-sign.h deduce-sign.cpp fern-poly-iter.o

sactester.o: mono-explain.o poly-explain.o deduce-sign.o sactester.h sactester.cpp ../formula/formula.a

wbsat.o: wbsat.cpp wbsat.h ../formula/formula.a ../formrepconventions.h deduce-sign.o poly-explain.o mono-explain.o fern-poly-iter.o deduction.o

deduction.o: deduction.cpp deduction.h ../poly/poly.h ../poly/poly.cpp

wbsat-tester.o: wbsat-tester.cpp wbsat-tester.h wbsat.o

bbded.o: deduction.o BBSat.o

minwtbasis.o: minwtbasis.cpp minwtbasis.h bbded.o deduction.o ../formula/formula.a

boxer.o: boxer.cpp boxer.h wbsat.o bbded.o deduction.o

bbwb.o: bbwb.cpp bbwb.h boxer.o

formula-maker.o: formula-maker.cpp formula-maker.h ../minisat/core/lib.a ../formula/formula.a idx-manager.h

mhs-gen.o: mhs-gen.cpp mhs-gen.h minhitset/naive/hitset.o ../minisat/core/lib.a

minhitset/naive/hitset.o: minhitset/naive/hitset.cpp minhitset/naive/hitset.h

box-solver.o: box-solver.cpp box-solver.h ../poly/factor.cpp ../poly/factor.h ../shell/qepcad-inter/qepcad-session.h ../minisat/core/lib.a ../formula/formula.cpp ../formula/formula.h formula-maker.o idx-manager.h mhs-gen.o minhitset/naive/hitset.o

box-solver-comm.o: box-solver-comm.h box-solver-comm.cpp box-solver.o ../minisat/core/lib.a

qep-solver.o: box-solver.o qep-solver.cpp qep-solver.h ../minisat/core/lib.a mhs-gen.o minhitset/naive/hitset.o formula-maker.o

qep-solver-comm.o: qep-solver.o

../minisat/core/lib.a:
	cd ../minisat/core; make libd


.PHONY: lib deb clean



lib:
	$(LIB)

deb:
	$(LIB)


clean:
	rm -f $(LIB)
	rm -f *.o

$(LIB): $(LIB_OBJS)

