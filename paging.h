#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stdbool.h>

// -----------------------------------
//     CONSTANTES DEL SISTEMA
// -----------------------------------
#define PAGE_SIZE 4096 // Tamaño de página en bytes
#define NUM_FRAMES 32  // Cantidad de marcos físicos en RAM
#define MAX_THREADS 10 // Número máximo de hilos soportados
#define TLB_SIZE 16    // Entradas máximas en la TLB por hilo

// -----------------------------------
//     ESTRUCTURAS DE PAGINACIÓN
// -----------------------------------

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

// Historial para el algoritmo FIFO (Global)
typedef struct {
    int history[NUM_FRAMES];
    int head;
    int count;
} ColaFifo;

// -----------------------------------
//     ESTRUCTURAS DE LA TLB
// -----------------------------------

// Entrada de la TLB
typedef struct {
    int vpn;              // Virtual Page Number
    int frame_number;     // Physical Frame Number
    int valid;            // 1 si la entrada es válida
} TLBEntry;

// TLB (Una por hilo)
typedef struct {
    TLBEntry entries[TLB_SIZE];
    int head;             // Puntero para el reemplazo FIFO interno de la TLB
} TLB;

// -----------------------------------
//     VARIABLES GLOBALES (Extern)
// -----------------------------------
// Usamos 'extern' para que el compilador sepa que estas variables 
// existen y se inicializan en paging.c, pero se pueden ver desde main.c

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

// Futuras funciones de la TLB (opcionales por ahora, pero recomendadas)
// int searchTLB(int page_index, TLB* tlb);
// void updateTLB(int vpn, int frame, TLB* tlb);

#endif // PAGING_H