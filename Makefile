CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pthread
SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:src/%.cpp=build/%.o)
BIN := bin/gsea

all: $(BIN)

$(BIN): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^

build/%.o: src/%.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build $(BIN)

.PHONY: all clean

# Note: tests and more targets will be added later.
