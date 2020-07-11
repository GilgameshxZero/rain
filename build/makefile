SHELL := /bin/bash

# Directories relative paths.
ROOT_DIR := ../
INCLUDE_DIR := $(ROOT_DIR)include/
SRC_DIR := $(ROOT_DIR)src/
OBJ_DIR := $(ROOT_DIR)obj/
BIN_DIR := $(ROOT_DIR)bin/
TEST_DIR := $(ROOT_DIR)test/

# Groups of files and filenames.
RAIN_INCLUDE := $(wildcard $(INCLUDE_DIR)*)
RAIN_NAME := $(basename $(notdir $(wildcard $(SRC_DIR)*.cpp)))
RAIN_OBJ := $(addsuffix .o, $(addprefix $(OBJ_DIR)rain/, $(RAIN_NAME)))

TEST_NAME := $(basename $(notdir $(wildcard $(TEST_DIR)*.cpp)))
TEST_OBJ := $(addsuffix .o, $(addprefix $(OBJ_DIR)test/, $(TEST_NAME)))
TEST_BIN := $(addprefix $(BIN_DIR), $(TEST_NAME))

VERSION_BUILD_INCLUDE := $(INCLUDE_DIR)rain/build.hpp

# Flags for build. Release by default.
CXX := g++
CXXFLAG := -I$(INCLUDE_DIR) -std=c++17 -g -Wall -pthread
CXXFLAG_DEBUG := -O0
CXXFLAG_RELEASE := -O3 -march=native

# Check if DEBUG=1 is set.
ifeq ($(DEBUG), 1)
  CXXFLAG := $(CXXFLAG) $(CXXFLAG_DEBUG)
else
  CXXFLAG := $(CXXFLAG) $(CXXFLAG_RELEASE)
endif

# Keep intermediate files to speed up compilation times.
.SECONDARY: $(RAIN_OBJ) $(TEST_OBJ) $(TEST_BIN)

# Mark .PHONY for rules to avoid compiling files named the same as rules.

# Compile and run all tests.
.PHONY: all
all: increment-build $(TEST_NAME)

# Rules for single tests just redirect to test binaries.
%: increment-build $(BIN_DIR)%
	./$(BIN_DIR)$@

# Rules for test binaries and objects.
$(BIN_DIR)%: $(OBJ_DIR)test/%.o $(RAIN_OBJ) $(RAIN_INCLUDE)
	@mkdir -p $(@D)
	$(CXX) -o $@ $< $(RAIN_OBJ) $(CXXFLAG)
$(OBJ_DIR)test/%.o: $(TEST_DIR)%.cpp $(RAIN_INCLUDE)
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CXXFLAG)

# Delete compile-time files.
.PHONY: clean
clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

# Rules for rain library objects.
$(OBJ_DIR)rain/%.o: $(SRC_DIR)%.cpp $(RAIN_INCLUDE)
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CXXFLAG)

# Increment build number.
.PHONY: increment-build
increment-build:
	@read line < $(VERSION_BUILD_INCLUDE); \
	TOKENS=($$line); \
	RAIN_VERSION_BUILD=$$(($${TOKENS[2]} + 1)); \
	echo -e "$${TOKENS[0]} $${TOKENS[1]} $$RAIN_VERSION_BUILD\c" > \
		$(VERSION_BUILD_INCLUDE); \
	echo build $$RAIN_VERSION_BUILD
