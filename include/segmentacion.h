// Header de segmentacion

#ifndef SEGMENTACION_H
#define SEGMENTACION_H

// Includes
#include <stdint.h>
#include <stdbool.h>



// -----------------------------------
//     ESTRUCTURAS DE SEGMENTACION
// -----------------------------------

struct segment_entry {
    uint64_t base; // direccion fisica base
    uint64_t limit; // Limite del segmento, Tamaño maximo del segmento en bytes
};

struct segment_table {
    struct segment_entry *segments;
    int num_segments;
};

// -----------------------------------
//     FIRMAS DE FUNCIONES
// -----------------------------------

struct segment_table* initSegmentTable(int numSegments, uint64_t* limits);
int translateAddress(struct segment_table* table, uint64_t segId, uint64_t offset, uint64_t* physicalAddress);
void destroySegmentTable(struct segment_table* table);

#endif