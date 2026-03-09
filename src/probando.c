#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#define NUM_ITERACIONES 1000000
pthread_mutex_t lock_global = PTHREAD_MUTEX_INITIALIZER;
bool safe = true;

// Variable global que compartirán ambos hilos
int contador_compartido = 0;

// Función que ejecutarán los hilos
void* incrementar(void* arg) {
    for (int i = 0; i < NUM_ITERACIONES; i++) {
        if (safe) {pthread_mutex_lock(&lock_global);}
        // SECCIÓN CRÍTICA: Aquí ocurre la condición de carrera
        contador_compartido++; 
        if (safe) {pthread_mutex_unlock(&lock_global);}
    }
    return NULL;
}

int main() {
    pthread_t hilo1, hilo2;
    // Creamos dos hilos que ejecutarán la misma función
    pthread_create(&hilo1, NULL, incrementar, NULL);
    pthread_create(&hilo2, NULL, incrementar, NULL);

    // Esperamos a que ambos hilos terminen
    pthread_join(hilo1, NULL);
    pthread_join(hilo2, NULL);

    // Lo esperado sería 2,000,000
    printf("Valor final del contador: %d\n", contador_compartido);

    return 0;
}