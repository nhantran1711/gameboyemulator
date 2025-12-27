# Compilers
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -IC:/SDL3-devel-3.2.28-mingw/SDL3-3.2.28/x86_64-w64-mingw32/include
LDFLAGS  = -LC:/SDL3-devel-3.2.28-mingw/SDL3-3.2.28/x86_64-w64-mingw32/lib -lmingw32 -lSDL3

# Output executable
OUTPUT = build/chip8.exe

# Default target
all:
	g++ chip8.cpp -o $(OUTPUT) $(CXXFLAGS) $(LDFLAGS)


# Clean build
clean:
	del $(OUTPUT)