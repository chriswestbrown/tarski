CXX=clang++ 
CC=clang++ 
MROOT=-I../minisat
extIncludesSACLIB:=-I${saclib}/include
extLibSACLIB:=${saclib}/lib/saclibo.a
extLibMINISAT:=../minisat/core/lib.a
extIncludeMINISAT:=-I${MROOT}/core
extIncludesMD5:=
extLibMD5:=-lssl  -lcrypto

TARGET_EXEC ?= tarski

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

LDFLAGS := -lssl -lcrypto -lreadline $(extLibSACLIB) $(extLibMINISAT) 

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS)) $(extIncludesSACLIB) 

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -g 

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	mv build/tarski shell/tarski

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
