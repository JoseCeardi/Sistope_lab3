#include <stdio.h>
#include <stdlib.h>
#include "paging.h"

int main() {
    inicializar_sistema();

    PageTable tp_hilo0;
    tp_hilo0.num_pages = 100; // Simulamos que el hilo usa 100 páginas
    tp_hilo0.entries = malloc(sizeof(PageTableEntry) * tp_hilo0.num_pages);
    
    for(int i = 0; i < tp_hilo0.num_pages; i++) {
        tp_hilo0.entries[i].valid = 0;
        tp_hilo0.entries[i].frame_number = 0;

        
    }

    Thread_Tables[0] = &tp_hilo0;

    for (int i = 0; i < NUM_FRAMES; i++) {
        int marco_asignado = requestReplaceFrame(0, i); 
        tp_hilo0.entries[i].frame_number = marco_asignado;
        tp_hilo0.entries[i].valid = 1;
        
        printf("Página Virtual %2d -> Cargada en Frame Físico %2d\n", i, marco_asignado);
    }

    int marco_victima = requestReplaceFrame(0, 32);
    
    tp_hilo0.entries[32].frame_number = marco_victima;
    tp_hilo0.entries[32].valid = 1;


    free(tp_hilo0.entries);
    return 0;
}