# Variables del compilador
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=199309L
LDFLAGS = -pthread

# Nombres y carpetas
TARGET = simulator
BUILD_DIR = .build

# Archivos fuente
SRCS = main.c paging.c
OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))

# Regla por defecto
all: $(TARGET)

# Unir (linkear) los archivos .o para crear el ejecutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Regla para compilar los .c en .o dentro de la carpeta .build
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar todo lo autogenerado
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Compilar y ejecutar de una vez
run: all
	./$(TARGET)