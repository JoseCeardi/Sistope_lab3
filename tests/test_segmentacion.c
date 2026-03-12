#include "segmentacion.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    printf("=== PRUEBA DE SEGMENTACIÓN AISLADA ===\n");

    int ops =  10000;
    unsigned int seed = 100;
    uint64_t limits[] = {1024, 2048, 4096, 8192}; 
    int transOk = 0;
    int segFaults = 0;
    int errores_matematicos = 0;
    
    srand(seed); 

    struct segment_table* miTabla = initSegmentTable(4, limits);
    
    for (int i = 0; i < ops; i++) {
        uint64_t segIdAleatorio = rand() % 4; 
        uint64_t offsetAleatorio = rand() % 9000; 
        uint64_t miDireccionFisica = 0;

        int resultado = translateAddress(miTabla, segIdAleatorio, offsetAleatorio, &miDireccionFisica);

        if (resultado == 1) {
            transOk++;
            // Validar que la matemática (Base + Offset) se hizo bien internamente
            uint64_t base_esperada = miTabla->segments[segIdAleatorio].base;
            if (miDireccionFisica != (base_esperada + offsetAleatorio)) {
                errores_matematicos++;
            }
        } else {
            segFaults++;
        }
    }
    
    destroySegmentTable(miTabla);

    printf("\n--- RESULTADOS ---\n");
    printf("Operaciones totales : %d\n", ops);
    printf("Traducciones OK     : %d\n", transOk);
    printf("SegFaults Simulados : %d\n", segFaults);
    printf("Errores Matemáticos : %d\n", errores_matematicos);
    
    if ((transOk + segFaults) == ops && errores_matematicos == 0) {
        printf("\n>> TEST PASADO: Los resultados son consistentes y exactos.\n");
    } else {
        printf("\n>> TEST FALLIDO: Hay inconsistencias en la traducción.\n");
    }

    return 0;
}