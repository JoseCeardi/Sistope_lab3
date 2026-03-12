#include "simulator.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
Esta clase es la fundamental, maneja toda la concurrencia y en general es el cerebro de todo
Como esta centralizado con un unico semaforo tanto las metricas como la ram son protegidas a la vez
por el mismo lock, eso no es ideal pero funciona bien, y la otra opcion eran varios semaforos, pero causa
demasiados problemas, asi que se descarta por temas de tiempo.

En resumen el mismo semaforo para metricas y RAM, pero el de metricas es un metodo especifico que solo cierra, cambia la metrica y abre
*/

Config GlobConfig;
GlobalStats GlobStats = {0};

pthread_mutex_t metricsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t RamMutex = PTHREAD_MUTEX_INITIALIZER;

PageTable* Thread_Tables[10]; 
tlb* Thread_TLBs[10];


/*
Simplemente aumenta las metricas haciendo lock para evitar temas de CC
*/
void registrar_metrica(int* metrica) {
    if (!GlobConfig.unsafe) pthread_mutex_lock(&metricsMutex);
    (*metrica)++;
    if (!GlobConfig.unsafe) pthread_mutex_unlock(&metricsMutex);
}


/*
(esto se ejecuta con la RAM y los hilos ya inicializados)

La ejecución completa de cada hilo:

1° INICIALIZACION
    - genera seed del hilo
    - genera las tablas de segmentos, paginas y tlb en NULL;

2° ELEGIR MODO Y CONFIGURACIONES INICIALES
    - (A) paginacion -> hay tlb?
    - (B) Segmentacion

3° BUCLE PRINCIPAL
    - Generar direccion con el workload -> puede ser 8020 (aprovecha localidad) o aleatorio

    (A) PAGINACION
    - Buscar en la TLB
    - Si no esta en la TLB buscar en la TP
    - Si no esta en la TP PAGE_FAULT

    (B) SEGMENTACION
    - Buscar en la TS
    - Si se pasa del limite -> SEG_FAULT

*/
void* execute_thread(void* arg){

    // 1° INICIALIZACION
    int thread_id = *(int*)arg;
    free(arg); 

    unsigned int local_seed = GlobConfig.seed + thread_id;

    struct segment_table* STable = NULL;

    Thread_Tables[thread_id] = NULL;
    Thread_TLBs[thread_id] = NULL;


    // 2° ELEGIR MODO Y CONFIGURACIONES INICIALES (nota las GlobalConfigs se conocen de antes de llamar a esta funcion)

    // hay paginacion?
    if (strcmp(GlobConfig.mode, "page") == 0 || strcmp(GlobConfig.mode, "tlb") == 0) {
        Thread_Tables[thread_id] = init_page_table(64);

        // hay tlb?
        if (strcmp(GlobConfig.mode, "tlb") == 0) {
            Thread_TLBs[thread_id] = init_tlb(8, thread_id);
        }

        // - no hay tlb ni paginacion
        // hay segmentacion? 
    } else if (strcmp(GlobConfig.mode, "seg") == 0) {
        STable = initSegmentTable(GlobConfig.num_segments, GlobConfig.seg_limits);
    }


    // 3° BUCLE PRINCIPAL
    for (int i = 0; i < GlobConfig.ops_per_thread; i++) {
        uint64_t v_addr = generate_v_addr(GlobConfig.workload, &local_seed);  // generar direccion con el workload
        uint64_t p_addr = 0;

        if (Thread_Tables[thread_id] != NULL) {
            // (A) PAGINACION

            int vpn = v_addr / PAGE_SIZE;           // nro pagina
            int offset = v_addr % PAGE_SIZE;        // offset
            int frame = -1; // parte vacia la ram

            if (Thread_TLBs[thread_id] != NULL) {
                frame = search_tlb(Thread_TLBs[thread_id], vpn); // si hay tlb busca en la tlb primero
            }

            // HIT TLB
            if (frame != -1) { 
                registrar_metrica(&GlobStats.total_tlb_hits);
            // MISS TLB
            } else {
                if (Thread_TLBs[thread_id] != NULL) {
                    registrar_metrica(&GlobStats.total_tlb_misses);
                }

                // no esta en la TLB -> toca buscar en la RAM (TP)
                frame = getFrameByPage(vpn, Thread_Tables[thread_id]);

                if (frame == -1) { // NO ESTA EN LA RAM
                    
                    // Tenemos que acceder a la RAM para cargar la pagina, ya que hubo miss de TP
                    // como modificaremos datos puede haber CC, asi que protegemos antes de entrar a la RAM
                    if (!GlobConfig.unsafe) pthread_mutex_lock(&RamMutex);
                    
                    // en esencia esto entra a la RAM, y carga la pagina en memoria principal, (dependiendo puede haber EVICTION ACA)
                    frame = requestReplaceFrame(thread_id, vpn);
                    
                    // actualizamos las tablas del hilo en cuestion
                    Thread_Tables[thread_id]->entries[vpn].frame_number = frame;
                    Thread_Tables[thread_id]->entries[vpn].valid = 1;

                    // lo mismo con la tlb si hay
                    if (Thread_TLBs[thread_id] != NULL) {
                        update_tlb(Thread_TLBs[thread_id], vpn, frame);
                    }
                    
                    // ya salimos de la RAM
                    if (!GlobConfig.unsafe) pthread_mutex_unlock(&RamMutex);
                }
            }

            // Calculamos la direccion de la pagina
            p_addr = (uint64_t)frame * PAGE_SIZE + offset;
            (void)p_addr; 
            registrar_metrica(&GlobStats.total_translation_ok);
            
           

        } 

        /* (B) MODO SEGMENTACIÓN: 
             * Traduce direcciones mediante Base + Límite.
             * - Descompone v_addr en ID de segmento y Offset
             * - translateAddress valida que el Offset no supere el Límite del segmento.
             * - Registra éxito (Traducción OK) o error (Segmentation Fault) en las métricas.
             */
        else if (STable != NULL) { 
            uint64_t segIdAleatorio = (v_addr >> 12) % STable->num_segments;  // offset de 12 bits
            uint64_t offsetAleatorio = v_addr % 9000;  //Nota: Se usa % 9000 para forzar offsets fuera de límite y así testear SegFaults.
            uint64_t pa = 0;

            int res = translateAddress(STable, segIdAleatorio, offsetAleatorio, &pa);
            (void)pa; 

            if (res == 1) {
                registrar_metrica(&GlobStats.total_translation_ok);
            } else {
                registrar_metrica(&GlobStats.total_segfaults);
            }
        }
    }

    // limpiar
    if (STable) destroySegmentTable(STable);

    return NULL;
}




