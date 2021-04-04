SHELL := /bin/bash

# Directories relative paths.
ROOT_DIR := ../
INCLUDE_DIR := $(ROOT_DIR)include/
OBJ_DIR := $(ROOT_DIR)obj/
BIN_DIR := $(ROOT_DIR)bin/
TEST_DIR := $(ROOT_DIR)test/

# Groups of files and filenames.
VERSION_BUILD_INCLUDE := $(INCLUDE_DIR)rain/build.hpp

# All of rain's include fules, except for the build file.
RAIN_INCLUDES := $(filter-out $(VERSION_BUILD_INCLUDE), \
	$(shell find $(INCLUDE_DIR) -name *.hpp))

TEST_SRCS := $(wildcard $(TEST_DIR)*.cpp)
TEST_NAMES := $(basename $(notdir $(wildcard $(TEST_DIR)*.cpp)))
TEST_OBJS := $(addsuffix .o, $(addprefix $(OBJ_DIR), $(TEST_NAMES)))
TEST_BINS := $(addprefix $(BIN_DIR), $(TEST_NAMES))

# Flags for build. Debug by default.
CXX := g++
CXXFLAGS := -I$(INCLUDE_DIR) -std=c++17 -pthread -Wall
CXXFLAGS_DEBUG := -O0 -g
CXXFLAGS_RELEASE := -O3 # -march=native
LDLIBS := -lresolv

# OS-specific behavior.
UNAME_S := $(shell uname -s)
ifneq ($(UNAME_S), Darwin)
	LDLIBS := $(LDLIBS) -lstdc++fs
endif

# Check if RELEASE=1 is set.
ifeq ($(RELEASE), 1)
  CXXFLAGS := $(CXXFLAGS) $(CXXFLAGS_RELEASE)
else
  CXXFLAGS := $(CXXFLAGS) $(CXXFLAGS_DEBUG)
endif

# Keep intermediate files to speed up compilation times.
.SECONDARY: $(TEST_BINS) $(TEST_OBJS) $(TEST_NAMES)

# Mark .PHONY for rules to avoid compiling files named the same as rules.

# Build all tests.
.PHONY: all
all: $(TEST_NAMES)

.PHONY: noinc
noinc: $(TEST_BINS)

# Run tests.
.PHONY: run
run: $(TEST)
	@$(BIN_DIR)$(TEST) $(ARGS)

# Working directory should be consistently bin/.
.PHONY: runall
runall: $(TEST_NAMES)
	@cd $(BIN_DIR); \
	for file in $(TEST_NAMES); do \
		printf "\n-------- $$file --------\n"; \
		./$$file; \
	done

# Rules for single tests just redirect to test binaries.
.SECONDEXPANSION:
$(TEST_NAMES): $(VERSION_BUILD_INCLUDE) $(BIN_DIR)$$@

# Rules for test binaries and objects.
$(BIN_DIR)%: $(OBJ_DIR)%.o
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(LDLIBS) $< -o $@
$(OBJ_DIR)%.o: $(TEST_DIR)%.cpp $(RAIN_INCLUDES)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Delete intermediate files.
.PHONY: clean
clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

# Increment build number.
$(VERSION_BUILD_INCLUDE): $(RAIN_INCLUDES) $(TEST_SRCS)
	@read line < $(VERSION_BUILD_INCLUDE); \
	TOKENS=($$line); \
	RAIN_VERSION_BUILD=$$(($${TOKENS[2]} + 1)); \
	echo -e "$${TOKENS[0]} $${TOKENS[1]} $$RAIN_VERSION_BUILD\c" > \
		$(VERSION_BUILD_INCLUDE); \
	echo build $$RAIN_VERSION_BUILD
