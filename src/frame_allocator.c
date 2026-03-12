#include <stdio.h>
#include <stdlib.h>
#include "frame_allocator.h"
#include <time.h>


/*
NOTA:  BORRAR LUEGO
Quité acá el manejo de threads, eso quedará solo en simulator
porque si ponia el lock acá, el tema es que tanto al ver si esta en ram
como el sleep cuando no está causaban problemas de concurrencia porque
tambien se tienen que proteger las estadisticas, y la cosa es que las estadisticas
requieren ser protegidas aparte, pero es un cacho tener 2 semaforos para proteger tanto
las metricas como la ram, ya que seria tener semarofors en frame_allocator.c y en simulator.c
eso causa demasiados problemas, asi que lo dejé asi.

el diseño actual toda la concurrencia se maneja en el simulator.c, asi que se hace lock antes de entrar
a esta clase, de manera que tanto la proteccion de metricas como la ram estan manejadas con el mismo semaforo
no da tiempo a corregirlo y no creo que sea tan malo, los resultados me dan con este diseño.
*/

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
*/
int requestReplaceFrame(int t_id, int page_index) {
    int frame = -1;

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

    // dormir 5ms
    simular_carga_disco();

    return frame; 
}