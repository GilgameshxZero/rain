# Directories relative paths.
ROOT_DIR = ../
INCLUDE_DIR := $(ROOT_DIR)include/
SRC_DIR := $(ROOT_DIR)src/
OBJ_DIR := $(ROOT_DIR)obj/
BIN_DIR := $(ROOT_DIR)bin/
TEST_DIR := $(ROOT_DIR)test/

# Groups of files and filenames.
EMILIA_INCLUDE := $(wildcard $(INCLUDE_DIR)*)
EMILIA_NAME := $(basename $(notdir $(wildcard $(SRC_DIR)*.cpp)))
EMILIA_OBJ := $(addsuffix .o, $(addprefix $(OBJ_DIR), $(EMILIA_NAME)))

TEST_NAME := $(basename $(notdir $(wildcard $(TEST_DIR)*.cpp)))
TEST_BIN := $(addprefix $(BIN_DIR), $(TEST_NAME))

VERSION_BUILD_INCLUDE := $(INCLUDE_DIR)rain-version-build.hpp

# Flags for build. Release by default.
CXX := g++
CXXFLAG := -I$(INCLUDE_DIR) -std=c++17 -g -Wall -pthread
CXXFLAG_DEBUG := -O0
CXXFLAG_RELEASE := -O3 -march=native

# Check if DEBUG=1 is set.
ifeq ($(DEBUG),1)
  CXXFLAG := $(CXXFLAG) $(CXXFLAG_DEBUG)
else
  CXXFLAG := $(CXXFLAG) $(CXXFLAG_RELEASE)
endif

# Makefile definitions to avoid any file naming conflicts.
%:
	@:

.SECONDARY: $(EMILIA_OBJ)
.PHONY: testall
.PHONY: test
.PHONY: clean

# Compile and run all tests.
testall: increment-build $(TEST_BIN)
	@for x in $(TEST_BIN); do \
		echo $$x; \
		./$$x; \
	done

# Compile and run single test.
test: increment-build
	@$(eval TEST:=$(filter-out $@,$(MAKECMDGOALS)))
	$(MAKE) $(BIN_DIR)$(TEST)
	./$(BIN_DIR)$(TEST)

# Rules for test binaries and objects.
$(BIN_DIR)%: $(OBJ_DIR)test/%.o $(EMILIA_OBJ) $(EMILIA_INCLUDE)
	@mkdir -p $(@D)
	$(CXX) -o $@ $< $(EMILIA_OBJ) $(CXXFLAG)
$(OBJ_DIR)test/%.o: $(TEST_DIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CXXFLAG)

# Delete compile-time files.
clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

# Rules for Emilia library objects.
$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CXXFLAG)

# Increment build number.
increment-build:
	@while read line || [ -n "$$line" ]; do \
		TOKENS=($$line); \
		RAIN_VERSION_BUILD=$$(($${TOKENS[2]} + 1)); \
		echo -e "$${TOKENS[0]} $${TOKENS[1]} $$RAIN_VERSION_BUILD\c" > $(VERSION_BUILD_INCLUDE); \
	done < $(VERSION_BUILD_INCLUDE); \
	echo build $$RAIN_VERSION_BUILD
