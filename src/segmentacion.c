#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "segmentacion.h"

/*
NOTA:
    Pese a que la paginacion es directa a la memoria principal con frame_allocator.c, el caso de la segmentación no se pide eso,
    el laboratorio solo se pide ver temas de limites y segmentation faults.
*/


/*
 * Función: initSegmentTable
 * Descripción: Inicializa la tabla de segmentos para un proceso/hilo. Reserva la memoria dinámica necesaria para la estructura principal y su arreglo interno de segmentos. 
 * Además, calcula una dirección base física simulada para cada segmento (comenzando en 0x1000) asegurando que no se superpongan, añadiendo un gap de seguridad de 256 bytes.
 * Entrada:
 * - numSegments (int): Cantidad total de segmentos que tendrá la tabla.
 * - limits (uint64_t*): Arreglo con los tamaños máximos (límites) de cada segmento.
 * Salida:
 * - struct segment_table*: Puntero a la nueva tabla de segmentos inicializada. 
 * (Nota: Si falla la asignación de memoria, el programa aborta con exit(EXIT_FAILURE)).
 */
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


/*
 * Función: translateAddress
 * Descripción: Traduce una dirección virtual (compuesta por un ID de segmento y un offset) a una dirección física simulada. Realiza dos validaciones de seguridad críticas: 
 * comprueba que el segmento exista en la tabla del hilo y verifica que el offset no exceda el límite de memoria asignado a ese segmento. Si es válido, calcula la dirección.
 * Entrada:
 * - table (struct segment_table*): Puntero a la tabla de segmentos del hilo actual.
 * - segId (uint64_t): Identificador del segmento al que se solicita acceso.
 * - offset (uint64_t): Desplazamiento en bytes dentro del segmento.
 * - physicalAddress (uint64_t*): Puntero donde se almacenará la dirección física calculada.
 * Salida:
 * - int: Retorna 1 si la traducción es exitosa. Retorna 0 si falla alguna validación de seguridad (simulando un Segmentation Fault).
 */int translateAddress(struct segment_table* table, uint64_t segId, uint64_t offset, uint64_t* physicalAddress) {

    // Validación de seguridad
    if (segId >= (uint64_t)table->num_segments) {
        return 0; // Segfault
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


/*
 * Función: destroySegmentTable
 * Descripción: Libera toda la memoria dinámica reservada para la tabla de segmentos. Primero verifica y libera el arreglo interno de descriptores de segmento, y finalmente libera la estructura principal de la tabla. 
 * Esto es crucial para prevenir fugas de memoria (memory leaks) al terminar el hilo.
 * Entrada:
 * - table (struct segment_table*): Puntero a la tabla de segmentos que se desea destruir.
 * Salida:
 * - void: No retorna ningún valor.
 */
void destroySegmentTable(struct segment_table* table) {
    if (table != NULL) {

        if (table->segments != NULL) {
            free(table->segments);
        }
        
        free(table);
    }
}







/*
 * Función: translateAddress
 * Descripción: Simula la MMU traduciendo una dirección lógica (virtual) a una dirección física. Verifica rigurosamente que el identificador de segmento exista en la tabla y que el 
 * desplazamiento (offset) solicitado no supere el límite permitido para dicho segmento. Si las validaciones pasan, calcula la dirección física sumando la base y el offset.
 * * Entrada:
 * - table (struct segment_table*): Puntero a la tabla de segmentos del hilo/proceso.
 * - segId (uint64_t): Identificador del segmento al que se intenta acceder.
 * - offset (uint64_t): Desplazamiento (en bytes) dentro del segmento.
 * - physicalAddress (uint64_t*): Puntero donde se guardará la dirección física calculada.
 * * Salida:
 * - int: Retorna 1 si la traducción fue exitosa (acceso válido). Retorna 0 si ocurre un fallo de segmentación simulado (Segmentation Fault) por segmento inexistente o límite excedido.
 */
