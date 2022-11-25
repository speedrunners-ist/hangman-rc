# Compiler flags
CC ?= g++
LD ?= g++

INCLUDE_DIRS := client server .
INCLUDES = $(addprefix -I, $(INCLUDE_DIRS))

SOURCES  := $(wildcard */*.c)
HEADERS  := $(wildcard */*.h)
OBJECTS  := $(SOURCES:.c=.o)
TARGET_EXECS := $(SOURCES:.c=)

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

clean:
	rm -f $(OBJECTS) $(TARGET_EXECS)

fmt: $(SOURCES) $(HEADERS)
	clang-format -i $^
