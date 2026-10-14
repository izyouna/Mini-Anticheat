CC ?= gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2 -Iinclude

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
TEST_DIR = tests

SCANNER_SRCS = $(SRC_DIR)/process_scanner.c
DETECTOR_SRCS = $(SRC_DIR)/cheat_detector.c
CORE_SRCS = $(SCANNER_SRCS) $(DETECTOR_SRCS)
MAIN_SRCS = $(SRC_DIR)/main.c $(CORE_SRCS)
MAIN_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(MAIN_SRCS))

TARGET = anticheat
TEST_SCANNER_BIN = $(BUILD_DIR)/test_scanner
TEST_DETECTOR_BIN = $(BUILD_DIR)/test_detector

.PHONY: all clean test

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(MAIN_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

test: $(TEST_SCANNER_BIN) $(TEST_DETECTOR_BIN)
	@echo "Running tests..."
	@./$(TEST_SCANNER_BIN)
	@echo ""
	@./$(TEST_DETECTOR_BIN)

$(TEST_SCANNER_BIN): $(TEST_DIR)/test_scanner.c $(SCANNER_SRCS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_DETECTOR_BIN): $(TEST_DIR)/test_detector.c $(CORE_SRCS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
