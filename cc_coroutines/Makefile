# Compiler configuration
CXX := clang++
CXXFLAGS := -Wall -Wextra -std=c++20

# Source and binary filenames
SRC := main.cc
OBJ := main.o
BIN := main

# Targets
.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ): $(SRC)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(BIN) $(OBJ)

