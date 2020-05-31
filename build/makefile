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
testall: $(TEST_BIN)
	@for x in $(TEST_BIN) ; do echo $$x ; ./$$x ; done ;

# Compile and run single test.
test:
	@$(eval TEST=$(filter-out $@,$(MAKECMDGOALS)))
	$(MAKE) $(BIN_DIR)$(TEST)
	./$(BIN_DIR)$(TEST)

# Delete compile-time files.
clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

# Rules for test binaries and objects.
$(BIN_DIR)%: $(OBJ_DIR)test/%.o $(EMILIA_OBJ) $(EMILIA_INCLUDE)
	@mkdir -p $(@D)
	$(CXX) -o $@ $< $(EMILIA_OBJ) $(CXXFLAG)
$(OBJ_DIR)test/%.o: $(TEST_DIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CXXFLAG)

# Rules for Emilia library objects.
$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CXXFLAG)

