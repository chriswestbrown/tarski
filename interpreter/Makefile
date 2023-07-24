# Master Makefile for the Tarski Intepreter
# Following the blog post by Job Vranish
# https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

UNAME_S := $(shell uname -s)
JAVA := ${JAVA}

# Platform detection
PLATFORM:=UNKNOWN
ifeq (,$(findstring MINGW,$(UNAME_S)))
ifneq ($(UNAME_S),Darwin)
ifneq ($(TOOLCHAIN),emmake)
PLATFORM:=LINUX
else
PLATFORM:=WASM
endif
else
PLATFORM:=MAC
endif
else
PLATFORM:=WINDOWS
endif
$(info Tarski is about to be built for platform $(PLATFORM).)

# Setting internal and external libs and includes
MROOT=-I../minisat
TMROOT=../minisat
extIncludesSACLIB:=-I${saclib}/include -I${qe}/source 
extLibSACLIB:=${saclib}/lib/saclibo.a
extLibMINISAT:=../minisat/core/lib.a
extIncludeMINISAT:=-I${TMROOT}/core
extIncludesMD5:=
extLibMD5:=-lcrypto
# On Linux and Windows (that is, not Mac), if we build statically, libcrypto will be set later
ifdef STATIC
ifneq ($(PLATFORM),MAC)
extLibMD5:=
endif
endif

saclibOverrides:=$(shell find ${qe}/source/saclib/ | grep -E '[.]o' | grep -Ev 'CSSP.o|IPIIS.o|CREAD.o|CWRITE.o|GCSI.o|gcword.o' | xargs)

extLibQEPCAD:=  ${qe}/source/qepcad.a \
		${qe}/extensions/sfext/sfexto.a \
		${qe}/extensions/lift2D/lift2Do.a \
		${qe}/extensions/adj2d/adj2do.a \
		${qe}/extensions/sfext/sfexto.a \
		${qe}/source/qepcad.a \
		${qe}/extensions/rend/rendo.a \
		${qe}/extensions/adj2d/adj2do.a \
    ${qe}/extensions/lift2D/lift2Do.a
    
# Note that the adj2do and lif2Do extensions had to be added twice (in this order).

# Find the openssl include/lib folders automatically on a Mac. We assume that Homebrew is already installed.
ifeq ($(PLATFORM),MAC)
MAC_OPENSSL_INCLUDE := $(shell find /usr/local/Cellar/openssl*/* -name include | sort | tail -1)
MAC_OPENSSL_LIB := $(shell find /usr/local/Cellar/openssl*/* -name lib | sort | tail -1)
extIncludesMD5:=-I$(MAC_OPENSSL_INCLUDE)
extLibMD5:=$(MAC_OPENSSL_LIB)/libssl.a $(MAC_OPENSSL_LIB)/libcrypto.a
endif

# For wasm we need to have openssl preinstalled
ifeq ($(PLATFORM),WASM)
WASM_OPENSSL_INCLUDE=$(EMSDK)/upstream/emscripten/system/include
WASM_OPENSSL_LIB=$(EMSDK)/upstream/emscripten/system/lib
extIncludesMd5:=-Icache/sysroot/include -Icache/sysroot/include/openssl
extLibMD5:=$(WASM_OPENSSL_LIB)/libssl.a $(WASM_OPENSSL_LIB)/libcrypto.a
endif

# By default we build an executable
TARGET_EXEC ?= tarski

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

SRCS := $(sort $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s))
ASRC := $(filter-out */shell.cpp, $(sort $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)))
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
AOBJ := $(ASRC:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

LDFLAGS :=  $(saclibOverrides) $(extLibQEPCAD) $(extLibSACLIB) $(extLibMINISAT) $(extLibMD5)
# On non-WASM we always need readline if set (and the order matters, it must precede -lrt):
ifneq ($(PLATFORM),WASM)
ifeq ($(READLINE),1)
LDFLAGS += -lreadline
endif
else
# Otherwise, on WASM we use a different target suffix:
TARGET_EXEC := tarski.js
LDFLAGS += -s "EXTRA_EXPORTED_RUNTIME_METHODS=['cwrap']" -fwasm-exceptions -s ALLOW_MEMORY_GROWTH=1 -s SINGLE_FILE=1 -s MODULARIZE=1 -s EXPORT_NAME=Tarski -O3
endif

# On Mac and Windows MSYS2/CLANG* the rt library is not present, but otherwise it is necessary.
ifneq ($(PLATFORM),MAC)
ifneq ($(PLATFORM),WINDOWS)
LDFLAGS += -lrt
ifdef STATIC
LIBCRYPTOA := $(shell find /usr/lib -name libcrypto.a)
LDFLAGS += -pthread -static \
 $(LIBCRYPTOA) -static-libgcc -static-libstdc++
endif
endif
endif

# On Windows we need a bit different options for a static build.
ifeq ($(PLATFORM),WINDOWS)
ifdef STATIC
MSYS2_ENV := $(shell echo ${MSYSTEM} | tr CLANG clang)
LIBCRYPTOA := /${MSYS2_ENV}/lib/libcrypto.a
LDFLAGS += -static \
 $(LIBCRYPTOA) -static-libgcc -static-libstdc++
endif
endif

# When built statically on Linux, we need tinfo if readline is present.
ifdef STATIC
ifeq ($(READLINE),1)
LDFLAGS += -ltinfo
endif
endif

# Set/unset readline support
ifeq ($(READLINE),0)
CXXFLAGS += -DNO_READLINE
endif

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS)) $(extIncludesSACLIB) $(extIncludeMINISAT) $(extIncludesMD5)

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -g -O3
# On Mac the C++11 dialect must be set, on Linux it is optional, on Windows MSYS2/CLANG* it should not be set.
ifeq ($(PLATFORM),MAC)
CPPFLAGS += --std=c++11
else
ifneq ($(PLATFORM),WINDOWS)
CPPFLAGS += --std=gnu++14
endif
endif

