# Compiler flags
CXX ?= g++
LD ?= g++

INCLUDE_DIRS := lib client/include server/include client/src server/src
INCLUDES := $(addprefix -I,$(INCLUDE_DIRS))
SOURCES := $(shell find . -name "*.cpp")
HEADERS := $(shell find . -name "*.h")
OBJECTS := $(SOURCES:.cpp=.o)
TARGET_EXECS := client/bin/player server/bin/GS

# VPATH is a variable used by Makefile which finds *sources* and makes them available throughout the codebase
# vpath %.h <DIR> tells make to look for header files in <DIR>
vpath # clears VPATH
vpath %.h $(INCLUDE_DIRS)

CXXFLAGS = -std=c++20 -O3
CXXFLAGS += $(INCLUDES)
# Warnings
CXXFLAGS += -fdiagnostics-color=always -Wall  -Wextra -Wcast-align -Wconversion -Wfloat-equal -Wformat=2 -Wnull-dereference -Wshadow -Wsign-conversion -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused -Wno-sign-compare
# Differentiate between dev and prod
CXXFLAGS += -DPRODUCTION

.PHONY: all dev clean fmt depend test

all: $(TARGET_EXECS)

# In development, we want the rev command to be answered with the actual word
# Moreover, in dev mode, the file read order (server-side) will be sequential
dev: CXXFLAGS := $(filter-out -DPRODUCTION,$(CXXFLAGS))
dev: all

clean:
	rm -f $(OBJECTS) $(TARGET_EXECS)
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
	$(CC) $(INCLUDES) -MM $^ > autodep

test: dev
	./test.sh

client/bin/player: client/bin/client-api.o client/bin/client-tcp.o client/bin/client-udp.o client/bin/client-sockets.o lib/common.o
server/bin/GS: server/bin/server-utils.o server/bin/server-api.o server/bin/server-tcp.o server/bin/server-udp.o server/bin/server-sockets.o lib/common.o
