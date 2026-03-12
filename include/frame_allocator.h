#ifndef FRAME_ALLOCATOR_H
#define FRAME_ALLOCATOR_H

#include <stdint.h>
#include <stdbool.h>
#include "paginacion.h"
#include "tlb.h" 
#include "simulator.h"
#include <pthread.h>

#define NUM_FRAMES 32

typedef struct {
    int thread_id;        
    int page_index;       
    int valid;            
} Frame;

typedef struct {
    int history[NUM_FRAMES]; 
    int head;
    int count;
} ColaFifo;

// RAM y FIFO publicos y globales
extern Frame RAM[NUM_FRAMES];
extern ColaFifo Global_Fifo;

// arreglos globales para poder modificarlos desde distintas clases
// esto para que se invaliden todas las tlb a la vez si ocurre un eviction
extern PageTable* Thread_Tables[10]; 
extern tlb* Thread_TLBs[10];
extern pthread_mutex_t RamMutex;

void inicializar_ram();
void simular_carga_disco();
int requestReplaceFrame(int t_id, int page_index);

#endif