# Makefile for Split UNO Arbiter
# Compiles the arbiter application with proper flags and provides utility targets

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O2
DEBUGFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -g -O0
TARGET = split_uno_arbiter
SOURCE = arbiter.cpp
BACKUP = arbiter.cpp.backup

# Default target
all: $(TARGET)

# Build the release version
$(TARGET): $(SOURCE)
	@echo "Compiling Split UNO Arbiter (Release)..."
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)
	@echo "Build successful! Run with: ./$(TARGET)"

# Build debug version
debug: $(SOURCE)
	@echo "Compiling Split UNO Arbiter (Debug)..."
	$(CXX) $(DEBUGFLAGS) -o $(TARGET)_debug $(SOURCE)
	@echo "Debug build successful! Run with: ./$(TARGET)_debug"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(TARGET) $(TARGET)_debug
	@echo "Clean complete."

# Run the program
run: $(TARGET)
	@echo "Running Split UNO Arbiter..."
	./$(TARGET)

# Check for compilation warnings
strict: $(SOURCE)
	@echo "Compiling with strict warnings..."
	$(CXX) -std=c++17 -Wall -Wextra -Wpedantic -Werror -O2 -o $(TARGET) $(SOURCE)
	@echo "Strict build successful - no warnings!"

# Display help
help:
	@echo "Split UNO Arbiter - Makefile Targets:"
	@echo "  make          - Build release version"
	@echo "  make debug    - Build debug version with symbols"
	@echo "  make run      - Build and run the arbiter"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make strict   - Build with warnings as errors"
	@echo "  make help     - Show this help message"

.PHONY: all debug clean run strict help
