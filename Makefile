# Compilers
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra $(shell pkg-config --cflags sdl3)
LDFLAGS  = $(shell pkg-config --libs sdl3)

# Output executable
OUTPUT = build/chip8

# Default target
all:
	g++ chip8.cpp -o $(OUTPUT) $(CXXFLAGS) $(LDFLAGS)


# Clean build
clean:
	del $(OUTPUT)