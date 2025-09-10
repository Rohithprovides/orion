# Orion Compiler Makefile

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -lm 

# Source files
SOURCES = main.cpp lexer.cpp parser.cpp types.cpp codegen.cpp ast_impl.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = orion

# Default target
all: $(TARGET)

# Build the compiler
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile individual source files
%.o: %.cpp ast.h lexer.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Install the compiler (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Uninstall the compiler
uninstall:
	rm -f /usr/local/bin/$(TARGET)

# Run tests (placeholder)
test: $(TARGET)
	@echo "Running tests..."
	@echo "Tests would go here"

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# Profile build
profile: CXXFLAGS += -pg
profile: LDFLAGS += -pg
profile: $(TARGET)

# Dependencies
main.o: main.cpp ast.h lexer.h
lexer.o: lexer.cpp lexer.h
parser.o: parser.cpp ast.h lexer.h
types.o: types.cpp ast.h
codegen.o: codegen.cpp ast.h
ast_impl.o: ast_impl.cpp ast.h

.PHONY: all clean install uninstall test debug profile