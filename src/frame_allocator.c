#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "paginacion.h"
#include "tlb.h"           
#include "frame_allocator.h"
#include "simulator.h"

// Definición datos compartidos dinámicos
Frame* RAM = NULL;
ColaFifo Global_Fifo;

void inicializar_ram() {
    //  Asignar memoria dinámica según config
    RAM = (Frame*)malloc(GlobConfig.frames * sizeof(Frame));
    Global_Fifo.history = (int*)malloc(GlobConfig.frames * sizeof(int));

    if (RAM == NULL || Global_Fifo.history == NULL) {
        perror("Error asignando memoria para la RAM o el FIFO");
        exit(EXIT_FAILURE);
    }

    // Inicializar los marcos según el número solicitado
    for (int i = 0; i < GlobConfig.frames; i++) {
        RAM[i].valid = 0;
        RAM[i].thread_id = -1;
        RAM[i].page_index = -1;
        Global_Fifo.history[i] = -1;
    }

    Global_Fifo.head = 0;
    Global_Fifo.count = 0;

    // Limpiar punteros hilos
    for (int i = 0; i < 10; i++) {
        Thread_Tables[i] = NULL;
        Thread_TLBs[i] = NULL;
    }
}

// Dormir 1-5ms para simular acceso a MS
void simular_carga_disco() {
    int margen = (rand() % 5 + 1) * 1000000;
    struct timespec remaining, request = { 0, margen };
    nanosleep(&request, &remaining);
}

// Manejador de Page Faults y Reemplazo FIFO Global
int requestReplaceFrame(int t_id, int page_index) {
    int frame = -1;

    // Registrar Page Fault en las métricas globales
    GlobStats.total_page_faults++;

    //  Buscar si existe algún marco libre físicamente
    for (int i = 0; i < GlobConfig.frames; i++) {
        if (RAM[i].valid == 0) {
            frame = i;
            int pos = (Global_Fifo.head + Global_Fifo.count) % GlobConfig.frames;
            Global_Fifo.history[pos] = i;
            Global_Fifo.count++;
            break;
        }
    }

    // Si no hay marcos libres -> Ejecuta FIFO
    if (frame == -1) {
        GlobStats.total_evictions++; // Registrar eviction
        frame = Global_Fifo.history[Global_Fifo.head];

        int victim_thread = RAM[frame].thread_id;
        int victim_page = RAM[frame].page_index;

        // EVICTION

        /*
        NOTA: Como hay una dependencia circular entre simulator.h, frame_allocator.h y paginacion.h
              usamos casteo para acceder a los threads, esto porque el void* oculta la estructura original al header,
              permitiendo que se salte los includes y por ende la dependencia circular
        */

        // Invalidar en la Tabla de Páginas del hilo víctima
        if (Thread_Tables[victim_thread] != NULL) {
            PageTable* pt_victima = (PageTable*)Thread_Tables[victim_thread];
            pt_victima->entries[victim_page].valid = 0;
        }

        // Invalidar en la TLB del hilo víctima
        if (Thread_TLBs[victim_thread] != NULL) {
            invalidate_tlb_entry((tlb*)Thread_TLBs[victim_thread], (uint64_t)victim_page);
        }

        // Actualizar el puntero de la cola circular FIFO
        Global_Fifo.history[Global_Fifo.head] = frame;
        Global_Fifo.head = (Global_Fifo.head + 1) % GlobConfig.frames;
    }

    // Ocupar el frame con la nueva página cargada
    RAM[frame].thread_id = t_id;
    RAM[frame].page_index = page_index;
    RAM[frame].valid = 1;

    // Simular latencia de disco
    simular_carga_disco(); 

    return frame; 
}