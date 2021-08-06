SHELL := /bin/bash

# Directories relative paths.
ROOT_DIR := ../
INCLUDE_DIR := $(ROOT_DIR)include/
TEST_DIR := $(ROOT_DIR)test/
OBJ_DIR := $(ROOT_DIR)obj/
BIN_DIR := $(ROOT_DIR)bin/

# Groups of files and filenames.
VERSION_BUILD_INCLUDE := $(INCLUDE_DIR)rain/build.hpp

# All of rain's include fules, except for the build file.
RAIN_INCLUDES := $(filter-out $(VERSION_BUILD_INCLUDE), \
	$(shell find $(INCLUDE_DIR) -name *.hpp))

TEST_SRCS := $(wildcard $(TEST_DIR)*.cpp)
TEST_NAMES := $(basename $(notdir $(wildcard $(TEST_DIR)*.cpp)))
TEST_OBJS := $(join $(addsuffix /, \
	$(addprefix $(OBJ_DIR), $(TEST_NAMES))), $(addsuffix .o, $(TEST_NAMES)))
TEST_BINS := $(addprefix $(BIN_DIR), $(TEST_NAMES))

# Flags for build. Debug by default.
CXX := g++
CXXFLAGS_COMMON := -I$(INCLUDE_DIR) -std=c++17 -pthread \
	-Wall -Wextra -Wno-overloaded-virtual -D _CONSOLE -m64 -pedantic
CXXFLAGS_DEBUG := -D _DEBUG -g -O0
CXXFLAGS_RELEASE := -D NDEBUG -Ofast
CXXFLAGS_INSTRUMENT := -fsanitize=address,undefined,leak
LDLIBS := -lresolv

# Default to DEBUG builds. Check if RELEASE=1 is set.
ifeq ($(RELEASE), 1)
  CXXFLAGS := $(CXXFLAGS_COMMON) $(CXXFLAGS_RELEASE)
else
  CXXFLAGS := $(CXXFLAGS_COMMON) $(CXXFLAGS_DEBUG)
endif

ifeq ($(INSTRUMENT), 1)
  CXXFLAGS := $(CXXFLAGS) $(CXXFLAGS_INSTRUMENT)
endif

# -lstdc++fs is included by default on MacOS and cannot be specified on the \
	command line.
UNAME_S := $(shell uname -s)
ifneq ($(UNAME_S), Darwin)
	LDLIBS := $(LDLIBS) -lstdc++fs
endif

# Keep intermediate and output files.
.SECONDARY: $(TEST_BINS) $(TEST_OBJS)

# Mark .PHONY for rules to avoid compiling files named the same as rules.

# Build and run all tests in order.
.PHONY: runall
runall: $(TEST_NAMES)
	@cd $(BIN_DIR); \
	for file in $(TEST_NAMES); do \
		printf "\n-------- $$file --------\n"; \
		./$$file; \
	done

# Run a single test whose name is specified by TEST.
.PHONY: run
run: $(TEST)
	@$(BIN_DIR)$(TEST) $(ARGS)

# Build all tests.
.PHONY: all
all: $(TEST_NAMES)

# Delete intermediate files.
.PHONY: clean
clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

# Rules for single tests just redirect to test binaries.
# Enable second expansion for some fancy pattern matching later.
.SECONDEXPANSION:
$(TEST_NAMES): $(VERSION_BUILD_INCLUDE) $(BIN_DIR)$$@

# Static rules to nest one directory deep inside ../obj.
# Link rule.
$(TEST_BINS): $(BIN_DIR)% : $(OBJ_DIR)$$*/$$*.o
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $< $(LDLIBS) -o $@

# Compile rule. Stem % matches TEST_NAME/TEST_NAME, in which / is replaced \
	with a space and the first word is extracted for the cpp file name.
$(TEST_OBJS): $(OBJ_DIR)%.o : $(TEST_DIR)$$(firstword $$(subst /, ,$$*)).cpp \
	$(RAIN_INCLUDES)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Increment version build number.
$(VERSION_BUILD_INCLUDE): $(RAIN_INCLUDES) $(TEST_SRCS)
	@read line < $(VERSION_BUILD_INCLUDE); \
	TOKENS=($$line); \
	RAIN_VERSION_BUILD=$$(($${TOKENS[2]} + 1)); \
	echo -e "$${TOKENS[0]} $${TOKENS[1]} $$RAIN_VERSION_BUILD\c" > \
		$(VERSION_BUILD_INCLUDE); \
	echo VERSION_BUILD: $$RAIN_VERSION_BUILD.
