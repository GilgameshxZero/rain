SHELL := /bin/bash

# Directories relative paths.
ROOT_DIR := ../
INCLUDE_DIR := $(ROOT_DIR)include/
SRC_DIR := $(ROOT_DIR)src/
OBJ_DIR := $(ROOT_DIR)obj/
BIN_DIR := $(ROOT_DIR)bin/
TEST_DIR := $(ROOT_DIR)test/

# Groups of files and filenames.
VERSION_BUILD_INCLUDE := $(INCLUDE_DIR)rain/build.hpp

# All of rain's include fules, except for the build file.
RAIN_INCLUDE := $(filter-out $(VERSION_BUILD_INCLUDE), \
	$(shell find $(INCLUDE_DIR) -name *.hpp))

TEST_NAME := $(basename $(notdir $(wildcard $(TEST_DIR)*.cpp)))
TEST_OBJ := $(addsuffix .o, $(addprefix $(OBJ_DIR)test/, $(TEST_NAME)))
TEST_BIN := $(addprefix $(BIN_DIR), $(TEST_NAME))

# Flags for build. Release by default.
CXX := g++
CXXFLAG := -I$(INCLUDE_DIR) -std=c++17 -g -Wall -pthread -lresolv
CXXFLAG_DEBUG := -O0
CXXFLAG_RELEASE := -O3 -march=native

# OS-specific behavior.
UNAME_S := $(shell uname -s)
ifneq ($(UNAME_S), Darwin)
	CXXFLAG := $(CXXFLAG) -lstdc++fs
endif

# Check if DEBUG=1 is set.
ifeq ($(DEBUG), 1)
  CXXFLAG := $(CXXFLAG) $(CXXFLAG_DEBUG)
else
  CXXFLAG := $(CXXFLAG) $(CXXFLAG_RELEASE)
endif

# Keep intermediate files to speed up compilation times.
.SECONDARY: $(TEST_OBJ) $(TEST_BIN)

# Mark .PHONY for rules to avoid compiling files named the same as rules.

# Compile and run all tests.
.PHONY: all
all: $(VERSION_BUILD_INCLUDE) $(TEST_NAME)

# Rules for single tests just redirect to test binaries.
%: $(VERSION_BUILD_INCLUDE) $(BIN_DIR)%
	@echo $@
	@./$(BIN_DIR)$@

# Rules for test binaries and objects.
$(BIN_DIR)%: $(OBJ_DIR)test/%.o
	@mkdir -p $(@D)
	$(CXX) -o $@ $< $(CXXFLAG)
$(OBJ_DIR)test/%.o: $(TEST_DIR)%.cpp $(RAIN_INCLUDE)
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CXXFLAG)

# Delete intermediate files.
.PHONY: clean
clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

# Increment build number.
$(VERSION_BUILD_INCLUDE): $(RAIN_INCLUDE)
	@read line < $(VERSION_BUILD_INCLUDE); \
	TOKENS=($$line); \
	RAIN_VERSION_BUILD=$$(($${TOKENS[2]} + 1)); \
	echo -e "$${TOKENS[0]} $${TOKENS[1]} $$RAIN_VERSION_BUILD\c" > \
		$(VERSION_BUILD_INCLUDE); \
	echo build $$RAIN_VERSION_BUILD
