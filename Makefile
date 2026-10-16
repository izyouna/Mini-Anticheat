CC ?= gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2 -Iinclude -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -D_DARWIN_C_SOURCE

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
TEST_DIR = tests
SIM_DIR = simulator

SCANNER_SRCS = $(SRC_DIR)/process_scanner.c
DETECTOR_SRCS = $(SRC_DIR)/cheat_detector.c
LAUNCHER_SRCS = $(SRC_DIR)/game_launcher.c
LOGGER_SRCS = $(SRC_DIR)/logger.c
CORE_SRCS = $(SCANNER_SRCS) $(DETECTOR_SRCS) $(LAUNCHER_SRCS) $(LOGGER_SRCS)
MAIN_SRCS = $(SRC_DIR)/main.c $(CORE_SRCS)
MAIN_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(MAIN_SRCS))

TARGET = anticheat
GAME_TARGET = game
CHEAT_TARGET = cheat_test

TEST_SCANNER_BIN = $(BUILD_DIR)/test_scanner
TEST_DETECTOR_BIN = $(BUILD_DIR)/test_detector
TEST_SIMULATOR_BIN = $(BUILD_DIR)/test_simulators
TEST_LAUNCHER_BIN = $(BUILD_DIR)/test_launcher
TEST_LOGGER_BIN = $(BUILD_DIR)/test_logger

.PHONY: all clean test

all: $(TARGET) $(GAME_TARGET) $(CHEAT_TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR) logs

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(MAIN_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(GAME_TARGET): $(SIM_DIR)/game.c
	$(CC) $(CFLAGS) $< -o $@

$(CHEAT_TARGET): $(SIM_DIR)/cheat_test.c
	$(CC) $(CFLAGS) $< -o $@

test: $(TEST_SCANNER_BIN) $(TEST_DETECTOR_BIN) $(TEST_SIMULATOR_BIN) $(TEST_LAUNCHER_BIN) $(TEST_LOGGER_BIN) $(GAME_TARGET) $(CHEAT_TARGET)
	@echo "Running Process Scanner tests..."
	@./$(TEST_SCANNER_BIN)
	@echo ""
	@echo "Running Cheat Detector tests..."
	@./$(TEST_DETECTOR_BIN)
	@echo ""
	@echo "Running Simulator tests..."
	@./$(TEST_SIMULATOR_BIN)
	@echo ""
	@echo "Running Game Launcher tests..."
	@./$(TEST_LAUNCHER_BIN)
	@echo ""
	@echo "Running Logger tests..."
	@./$(TEST_LOGGER_BIN)

$(TEST_SCANNER_BIN): $(TEST_DIR)/test_scanner.c $(SCANNER_SRCS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_DETECTOR_BIN): $(TEST_DIR)/test_detector.c $(CORE_SRCS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_SIMULATOR_BIN): $(TEST_DIR)/test_simulators.c $(CORE_SRCS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_LAUNCHER_BIN): $(TEST_DIR)/test_launcher.c $(CORE_SRCS) $(GAME_TARGET) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_launcher.c $(CORE_SRCS) -o $@

$(TEST_LOGGER_BIN): $(TEST_DIR)/test_logger.c $(CORE_SRCS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_logger.c $(CORE_SRCS) -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(GAME_TARGET) $(CHEAT_TARGET)
