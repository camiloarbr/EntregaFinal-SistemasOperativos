CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread
SRC = src/gsea.cpp src/main.cpp
OBJ = $(SRC:.cpp=.o)
BIN = gsea

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: test

test: $(BIN)
	@echo "Running basic tests..."
	@mkdir -p tests/out
	@cp tests/data/sample.txt tests/out/
	@./$(BIN) compress -i tests/out/sample.txt -o tests/out/sample.txt.rle
	@./$(BIN) decompress -i tests/out/sample.txt.rle -o tests/out/sample_decompressed.txt
	@if cmp -s tests/out/sample.txt tests/out/sample_decompressed.txt; then \
		echo "[OK] compress/decompress roundtrip passed"; \
	else \
		echo "[FAIL] roundtrip mismatch"; exit 1; \
	fi
	@./$(BIN) encrypt -i tests/out/sample.txt -o tests/out/sample.txt.enc -k secret
	@./$(BIN) decrypt -i tests/out/sample.txt.enc -o tests/out/sample.dec -k secret
	@if cmp -s tests/out/sample.txt tests/out/sample.dec; then \
		echo "[OK] encrypt/decrypt roundtrip passed"; \
	else \
		echo "[FAIL] encrypt/decrypt mismatch"; exit 1; \
	fi
	@echo "All basic tests passed."
