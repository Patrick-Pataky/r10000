CC = clang
CFLAGS = -Wall -Wextra -std=c17 -Iinclude -g

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

TARGET = $(BIN_DIR)/sim

all: $(TARGET)

debug: CFLAGS += -DDEBUG
debug: run
	@echo "Debugging the program..."

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

run: clean all
	@echo "Running the program with test input..."
	./run.sh given_tests/$(TEST)/input.json res.json

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

analyze:
	scan-build make clean all

lint:
	clang-tidy $(SRCS) -- -Iinclude $(CFLAGS)

.PHONY: all clean analyze lint
