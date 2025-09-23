# Simple Makefile for JsonDiffPatch

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
INCLUDES = -Iinclude -Ithirdparty

# Source files
SRCDIR = src
SOURCES = $(SRCDIR)/JsonDiffPatch.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Library
LIBNAME = libJsonDiffPatch.a
SHARED_LIBNAME = libJsonDiffPatch.so

# Example
EXAMPLE_SRC = examples/basic_example.cpp
EXAMPLE_BIN = basic_example

.PHONY: all clean example

all: $(LIBNAME) $(SHARED_LIBNAME)

# Static library
$(LIBNAME): $(OBJECTS)
	ar rcs $@ $^

# Shared library
$(SHARED_LIBNAME): $(OBJECTS)
	$(CXX) -shared -o $@ $^

# Object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Example
example: $(EXAMPLE_BIN)

$(EXAMPLE_BIN): $(EXAMPLE_SRC) $(LIBNAME)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< -L. -lJsonDiffPatch

clean:
	rm -f $(OBJECTS) $(LIBNAME) $(SHARED_LIBNAME) $(EXAMPLE_BIN)

install: $(LIBNAME) $(SHARED_LIBNAME)
	@echo "Install target not implemented. Please copy files manually:"
	@echo "  Library: $(LIBNAME) and $(SHARED_LIBNAME)"
	@echo "  Headers: include/JsonDiffPatch/JsonDiffPatch.h"