#ifndef FRAME_ALLOCATOR_H
#define FRAME_ALLOCATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct {
    int thread_id;        
    int page_index;       
    int valid;            
} Frame;

typedef struct {
    int* history; 
    int head;
    int count;
} ColaFifo;

extern Frame* RAM;
extern ColaFifo Global_Fifo;

// --- ELIMINAMOS LAS DECLARACIONES DE Thread_Tables Y Thread_TLBs DE ACÁ ---
// Las dejaremos solo en simulator.h para evitar el conflicto de tipos.

void inicializar_ram();
void simular_carga_disco();
int requestReplaceFrame(int t_id, int page_index);

#endif