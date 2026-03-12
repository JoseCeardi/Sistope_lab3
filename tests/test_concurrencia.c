#include "segmentacion.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 

#define NUM_THREADS 4
#define OPS_PER_THREAD 1000000

// Variables globales para el test
int g_ok = 0;
int g_fail = 0;
pthread_mutex_t m_metrics = PTHREAD_MUTEX_INITIALIZER;
int is_safe = 1;

void* threadRoutine(void* arg) {
    int id = *(int*)arg;
    uint64_t limits[] = {1024, 2048, 4096, 8192};
    struct segment_table* miTabla = initSegmentTable(4, limits);
    
    // Semilla privada por hilo para evitar usar un mutex en el rand()
    unsigned int seed = 42 + id; 

    for (int i = 0; i < OPS_PER_THREAD; i++) {
        uint64_t seg = rand_r(&seed) % 4; 
        uint64_t offset = rand_r(&seed) % 9000; 
        uint64_t pa;

        int res = translateAddress(miTabla, seg, offset, &pa);

        if (is_safe) {
            pthread_mutex_lock(&m_metrics);
            if (res == 1) g_ok++; else g_fail++;
            pthread_mutex_unlock(&m_metrics);
        } else {
            // Sin protección: aquí ocurre la condición de carrera
            if (res == 1) g_ok++; else g_fail++;
        }
    }
    
    destroySegmentTable(miTabla);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    printf("Iniciando test: %d hilos, %d ops/hilo (%s)\n", 
            NUM_THREADS, OPS_PER_THREAD, is_safe ? "SAFE" : "UNSAFE");

    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, threadRoutine, &ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    int esperado = NUM_THREADS * OPS_PER_THREAD;
    int total = g_ok + g_fail;

    printf("\n--- RESULTADOS ---\n");
    printf("Esperado: %d\n", esperado);
    printf("Contado:  %d\n", total);
    printf("Diferencia: %d\n", esperado - total);

    if (total == esperado) printf(">> TEST PASADO\n");
    else printf(">> TEST FALLIDO (Condicion de carrera identificada)\n");

    return 0;
}