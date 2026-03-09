#ifndef PAGINACION_H
#define PAGINACION_H

#include <stdint.h>
#include <stdbool.h>

#define PAGE_SIZE 4096 // Tamaño de página en bytes
#define NUM_FRAMES 32  // Cantidad de marcos físicos en RAM
#define MAX_THREADS 10 // Número máximo de hilos soportados

// Entrada de la Tabla de Páginas
typedef struct {
    uint64_t frame_number;
    int valid;            // 1 si está en RAM, 0 si no
    int dirty;            // 1 si fue modificado (Bonus)
} PageTableEntry;

// Tabla de Páginas (Una por hilo)
typedef struct {
    PageTableEntry *entries;
    int num_pages;
} PageTable;

// Marco Físico en la RAM
typedef struct {
    int thread_id;        // Dueño actual del marco
    int page_index;       // Página virtual alojada
    int valid;            // 1 si está ocupado, 0 si está libre
} Frame;

typedef struct {
    int history[NUM_FRAMES];
    int head;
    int count;
} ColaFifo;

extern Frame RAM[NUM_FRAMES];
extern ColaFifo Global_Fifo;
extern PageTable* Thread_Tables[MAX_THREADS];

// -----------------------------------
//     FIRMAS DE FUNCIONES
// -----------------------------------

void inicializar_sistema();
int getFrameByPage(int page_index, PageTable* TP);
int traductVirtualDir(int page_index, int offset, PageTable* TP);
int requestReplaceFrame(int t_id, int page_index);

#endif 