/*
INICIO DE LA APLICACION

    - inicializa todo: threads, TLBs, Ram, etc
    - llama a la ejecución completa de la simulación a cada thread
    - termina la simulación y espera a que todos los hilos terminen
    - limpia la memoria
*/
void start_simulation(Config* config) {
    for (int i = 0; i < 10; i++) {
        Thread_Tables[i] = NULL;
        Thread_TLBs[i] = NULL;
    }

    inicializar_ram();

    pthread_t hilos[config->num_threads];

    for (int i = 0; i < config->num_threads; i++) {
        int* id = malloc(sizeof(int));
        if (id == NULL) {
            perror("Error al asignar memoria");
            exit(EXIT_FAILURE);
        }
        *id = i;

        if (pthread_create(&hilos[i], NULL, execute_thread, id) != 0) { //llamar a ejecutar a todos los hilos
            perror("Error al crear el hilo");
            exit(EXIT_FAILURE);
        }
    }

    // Esperar a que terminen los hilos
    for (int i = 0; i < config->num_threads; i++) {
        pthread_join(hilos[i], NULL);
    }

    // Limpiar la memoria
    for (int i = 0; i < config->num_threads; i++) {
        if (Thread_Tables[i]) destroy_page_table(Thread_Tables[i]);
        if (Thread_TLBs[i]) destroy_tlb(Thread_TLBs[i]);
    }

    printf("\n>> Simulación finalizada.\n");
}