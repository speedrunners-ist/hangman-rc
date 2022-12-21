# Compiler flags
CXX ?= g++
LD ?= g++
SRC_EXT ?= cpp

# The following variables are used to define the source files, the header files, the object files and the executable files
INCLUDE_DIRS := client/include server/include lib
INCLUDES := $(addprefix -I, $(INCLUDE_DIRS))

BIN_DIR_CLIENT := client/bin
BIN_DIR_SERVER := server/bin
BIN_CLIENT = $(BIN_DIR_CLIENT)/player
BIN_SERVER = $(BIN_DIR_SERVER)/GS

OBJ_DIR_CLIENT := client/build
OBJ_DIR_SERVER := server/build
OBJ_DIR_LIB := lib/build

SRC_CLIENT := $(wildcard client/src/*.$(SRC_EXT))
SRC_SERVER := $(wildcard server/src/*.$(SRC_EXT))
SRC_LIB := $(wildcard lib/*.$(SRC_EXT))
SOURCES := $(SRC_CLIENT) $(SRC_SERVER) $(SRC_LIB)

HEADERS := $(wildcard client/include/*.h) $(wildcard server/include/*.h) $(wildcard lib/*.h)

OBJECTS_CLIENT = $(patsubst %.$(SRC_EXT), $(OBJ_DIR_CLIENT)/%.o, $(notdir $(SRC_CLIENT)))
OBJECTS_SERVER = $(patsubst %.$(SRC_EXT), $(OBJ_DIR_SERVER)/%.o, $(notdir $(SRC_SERVER)))
OBJECTS_LIB = $(patsubst %.$(SRC_EXT), $(OBJ_DIR_LIB)/%.o, $(notdir $(SRC_LIB)))

# VPATH is a variable used by Makefile which finds *sources* and makes them available throughout the codebase
# vpath %.h <DIR> tells make to look for header files in <DIR>
vpath # clears VPATH
vpath %.h $(INCLUDE_DIRS)

# CXXFLAGS is a variable used by the Makefile which defines the compiler flags

CXXFLAGS = -std=c++20 -O3
CXXFLAGS += $(INCLUDES)
# Warnings
CXXFLAGS += -fdiagnostics-color=always -Wall -Wextra -Wcast-align -Wconversion -Wfloat-equal -Wformat=2 -Wnull-dereference -Wshadow -Wsign-conversion -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused -Wno-sign-compare
# Differentiate between dev and prod
CXXFLAGS += -DPRODUCTION

.PHONY: all dev clean fmt depend test

# Defines the default target
all: $(BIN_CLIENT) $(BIN_SERVER)

# Defines pattern rules for building object files
$(OBJ_DIR_CLIENT)/%.o: client/src/%.$(SRC_EXT)
	@mkdir -p $(OBJ_DIR_CLIENT)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR_SERVER)/%.o: server/src/%.$(SRC_EXT)
	@mkdir -p $(OBJ_DIR_SERVER)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR_LIB)/%.o: lib/%.$(SRC_EXT)
	@mkdir -p $(OBJ_DIR_LIB)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Defines pattern rules for building executables
$(BIN_CLIENT): $(OBJECTS_CLIENT) $(OBJECTS_LIB)
	@echo "Linking $@"
	mkdir -p $(BIN_DIR_CLIENT)
	$(CXX) $^ -o $@

$(BIN_SERVER): $(OBJECTS_SERVER) $(OBJECTS_LIB)
	@echo "Linking $@"
	mkdir -p $(BIN_DIR_SERVER)
	$(CXX) $^ -o $@

# In development, we want the rev command to be answered with the actual word
# Moreover, in dev mode, the file read order (server-side) will be sequential
dev: CXXFLAGS := $(filter-out -DPRODUCTION,$(CXXFLAGS))
dev: all

# Remove all object files and executables + all generated files during the player and server's execution
clean:
	rm -f $(BIN_CLIENT) $(BIN_SERVER) $(OBJECTS_CLIENT) $(OBJECTS_SERVER) $(OBJECTS_LIB)
	rm -rf client/assets/hints
	rm -rf client/assets/state
	rm -rf client/assets/scoreboard
	rm -rf server/assets/games/*
	rm -rf tests/tmp

fmt: $(SOURCES) $(HEADERS)
	clang-format -i $^

# This generates a dependency file, with some default dependencies gathered from the include tree
# The dependencies are gathered in the file autodep. You can find an example illustrating this GCC feature, without Makefile, at this URL: https://renenyffenegger.ch/notes/development/languages/C-C-plus-plus/GCC/options/MM
# Run `make depend` whenever you add new includes in your files
depend : $(SOURCES)
	$(CXX) $(INCLUDES) -MM $^ > autodep

# In testing, we want to utilize the dev version of our project
test: dev
	./test.sh
