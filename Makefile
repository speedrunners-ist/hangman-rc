# Compiler flags
CC ?= g++
LD ?= g++

INCLUDE_DIRS := src src/client src/server
INCLUDES = $(addprefix -I, $(INCLUDE_DIRS))

SOURCES  := $(wildcard */*.cpp)
HEADERS  := $(wildcard */*.h)
OBJECTS  := $(SOURCES:.c=.o)
TARGET_EXECS := src/client/hangman-client-api src/server/hangman-server-api

# VPATH is a variable used by Makefile which finds *sources* and makes them available throughout the codebase
# vpath %.h <DIR> tells make to look for header files in <DIR>
vpath # clears VPATH
vpath %.h $(INCLUDE_DIRS)

CXXFLAGS = -std=c++20 -O3
CXXFLAGS += $(INCLUDES)
# Warnings
CXXFLAGS += -fdiagnostics-color=always -Wall -Werror -Wextra -Wcast-align -Wconversion -Wfloat-equal -Wformat=2 -Wnull-dereference -Wshadow -Wsign-conversion -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused
CXXFLAGS += -Wno-sign-compare

ifneq ($(strip $(DEBUG)), no)
  CXXFLAGS += -g
endif

.PHONY: all clean fmt

all: $(TARGET_EXECS)

clean:
	rm -f $(OBJECTS) $(TARGET_EXECS)

fmt: $(SOURCES) $(HEADERS)
	clang-format -i $^
