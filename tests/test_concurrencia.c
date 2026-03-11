#include "segmentacion.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
// gcc -Iinclude -Wall -Wextra -pthread src/segmentacion.c tests/test_concurrencia.c -o test_concurrencia
//cambiar dependiendo de n de thr y ops
#define NUM_THREADS 3
#define OPS_PER_THREAD 1000000

//variables globales
int global_translations_ok = 0;
int global_segfaults = 0;
pthread_mutex_t metrics_mutex; //mutex
pthread_mutex_t rand_mutex; //mutex para rand_R
int is_safe_mode = 0; // interruptor de modo seguro, 1 safe, 0 un-safe


// funcion de cada thread
void* threadRoutine(void* arg) {
    (void)arg; //para evitar la warning
    //int thread_id = *(int*)arg;
    uint64_t limits[] = {1024, 2048, 4096, 8192};
    
    // cada hilo crea su propia tabla privada
    struct segment_table* miTabla = initSegmentTable(4, limits);
    
    // derivamos una semilla única por hilo 
    //unsigned int seed = 100 + thread_id; 

    for (int i = 0; i < OPS_PER_THREAD; i++) {
        uint64_t segIdAleatorio; 
        uint64_t offsetAleatorio; 


        if (is_safe_mode) { //setteo de rand
            pthread_mutex_lock(&rand_mutex);
            segIdAleatorio = rand() % 4; 
            offsetAleatorio = rand() % 9000; 
            pthread_mutex_unlock(&rand_mutex);
        } else {
            
            segIdAleatorio = rand() % 4; 
            offsetAleatorio = rand() % 9000; 
        }


        uint64_t pa;

        int res = translateAddress(miTabla, segIdAleatorio, offsetAleatorio, &pa);

        // --- SECCIÓN CRÍTICA ---
        if (is_safe_mode) {
            pthread_mutex_lock(&metrics_mutex); // enter sc
            if (res == 1) global_translations_ok++;
            else global_segfaults++;
            pthread_mutex_unlock(&metrics_mutex); // exit sc
        } else { //sin SC
            if (res == 1) global_translations_ok++;
            else global_segfaults++;
        }
    }
    
    destroySegmentTable(miTabla);
    return NULL;
}


void testConcurrencia1(){
    printf("Iniciando test de concurrencia con %d hilos...\n", NUM_THREADS);
    printf("Modo: %s\n", is_safe_mode ? "SAFE (Con Mutex)" : "UNSAFE (Sin Mutex)");

    // Inicializamos el mutex
    pthread_mutex_init(&metrics_mutex, NULL);
    pthread_mutex_init(&rand_mutex, NULL);

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    // crear threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, threadRoutine, &thread_ids[i]);
    }

    // esperar que terminen
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // destruimos el mutex
    pthread_mutex_destroy(&metrics_mutex);
    pthread_mutex_destroy(&rand_mutex);


    int total_esperado = NUM_THREADS * OPS_PER_THREAD;
    int total_real = global_translations_ok + global_segfaults;

    printf("\n--- RESULTADOS GLOBALES ---\n");
    printf("Total operaciones esperadas: %d\n", total_esperado);
    printf("Total operaciones contadas:  %d\n", total_real);
    
    if (total_real == total_esperado) {
        printf("Salio bien. No se perdieron datos.\n");
    } else {
        printf("Se perdieron %d operaciones.\n", total_esperado - total_real);
    }


}

int main() {
    srand(100);
    testConcurrencia1();
    

    return 0;
}

