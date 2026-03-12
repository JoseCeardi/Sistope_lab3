CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -Iinclude
LDFLAGS = -pthread

# Nombres de los ejecutables
TARGET = simulator
TEST_PAG = test_paginacion
TEST_CONC = test_concurrencia
TEST_SEG = test_segmentacion

# Directorios
BUILD_DIR = .build
SRC_DIR = src
TEST_DIR = tests

# todos los .c de la carpeta src
SRCS = $(wildcard $(SRC_DIR)/*.c)

# todo lo de src que no es main
CORE_SRCS = $(filter-out $(SRC_DIR)/main.c, $(SRCS))

# Transformar rutas de .c a .o en la carpeta .build
CORE_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CORE_SRCS))
MAIN_OBJ = $(BUILD_DIR)/main.o

.PHONY: all clean run run-seg test-pag test-conc test-seg test-all

all: $(TARGET)

# Compilación del ejecutable principal
$(TARGET): $(CORE_OBJS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# generica compilar archivos .c
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# tests
$(TEST_PAG): $(CORE_OBJS) $(TEST_DIR)/test_paginacion.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(TEST_CONC): $(CORE_OBJS) $(TEST_DIR)/test_concurrencia.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(TEST_SEG): $(CORE_OBJS) $(TEST_DIR)/test_segmentacion.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# limpieza
clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_PAG) $(TEST_CONC) $(TEST_SEG)

#ejecucion 
# make -> 		compila todo
# make run -> 	compila y ejecuta todo
run: all
	./$(TARGET) --mode tlb --threads 4 --ops-per-thread 100 --workload 8020 --stats

run-seg: all
	./$(TARGET) --mode seg --threads 2 --ops-per-thread 5000 --segments 4 --seg-limits 1024,2048,4096,8192 --stats

# tests
test-pag: $(TEST_PAG)
	./$(TEST_PAG)

test-conc: $(TEST_CONC)
	./$(TEST_CONC)

test-seg: $(TEST_SEG)
	./$(TEST_SEG)

test-all: test-pag test-seg test-conc