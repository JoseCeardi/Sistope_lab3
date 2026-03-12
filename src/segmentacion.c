#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "segmentacion.h"

//apuntes, brorrar despues
// --seg-limits dicta la "distancia" o el desplazamiento que habrá entre las direcciones base de cada segmento -> limits var

//
struct segment_table* initSegmentTable(int numSegments, uint64_t* limits) {
    // Reservar memoria para la estructura de la tabla
    struct segment_table* table = (struct segment_table*)malloc(sizeof(struct segment_table));
    if (table == NULL) {
        perror("Error: No se pudo asignar memoria para la tabla de segmentos");
        exit(EXIT_FAILURE);
    }

    table->num_segments = numSegments;

    // Reservar memoria para el arreglo dinámico de segmentos
    table->segments = (struct segment_entry*)malloc(numSegments * sizeof(struct segment_entry));
    if (table->segments == NULL) {
        perror("Error: No se pudo asignar memoria para el arreglo de segmentos");
        free(table);
        exit(EXIT_FAILURE);
    }

    // Inicializar cada segmento con su base y límite
    uint64_t currentBase = 0x1000; // Dirección física inicial ficticia 

    for (int i = 0; i < numSegments; i++) {
        table->segments[i].limit = limits[i]; 
        table->segments[i].base = currentBase;

        // Para el siguiente segmento, movemos la base física, sumándole el límite del segmento actual para que no se superpongan
        currentBase += limits[i] + 256; 
    }

    return table;
}



//usar asi: int resultado = translateAddress(miTabla, segIdAleatorio, offsetAleatorio, &miDireccionFisica);
//despues haces un if para los seg fault, 1 esta ok, 0 hay seg fault
int translateAddress(struct segment_table* table, uint64_t segId, uint64_t offset, uint64_t* physicalAddress) {
    
    // Validación de seguridad básica (evitar que el programa se caiga de verdad)
    if (segId >= (uint64_t)table->num_segments) {
        return 0; // Segfault: El proceso intentó acceder a un segmento que no existe
    }

    // Extraer el segmento que el hilo está pidiendo
    struct segment_entry segment = table->segments[segId];

    //  Validación, offset < limit
    if (offset >= segment.limit) {
        return 0; // Segfault simulado: se pasó del límite permitido 
    }

    // Calcular la dirección física: PA = base + offset
    *physicalAddress = segment.base + offset;

    return 1; 
}



void destroySegmentTable(struct segment_table* table) {
    if (table != NULL) {

        if (table->segments != NULL) {
            free(table->segments); //liberamos el arreglo de segmentos
        }
        
        free(table);
    }
}