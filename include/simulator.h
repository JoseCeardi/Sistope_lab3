#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

/* Forward declarations */
struct PageTable;
typedef struct PageTable PageTable;
struct tlb;
typedef struct tlb tlb;

extern PageTable* Thread_Tables[10]; 
extern tlb* Thread_TLBs[10];
extern pthread_mutex_t RamMutex;

// Configuración capturada desde la terminal
typedef struct {
    char* mode;
    int num_threads;
    int ops_per_thread;
    char* workload;
    int seed;
    bool unsafe;
    bool stats_report;
    int num_segments;
    uint64_t* seg_limits;
    int pages;
    int frames;
    int page_size;
    int tlb_size;
    const char* tlb_policy;
    const char* evict_policy;
} Config;

// Estructura de métricas globales
typedef struct { 
    int total_translation_ok;
    int total_segfaults;
    int total_tlb_hits;
    int total_tlb_misses;
    int total_page_faults;
    int total_evictions;
    double start_time;
    double runtime_sec; 
    long avg_translation_time_ns;
} GlobalStats;

/* Variables globales */
extern Config GlobConfig;
extern GlobalStats GlobStats;
extern pthread_mutex_t metricsMutex;
extern pthread_mutex_t RamMutex;



/* Tablas globales */
extern PageTable* Thread_Tables[10]; 
extern tlb* Thread_TLBs[10];

// Funciones principales
void start_simulation(Config* config);
void* execute_thread(void* arg);

#endif