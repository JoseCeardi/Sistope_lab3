#include "segmentacion.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// gcc -Iinclude -Wall -Wextra src/segmentacion.c tests/test_segmentacion.c -o test_seg

 int main(){
    printf("prueba segmentacion, 4 limits 1 hebra\n");


    int ops =  10000;
    unsigned int seed = 100;
    uint64_t limits[] = {1024, 2048, 4096, 8192}; // Límites para cada segmento
    int transOk = 0;
    int segFaults = 0;
    srand(seed); // Semilla para reproducibilidad


    struct segment_table* miTabla = initSegmentTable(4, limits);
    
    for (int i = 0; i < ops; i++){
        uint64_t segIdAleatorio = rand() % 4; // Generar un ID de segmento aleatorio entre 0 y 3
        uint64_t offsetAleatorio = rand() % 9000; // Generar un offset aleatorio (puede ser mayor que el límite para probar el segfault)
        uint64_t miDireccionFisica;

        int resultado = translateAddress(miTabla, segIdAleatorio, offsetAleatorio, &miDireccionFisica);

        if (resultado == 1) {
            //printf("Acceso válido: Segmento %lu, Offset %lu -> Dirección Física: %lu\n", segIdAleatorio, offsetAleatorio, miDireccionFisica);
            transOk++;
        } else {
            //printf("Segmentation Fault simulado: Segmento %lu, Offset %lu\n", segIdAleatorio, offsetAleatorio);
            segFaults++;

        }
    }
    destroySegmentTable(miTabla);
    printf("Total de Segmentation Faults simulados: %d\n", segFaults);

    printf("\n--- RESULTADOS ---\n");
    printf("Operaciones totales: %d\n", ops);
    printf("Traducciones exitosas (translations_ok): %d\n", transOk);
    printf("Segmentation Faults simulados: %d\n", segFaults);
    
    if ((transOk + segFaults) == ops) {
        printf("La suma de resultados coincide con las operaciones totales.\n");
    } else {
        printf("Error: La suma de resultados no coincide con las operaciones totales.\n");
    }

}