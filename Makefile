# Compiler flags
CC ?= g++
LD ?= g++

# space separated list of directories with header files
INCLUDE_DIRS := client server .
# this creates a space separated list of -I<dir> where <dir> is each of the values in INCLUDE_DIRS
INCLUDES = $(addprefix -I, $(INCLUDE_DIRS))

SOURCES  := $(wildcard */*.c)
HEADERS  := $(wildcard */*.h)
OBJECTS  := $(SOURCES:.c=.o)

# VPATH is a variable used by Makefile which finds *sources* and makes them available throughout the codebase
# vpath %.h <DIR> tells make to look for header files in <DIR>
vpath # clears VPATH
vpath %.h $(INCLUDE_DIRS)

CFLAGS = -std=c++11 -O3
CFLAGS += $(INCLUDES)

# Warnings
CFLAGS += -fdiagnostics-color=always -Wall -Werror

ifneq ($(strip $(DEBUG)), no)
  CFLAGS += -g
endif

.PHONY: all clean fmt

all: $(TARGET_EXECS)

fmt: $(SOURCES) $(HEADERS)
	clang-format -i $^
