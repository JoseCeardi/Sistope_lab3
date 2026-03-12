#include <stdio.h>
#include <stdlib.h>
#include "frame_allocator.h"
#include <time.h>

// Definición real de los datos compartidos
Frame RAM[NUM_FRAMES];
ColaFifo Global_Fifo;

void inicializar_ram() {
    for (int i = 0; i < NUM_FRAMES; i++) {
        RAM[i].valid = 0;
        RAM[i].thread_id = -1;
        RAM[i].page_index = -1;
    }
    Global_Fifo.head = 0;
    Global_Fifo.count = 0;
    
    // Limpiamos los punteros de los hilos 
    for (int i = 0; i < 10; i++) {
        Thread_Tables[i] = NULL;
        Thread_TLBs[i] = NULL;
    }
}


// dormir 5ms
// fuente: https://www.geeksforgeeks.org/c/c-nanosleep-function/
void simular_carga_disco() {
    int margen = (rand() % 5 + 1) * 1000000; 
    struct timespec remaining, request = { 0, margen };
    nanosleep(&request, &remaining);
}


/*
Si hay marco disponible: reemplaza directo en la RAM

Si no hay marco disponible:
    - Tenemos que cambiar el marco a lo fifo
    - Tambien quitarlo de la TP del thread que lo ocupaba
    - Actualizar historial FIFO

nota:
    Como esto accede a la RAM (compartida por todos los precesos), necesitamos controlar la variable SAFE
    Pero como las hebras al iniciarse usan (void*) no podría pasarlo como parámetro
    por eso lo expresamos como extern.
        - Tanto GlobConfig como metricsMutex son extern, asi puedo preguntar
          GlobConfig.unsafe y saber el modo.
    */
int requestReplaceFrame(int t_id, int page_index) {
    int frame = -1;

    // SAFE
    if (!GlobConfig.unsafe) {
        pthread_mutex_lock(&RamMutex); 
    }
    GlobStats.total_page_faults++;

     // buscar marco libre --> SC accedemos a la RAM
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (RAM[i].valid == 0) {
            frame = i;
            int pos = (Global_Fifo.head + Global_Fifo.count) % NUM_FRAMES;
            Global_Fifo.history[pos] = i;
            Global_Fifo.count++;
            break;
        }
    }

    // Si no hay libres -> Reemplazo FIFO
    if (frame == -1) {

        // FAULT
        GlobStats.total_evictions++;
        frame = Global_Fifo.history[Global_Fifo.head];

        int victim_thread = RAM[frame].thread_id; //quienes se van
        int victim_page = RAM[frame].page_index;

        // Invalidar en la Tabla de Páginas del hilo víctima
        if (Thread_Tables[victim_thread] != NULL) {
            Thread_Tables[victim_thread]->entries[victim_page].valid = 0;
        }

        // Invalidar en la TLB del hilo víctima
        if (Thread_TLBs[victim_thread] != NULL) {
            invalidate_tlb_entry(Thread_TLBs[victim_thread], (uint64_t)victim_page);
        }

        // Actualizar cola FIFO
        Global_Fifo.history[Global_Fifo.head] = frame;
        Global_Fifo.head = (Global_Fifo.head + 1) % NUM_FRAMES;

    }

    // Ocupar el frame
    RAM[frame].thread_id = t_id;
    RAM[frame].page_index = page_index;
    RAM[frame].valid = 1;

    if (!GlobConfig.unsafe) {
        pthread_mutex_unlock(&RamMutex);
    }

    // dormir 5ms
    simular_carga_disco();

    return frame; 
}