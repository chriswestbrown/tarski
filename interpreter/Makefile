# Master Makefile for the Tarski Intepreter
# Following the blog post by Job Vranish
# https://spin.atomicobject.com/2016/08/26/makefile-c-projects/

CXX=g++
CC=g++
MROOT=-I../minisat
TMROOT=../minisat
extIncludesSACLIB:=-I${saclib}/include
extLibSACLIB:=${saclib}/lib/saclibo.a
extLibMINISAT:=../minisat/core/lib.a
extIncludeMINISAT:=-I${TMROOT}/core
extIncludesMD5:=
extLibMD5:=-lssl  -lcrypto

TARGET_EXEC ?= tarski

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

SRCS := $(sort $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s))
ASRC := $(filter-out */shell.cpp, $(sort $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)))
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
AOBJ := $(ASRC:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

LDFLAGS := -lssl -lcrypto -lreadline -lrt $(extLibSACLIB) $(extLibMINISAT) 

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS)) $(extIncludesSACLIB) $(extIncludeMINISAT)

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP --std=c++11 -g


$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	mv build/tarski bin/tarski


lib: tarski.a

tarski.a: $(AOBJ)
	$(AR) r $@ $(AOBJ)
	mv tarski.a ../lib/tarski.a


# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
