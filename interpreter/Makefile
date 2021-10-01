# Master Makefile for the Tarski Intepreter
# Following the blog post by Job Vranish
# https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

UNAME_S := $(shell uname -s)
JAVA := ${JAVA}

MROOT=-I../minisat
TMROOT=../minisat
extIncludesSACLIB:=-I${saclib}/include -I${qe}/source
extLibSACLIB:=${saclib}/lib/saclibo.a
extLibMINISAT:=../minisat/core/lib.a
extIncludeMINISAT:=-I${TMROOT}/core
extIncludesMD5:=
extLibMD5:=-lssl  -lcrypto

saclibOverrides:=$(shell find ${qe}/source/saclib/ | grep -E '[.]o' | grep -Ev 'CSSP.o|IPIIS.o|CREAD.o|CWRITE.o|GCSI.o|gcword.o' | xargs)

extLibQEPCAD:=  ${qe}/source/qepcad.a \
		${qe}/extensions/sfext/sfexto.a \
		${qe}/extensions/lift2D/lift2Do.a \
		${qe}/extensions/adj2d/adj2do.a \
		${qe}/extensions/sfext/sfexto.a \
		${qe}/source/qepcad.a

# The rend extension will not be built on Windows MSYS2/CLANG64, otherwise always.
ifeq (,$(findstring MINGW64,$(UNAME_S)))
extLibQEPCAD += ${qe}/extensions/rend/rendo.a
endif

# Find the openssl include/lib folders automatically. We assume that Homebrew is already installed.
ifeq ($(UNAME_S),Darwin)
MAC_OPENSSL_INCLUDE := $(shell find /usr/local/Cellar/openssl*/* -name include | sort | tail -1)
MAC_OPENSSL_LIB := $(shell find /usr/local/Cellar/openssl*/* -name lib | sort | tail -1)
extIncludesMD5:=-I$(MAC_OPENSSL_INCLUDE)
extLibMD5:=$(MAC_OPENSSL_LIB)/libssl.a $(MAC_OPENSSL_LIB)/libcrypto.a
endif

# For wasm we need to have openssl preinstalled
ifeq ($(TOOLCHAIN),emmake)
WASM_OPENSSL_INCLUDE=$(EMSDK)/upstream/emscripten/system/include
WASM_OPENSSL_LIB=$(EMSDK)/upstream/emscripten/system/lib
extIncludesMd5:=-I$(WASM_OPENSSL_INCLUDE)
extLibMD5:=$(WASM_OPENSSL_LIB)/libssl.a $(WASM_OPENSSL_LIB)/libcrypto.a
endif

TARGET_EXEC ?= tarski

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

SRCS := $(sort $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s))
ASRC := $(filter-out */shell.cpp, $(sort $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)))
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
AOBJ := $(ASRC:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

LDFLAGS :=  $(saclibOverrides) $(extLibQEPCAD) $(extLibSACLIB) $(extLibMINISAT) $(extLibMD5)
# On Mac and Windows MSYS2/CLANG64 the rt library is not present, but otherwise it is necessary.
ifneq ($(UNAME_S),Darwin)
ifeq (,$(findstring MINGW64,$(UNAME_S)))
LDFLAGS += -lrt
ifdef STATIC
LDFLAGS += -lpthread -ltinfo -static
endif
endif
endif
# On non-wasm we always need readline:
ifneq ($(TOOLCHAIN),emmake)
LDFLAGS += -lreadline
else
# Otherwise we use a different target suffix:
TARGET_EXEC := tarski.html
LDFLAGS += -s "EXTRA_EXPORTED_RUNTIME_METHODS=['cwrap']"
endif


INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS)) $(extIncludesSACLIB) $(extIncludeMINISAT) $(extIncludesMD5)

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -g
# On Mac the C++11 dialect must be set, on Linux it is optional, on Windows MSYS2/CLANG64 it should not be set.
ifeq ($(UNAME_S),Darwin)
CPPFLAGS += --std=c++11
endif


$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
ifneq ($(TOOLCHAIN),emmake)
	mv build/tarski bin/tarski
endif

lib: tarski.a

tarski.a: $(AOBJ)
	$(AR) r $@ $(AOBJ)
	mv tarski.a ../lib/tarski.a


### DLL related code ###

# Output file on Linux:
LIBTARSKI:=libtarski.so

# Output file on Mac:
ifeq ($(UNAME_S),Darwin)
LIBTARSKI:=libtarski.jnilib
endif

# Output file on Windows:
ifeq (,$(findstring MINGW64,$(UNAME_S)))
else
LIBTARSKI:=tarski.dll
endif

dll: $(LIBTARSKI)

SWIG := src/swig
LDFLAGS_SHARED = $(filter-out -static, $(LDFLAGS))

$(SWIG)/tarski_wrap.cxx:
	cd $(SWIG); swig -c++ -java tarski.i
src/swig/tarski_wrap.o: src/swig/tarski_wrap.cxx
	$(CXX) -c $(SWIG)/tarski_wrap.cxx -Isrc/main -Isrc -I$(SACLIB)/include \
        -I"$(JAVA)/include" -I"$(JAVA)/include/linux" -I"$(JAVA)/include/win32" -fPIC
	mv tarski_wrap.o $(SWIG)/
$(LIBTARSKI): $(OBJS) $(SWIG)/tarski_wrap.o
	$(CXX) -shared $(SWIG)/tarski_wrap.o $(OBJS) -o $@ $(LDFLAGS_SHARED) -fPIC
$(SWIG)/main.class: $(SWIG)/main.java
	cd $(SWIG); "$(JAVA)/bin/javac" main.java
dlltest: $(SWIG)/main.class $(LIBTARSKI)
	cd $(SWIG); "$(JAVA)/bin/java" -Djava.library.path=../.. main
$(SWIG)/JTarski.class: $(SWIG)/JTarski.java
	cd $(SWIG); "$(JAVA)/bin/javac" JTarski.java
jtarski: $(SWIG)/JTarski.class $(LIBTARSKI)
	cd $(SWIG); "$(JAVA)/bin/java" -Djava.library.path=../.. JTarski

### End of DLL related code ###


# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

CXXFLAGS += -fPIC
ifeq ($(TOOLCHAIN),emmake)
CXXFLAGS += -s USE_ZLIB=1 -D_EMCC2_ -s EXPORTED_FUNCTIONS=[_PCLIB,_mainLIB,_ENDTARSKILIB,_main] $(extIncludesMd5)
CXXFLAGS += -s "EXPORTED_RUNTIME_METHODS=['PCLIB,mainlib,ENDTARSKILIB,main']"
CXXFLAGS += -s "EXTRA_EXPORTED_RUNTIME_METHODS=['cwrap']"
endif

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
