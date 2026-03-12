CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=199309L -Iinclude
LDFLAGS = -pthread

# Nombres de los ejecutables
TARGET = simulator
TEST_TARGET = test_simulator

# Directorios
BUILD_DIR = .build
SRC_DIR = src
TEST_DIR = tests

SRCS = $(wildcard $(SRC_DIR)/*.c)

CORE_SRCS = $(filter-out $(SRC_DIR)/main.c, $(SRCS))

CORE_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CORE_SRCS))
MAIN_OBJ = $(BUILD_DIR)/main.o

# Phony evita conflictos con archivos que se llamen igual que comandos
.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(CORE_OBJS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_TARGET): $(CORE_OBJS) $(TEST_DIR)/test_simulator.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_TARGET)

run: all
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)