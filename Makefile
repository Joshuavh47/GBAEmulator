# Compiler
CC = gcc

# Output binary name
TARGET = GBAEmu

# Directories
INCLUDES = -I/opt/homebrew/include
LIBS = -L/opt/homebrew/lib -lSDL3

# Source files
SRCS = $(wildcard *.c)

# Build flags
CFLAGS_RELEASE = -O2
CFLAGS_DEBUG = -g

# Default target
all: release

# Release build
release:
	$(CC) $(SRCS) $(INCLUDES) $(LIBS) $(CFLAGS_RELEASE) -o $(TARGET)

# Debug build (for lldb)
debug:
	$(CC) $(SRCS) $(INCLUDES) $(LIBS) $(CFLAGS_DEBUG) -o $(TARGET)

# Clean build artifacts
clean:
	rm -rf $(TARGET) $(TARGET).dSYM

.PHONY: all release debug clean
