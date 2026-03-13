CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -Iinclude
LDFLAGS = -pthread

# Nombres de los ejecutables
TARGET = simulator

# Directorios
BUILD_DIR = .build
SRC_DIR = src
OUT_DIR = out

# Detección de archivos
SRCS = $(wildcard $(SRC_DIR)/*.c)
CORE_SRCS = $(filter-out $(SRC_DIR)/main.c, $(SRCS))
CORE_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CORE_SRCS))
MAIN_OBJ = $(BUILD_DIR)/main.o

# Lista de experimentos para el reproduce
EXPERIMENTS = exp1 exp2-notlb exp2-tlb exp3-1t exp3-8t

.PHONY: all clean run reproduce run-page-safe run-tlb-safe run-page-unsafe run-seg-safe run-seg-unsafe $(EXPERIMENTS)

all: $(TARGET)

# Compilación
$(TARGET): $(CORE_OBJS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Utilidad para crear carpeta de salida
$(OUT_DIR):
	@mkdir -p $(OUT_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(OUT_DIR)

# Pruebas con y sin concurrencia en page, seg y tlb

run: all $(OUT_DIR)
	./$(TARGET) --mode page --threads 4 --ops-per-thread 1000 --workload 80-20 --stats

run-page-safe: all $(OUT_DIR)
	./$(TARGET) --mode page --threads 4 --ops-per-thread 1000 --workload 80-20 --stats

run-page-unsafe: all $(OUT_DIR)
	./$(TARGET) --mode page --threads 4 --ops-per-thread 1000 --workload 80-20 --stats --unsafe

run-tlb-safe: all $(OUT_DIR)
	./$(TARGET) --mode page --threads 4 --ops-per-thread 1000 --workload 80-20 --tlb-size 16 --stats

run-seg-safe: all $(OUT_DIR)
	./$(TARGET) --mode seg --threads 2 --ops-per-thread 5000 --segments 4 --seg-limits 1024,2048,4096,8192 --stats

run-seg-unsafe: all $(OUT_DIR)
	./$(TARGET) --mode seg --threads 2 --ops-per-thread 5000 --segments 4 --seg-limits 1024,2048,4096,8192 --stats --unsafe



# Experimentos

exp1: all $(OUT_DIR)
	@echo "\n[TEST] Exp 1: Segmentación"
	./$(TARGET) --mode seg --threads 1 --workload uniform --ops-per-thread 10000 --segments 4 --seg-limits 1024,2048,4096,8192 --seed 100 --stats

exp2-notlb: all $(OUT_DIR)
	@echo "\n[TEST] Exp 2: Paginación sin TLB"
	./$(TARGET) --mode page --threads 1 --workload 80-20 --ops-per-thread 50000 --pages 128 --frames 64 --page-size 4096 --tlb-size 0 --tlb-policy fifo --seed 200 --stats

exp2-tlb: all $(OUT_DIR)
	@echo "\n[TEST] Exp 2: Paginación con TLB (Localidad)"
	./$(TARGET) --mode page --threads 1 --workload 80-20 --ops-per-thread 50000 --pages 128 --frames 64 --page-size 4096 --tlb-size 32 --tlb-policy fifo --seed 200 --stats

exp3-1t: all $(OUT_DIR)
	@echo "\n[TEST] Exp 3: Thrashing (1 Thread)"
	./$(TARGET) --mode page --threads 1 --workload uniform --ops-per-thread 10000 --pages 64 --frames 8 --page-size 4096 --tlb-size 16 --seed 300 --stats

exp3-8t: all $(OUT_DIR)
	@echo "\n[TEST] Exp 3: Thrashing (8 Threads - Stress Test)"
	./$(TARGET) --mode page --threads 8 --workload uniform --ops-per-thread 10000 --pages 64 --frames 8 --page-size 4096 --tlb-size 16 --seed 300 --stats

# REPRODUCCIÓN COMPLETA
reproduce: $(EXPERIMENTS)
	@echo "\n>> Todos los experimentos han finalizado. Revisa $(OUT_DIR)/summary.json"