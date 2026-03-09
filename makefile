# Variables del compilador
CC = gcc
# Añadimos -Iinclude para que el compilador sepa dónde buscar los archivos .h
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=199309L -Iinclude
LDFLAGS = -pthread

# Nombres y carpetas
TARGET = simulator
BUILD_DIR = .build
SRC_DIR = src
INC_DIR = include

SRCS = $(wildcard $(SRC_DIR)/*.c)

# Reemplazar 'src/archivo.c' por '.build/archivo.o'
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Regla por defecto
all: $(TARGET)

# Linker
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Regla para compilar los .c en .o dentro de la carpeta .build
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar todo lo autogenerado
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Compilar y ejecutar de una vez
run: all
	./$(TARGET)