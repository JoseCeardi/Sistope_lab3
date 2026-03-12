#include "paginacion.h"
#include <stdio.h>

int main() {
    printf("=== TEST PAGINACIÓN ===\n");

    int total_paginas = 64;
    PageTable* tp = init_page_table(total_paginas);
    
    int hits = 0;
    int misses = 0;
    int oob = 0; 

    // Inicializar solo pares para forzar errores
    for (int i = 0; i < total_paginas; i += 2) {
        tp->entries[i].valid = 1;
        tp->entries[i].frame_number = i * 2; 
    }

    printf("Simulando accesos del 0 al 99...\n");

    // probar direcciones
    for (int i = 0; i < 100; i++) {
        int pagina_a_probar = i;
        int dir = traductVirtualDir(pagina_a_probar, 100, tp);

        if (dir != -1) {
            hits++;
        } else {
            if (pagina_a_probar >= total_paginas) {
                oob++; // Debería contar desde la 64 hasta la 99 (36 páginas)
            } else {
                misses++; // Debería contar las impares del 0 al 63 (32 páginas)
            }
        }
    }

    // Resultados
    printf("\n--- Results ---\n");
    printf("Accesos Totales    : 100\n");
    printf("Hits (Pares 0-63)  : %d\n", hits);
    printf("Misses (Impares)   : %d\n", misses);
    printf("Fuera de Rango     : %d\n", oob);

    if (hits == 32 && misses == 32 && oob == 36) {
        printf("RESULTADO: TEST PASADO\n");
    } else {
        printf("RESULTADO: REVISAR LÓGICA\n");
    }

    destroy_page_table(tp);
    return 0;
}