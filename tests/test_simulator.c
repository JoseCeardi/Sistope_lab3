#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simulator.h"

int main() {
    GlobConfig.mode = "tlb";
    GlobConfig.num_threads = 4;
    GlobConfig.ops_per_thread = 1000;
    GlobConfig.unsafe = false;
    GlobConfig.seed = 42;
    srand(GlobConfig.seed);

    printf("Corriendo simulador en modo '%s'...\n", GlobConfig.mode);
    start_simulation(&GlobConfig);

    printf("\n--- Estadísticas ---\n");
    printf("Traducciones OK : %d\n", GlobStats.total_translation_ok);
    printf("Page Faults     : %d\n", GlobStats.total_page_faults);
    printf("Evictions       : %d\n", GlobStats.total_evictions);
    printf("Segfaults       : %d\n", GlobStats.total_segfaults);

    if (strcmp(GlobConfig.mode, "tlb") == 0 || strcmp(GlobConfig.mode, "page") == 0) {
        int accesos = GlobStats.total_tlb_hits + GlobStats.total_tlb_misses;
        float hr = accesos > 0 ? ((float)GlobStats.total_tlb_hits / accesos) * 100 : 0;
        
        printf("TLB Hits        : %d\n", GlobStats.total_tlb_hits);
        printf("TLB Misses      : %d\n", GlobStats.total_tlb_misses);
        printf("Hit Rate        : %.2f%%\n", hr);
    }

    return 0;
}