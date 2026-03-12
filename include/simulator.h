
#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "frame_allocator.h"
#include "paginacion.h"
#include "segmentacion.h"
#include "tlb.h"


// Capturar argumentos argv
typedef struct {
    char* mode;
    int num_threads;
    int ops_per_thread;
    char* workload;
    int seed;
    bool unsafe;
    bool stats_report;
} Config;

// estadisticas
typedef struct { 
    int total_translation_ok;
    int total_segfaults;
    int total_tlb_hits;
    int total_tlb_misses;
    int total_page_faults;
    int total_evictions;
    double start_time;
} GlobalStats;

void simular_carga_disco();

// usamos extern porque al crear los hilos no podríamos pasarlo como argumento
extern Config GlobConfig;
extern GlobalStats GlobStats;
extern pthread_mutex_t metricsMutex;

void iniciar_simulacion(Config* config);
void* ejecutar_hilo(void* arg);



#endif 
