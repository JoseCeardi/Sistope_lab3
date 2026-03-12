#include "simulator.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Config GlobConfig;
GlobalStats GlobStats = {0};

pthread_mutex_t metricsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t RamMutex = PTHREAD_MUTEX_INITIALIZER;

// cada proceso tiene su propia TP, TS y TLB
// es decir que Thread_Tables[1] = TP del hilo 1 (parte de 0)

PageTable* Thread_Tables[10]; 
tlb* Thread_TLBs[10];

void registrar_metrica(int* metrica) {
    if (!GlobConfig.unsafe) pthread_mutex_lock(&metricsMutex);
    (*metrica)++;
    if (!GlobConfig.unsafe) pthread_mutex_unlock(&metricsMutex);
}


/*
 ESTO SE RESUME A 3 PASOS:
 1° Elegir el modo  (A) paginacion  o  (B) segmentacion)

 2° Operaciones de traduccion se repiten ops_per_thread veces btw
    A) PAGINACION:    Buscar en la TLB -> si miss -> buscar en la TP -> si falla 5ms delay

    B) SEGMENTACION:  Revisa que el offset no supere el limite del segmento
                      si lo supera -> SEGFAULT -> error irreparable -> muere
                      -> actualizar TP y TLB según corresponda

 3° Actualizar las tablas de los hilos según corresponda

*/
void* execute_thread(void* arg){
    int thread_id = *(int*)arg;
    free(arg); 

    struct segment_table* STable = NULL;

    // Inicializamos punteros en el arreglo global para este hilo
    Thread_Tables[thread_id] = NULL;
    Thread_TLBs[thread_id] = NULL;

    // 1° Configuración según modo
    if (strcmp(GlobConfig.mode, "page") == 0 || strcmp(GlobConfig.mode, "tlb") == 0) {
        Thread_Tables[thread_id] = init_page_table(64);

        if (strcmp(GlobConfig.mode, "tlb") == 0) {
            Thread_TLBs[thread_id] = init_tlb(8, thread_id);
        }

    } else if (strcmp(GlobConfig.mode, "seg") == 0) {
        uint64_t limites[] = {1024, 2048, 1024, 512};
        STable = initSegmentTable(4, limites);
    }

    // 2° Bucle de ejecución de operaciones
    for (int i = 0; i < GlobConfig.ops_per_thread; i++) {

        // TODO: workloads.c para generar la direccion aleatoria
        uint64_t v_addr = rand() % 16384; 
        uint64_t p_addr = 0;

        //  MODO  (A) PAGINACIÓN /  (B) TLB
        if (Thread_Tables[thread_id] != NULL) {
            int vpn = v_addr / PAGE_SIZE;
            int offset = v_addr % PAGE_SIZE;
            int frame = -1;

            // (A) PAGINACION:
            //  Buscar en la TLB
            if (Thread_TLBs[thread_id] != NULL) {
                frame = search_tlb(Thread_TLBs[thread_id], vpn);
            }

            // hit o miss TLB?
            if (frame != -1) { 
                // HIT
                registrar_metrica(&GlobStats.total_tlb_hits);
            } else {
                // MISS
                if (Thread_TLBs[thread_id] != NULL) {
                    registrar_metrica(&GlobStats.total_tlb_misses);
                }

                frame = getFrameByPage(vpn, Thread_Tables[thread_id]);

                if (frame == -1) {
                    // PAGE FAULT: No está en RAM -> reemplazo
                    frame = requestReplaceFrame(thread_id, vpn);
                    Thread_Tables[thread_id]->entries[vpn].frame_number = frame;
                    Thread_Tables[thread_id]->entries[vpn].valid = 1;
                }

                // Actualizar TLB
                if (Thread_TLBs[thread_id] != NULL) {
                    update_tlb(Thread_TLBs[thread_id], vpn, frame);
                }
            }

            // Traduccion exitosa 
            p_addr = (uint64_t)frame * PAGE_SIZE + offset;
            registrar_metrica(&GlobStats.total_translation_ok);
            
        } 
        // (B) SEGMENTACION
        else if (STable != NULL) { 
            // TODO: agregar aca lo de segmentacion
        }
    }

    // Limpieza de memoria del hilo
    if (Thread_Tables[thread_id]) destroy_page_table(Thread_Tables[thread_id]);
    if (Thread_TLBs[thread_id]) destroy_tlb(Thread_TLBs[thread_id]);
    if (STable) destroySegmentTable(STable);

    return NULL;
}



// Inicializar y ejecutar todo, threads, tablas, ram, etc.
void start_simulation(Config* config) {
    for (int i = 0; i < 10; i++) {
        Thread_Tables[i] = NULL;
        Thread_TLBs[i] = NULL;
    }

    // Inicializar RAM y la cola FIFO
    inicializar_ram();

    // Creamos el arreglo de IDs de hilos 
    pthread_t hilos[config->num_threads];

    // Lanzamos los hilos concurrentemente
    for (int i = 0; i < config->num_threads; i++) {
        int* id = malloc(sizeof(int));
        if (id == NULL) {
            perror("Error al asignar memoria para thread_id");
            exit(EXIT_FAILURE);
        }
        *id = i;

        if (pthread_create(&hilos[i], NULL, execute_thread, id) != 0) {
            perror("Error al crear el hilo");
            exit(EXIT_FAILURE);
        }
    }

    // Esperar q todos terminen (es como en barreras)
    for (int i = 0; i < config->num_threads; i++) {
        pthread_join(hilos[i], NULL);
    }

    printf("Simulación finalizada.\n");
}