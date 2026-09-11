# Compiler configuration
CXX := clang++
CXXFLAGS := -Wall -Wextra -std=c++20

# Source and binary filenames
SRC := main.cc
OBJ := main.o
BIN := main

# Targets
.PHONY: all asan clean

all: $(BIN)

# Build and run with AddressSanitizer and UndefinedBehaviorSanitizer, which
# report use-after-free, double-destroy etc. immediately.
asan: $(SRC)
	$(CXX) $(CXXFLAGS) -g -fsanitize=address,undefined -o $(BIN)_asan $<
	./$(BIN)_asan

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ): $(SRC)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(BIN) $(OBJ) $(BIN)_asan $(BIN)_asan.dSYM