# By default build the target executable
ifneq ($(PLATFORM),WASM)
all: $(BUILD_DIR)/$(TARGET_EXEC)
else
# On WebAssembly platform, build also advanced versions of the .js, too
all: $(BUILD_DIR)/$(TARGET_EXEC) $(BUILD_DIR)/tarski-advanced.js $(BUILD_DIR)/tarski-jt-advanced.js
endif

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	test -r build/tarski && cp build/tarski bin/tarski || true

lib: tarski.a

tarski.a: $(AOBJ)
	$(AR) r $@ $(AOBJ)
	mv tarski.a ../lib/tarski.a

############################
### JNI DLL related code ###
############################

SWIG := src/swig
LDFLAGS_SHARED = $(filter-out -static, $(LDFLAGS))

# Output file on Linux:
LIBTARSKI:=libtarski.so

# Output file on Mac:
ifeq ($(PLATFORM),MAC)
LIBTARSKI:=libtarski.jnilib
endif

# Output file on Windows, add .def for MSYS2/CLANG32 (necessary for CLANG32, not harmful for CLANG64):
ifeq ($(PLATFORM),WINDOWS)
LIBTARSKI:=tarski.dll
DEF:=$(SWIG)/tarski.def
endif

dll: $(LIBTARSKI)

$(SWIG)/tarski_wrap.cxx $(SWIG)/tarski.java $(SWIG)/tarskiJNI.java:
	cd $(SWIG); swig -c++ -java -package tarski tarski.i
$(SWIG)/tarski_wrap.o: src/swig/tarski_wrap.cxx
	$(CXX) -c $(SWIG)/tarski_wrap.cxx -Isrc/main -Isrc -I$(SACLIB)/include \
        -I"$(JAVA)/include" -I"$(JAVA)/include/linux" -I"$(JAVA)/include/win32" -I"$(JAVA)/include/darwin" -fPIC
	mv tarski_wrap.o $(SWIG)/
$(LIBTARSKI): $(OBJS) $(SWIG)/tarski_wrap.o
	$(CXX) -shared $(SWIG)/tarski_wrap.o $(OBJS) $(DEF) -o $@ $(LDFLAGS_SHARED) -fPIC
$(SWIG)/main.class: $(SWIG)/main.java $(SWIG)/tarski.java $(SWIG)/tarskiJNI.java
	cd $(SWIG); "$(JAVA)/bin/javac" -d . main.java tarski.java tarskiJNI.java
dlltest: $(SWIG)/main.class $(LIBTARSKI)
	cd $(SWIG); "$(JAVA)/bin/java" -Djava.library.path=../.. tarski.main
$(SWIG)/JTarski.class: $(SWIG)/JTarski.java $(SWIG)/tarski.java $(SWIG)/tarskiJNI.java
	cd $(SWIG); "$(JAVA)/bin/javac" -d . JTarski.java tarski.java tarskiJNI.java
jtarski: $(SWIG)/JTarski.class $(LIBTARSKI)
	cd $(SWIG); "$(JAVA)/bin/java" -Djava.library.path=../.. tarski.JTarski

###############################
### End of DLL related code ###
###############################

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

CXXFLAGS += -fPIC
ifeq ($(PLATFORM),WASM)
CXXFLAGS += -s USE_ZLIB=1 -D_EMCC2_ -s EXPORTED_FUNCTIONS=[_TARSKIINIT,_TARSKIEVAL,_TARSKIEND] $(extIncludesMd5)
CXXFLAGS += -s "EXTRA_EXPORTED_RUNTIME_METHODS=['cwrap']" -fwasm-exceptions -s ALLOW_MEMORY_GROWTH=1
endif

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Create an advanced version of the .js file with proper redirection of stdout/stderr.
# See https://emscripten.org/docs/api_reference/module.html#creating-the-module-object
$(BUILD_DIR)/tarski-advanced.js: $(BUILD_DIR)/tarski.js
	sed 's/Module\["print"\]/\
 function (message) {\
        var outputtext = document.getElementById("outputtext");\
        outputtext.innerHTML += "<i>" + message.replaceAll("<", "\&lt;") + "<\/i><br>";\
        }\
/ ; s/Module\["printErr"\]/\
 function (message) {\
        var outputtext = document.getElementById("outputtext");\
        outputtext.innerHTML += "<i>" + message.replaceAll("<", "\&lt;") + "<\/i><br>";\
        }\
/' $< > $@

# Create another advanced version of the .js file with proper redirection of stdout/stderr (jquery.terminal variant).
$(BUILD_DIR)/tarski-jt-advanced.js: $(BUILD_DIR)/tarski.js
	sed 's/Module\["print"\]/\
 function (message) {\
        if (typeof tarski_term !== "undefined") tarski_term.echo("%noformat%[[i;gray;]" + $$.terminal.escape_formatting(message) + "]");\
        }\
/ ; s/Module\["printErr"\]/\
 function (message) {\
        if (typeof tarski_term !== "undefined") tarski_term.echo("%noformat%[[b;red;]" + $$\.terminal.escape_formatting(message) + "]");\
        }\
/' $< > $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
