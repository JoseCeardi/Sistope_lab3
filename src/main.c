#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "frame_allocator.h"
#include "paginacion.h"
#include "tlb.h"

#define NUM_PAGINAS 64
#define ACCESOS_TOTALES 1000

// Función auxiliar para simular un intento de lectura a la memoria
void acceder_memoria(int id, PageTable* tp, tlb* mi_tlb, int* hits, int* misses, int* faults) {
    int vpn_deseado = rand() % NUM_PAGINAS;
    int frame_obtenido = search_tlb(mi_tlb, vpn_deseado);

    if (frame_obtenido != -1) {
        // TLB Hit
        (*hits)++;
    } else {
        // TLB Miss
        (*misses)++;

        if (tp->entries[vpn_deseado].valid == 1) {
            // Page Table Hit
            frame_obtenido = tp->entries[vpn_deseado].frame_number;
        } else {
            // Page Fault
            (*faults)++;
            frame_obtenido = requestReplaceFrame(id, vpn_deseado);

            // Actualizamos la tabla de páginas
            tp->entries[vpn_deseado].frame_number = frame_obtenido;
            tp->entries[vpn_deseado].valid = 1;
        }

        // Actualizamos la TLB
        update_tlb(mi_tlb, vpn_deseado, frame_obtenido);
    }
}

int main() {
    srand(time(NULL));

    printf("=== PRUEBA SECUENCIAL DE MEMORIA ===\n");
    inicializar_ram();

    // Inicializamos las estructuras del Proceso 0
    PageTable tp0;
    tp0.num_pages = NUM_PAGINAS;
    tp0.entries = malloc(sizeof(PageTableEntry) * NUM_PAGINAS);
    for(int i=0; i<NUM_PAGINAS; i++) tp0.entries[i].valid = 0;
    tlb* tlb0 = init_tlb(16, 0);

    // Inicializamos las estructuras del Proceso 1
    PageTable tp1;
    tp1.num_pages = NUM_PAGINAS;
    tp1.entries = malloc(sizeof(PageTableEntry) * NUM_PAGINAS);
    for(int i=0; i<NUM_PAGINAS; i++) tp1.entries[i].valid = 0;
    tlb* tlb1 = init_tlb(16, 1);

    // Los registramos en el directorio global para el Allocator
    Thread_Tables[0] = &tp0;
    Thread_Tables[1] = &tp1;
    Thread_TLBs[0] = tlb0;
    Thread_TLBs[1] = tlb1;

    int hits0 = 0, misses0 = 0, faults0 = 0;
    int hits1 = 0, misses1 = 0, faults1 = 0;

    printf("Simulando accesos intercalados...\n");

    // Bucle principal: Alternamos accesos para forzar Evictions en la RAM
    for (int i = 0; i < ACCESOS_TOTALES; i++) {
        acceder_memoria(0, &tp0, tlb0, &hits0, &misses0, &faults0);
        acceder_memoria(1, &tp1, tlb1, &hits1, &misses1, &faults1);
    }

    printf("\n[Proceso 0] TLB Hits: %d | Misses: %d | Page Faults: %d\n", hits0, misses0, faults0);
    printf("[Proceso 1] TLB Hits: %d | Misses: %d | Page Faults: %d\n", hits1, misses1, faults1);

    // Limpieza
    free(tp0.entries);
    free(tp1.entries);
    destroy_tlb(tlb0);
    destroy_tlb(tlb1);

    printf("=== FIN DE LA PRUEBA ===\n");
    return 0;
}