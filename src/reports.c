#include "reports.h"
#include <stdio.h>
#include <string.h>

void print_stats_report() {
    printf("\n");
    printf("==================================================\n");
    printf("         REPORTE FINAL DE LA SIMULACIÓN           \n");
    printf("==================================================\n");
    printf(" CONFIGURACIÓN:\n");
    printf("  - Modo:     %s\n", GlobConfig.mode);
    printf("  - Workload: %s\n", GlobConfig.workload);
    printf("  - Hilos:    %d\n", GlobConfig.num_threads);
    printf("  - Ops/Hilo: %d\n", GlobConfig.ops_per_thread);
    printf("--------------------------------------------------\n");
    printf(" MÉTRICAS GENERALES:\n");
    printf("  - Traducciones OK:   %d\n", GlobStats.total_translation_ok);
    printf("  - Segmentation Faults: %d\n", GlobStats.total_segfaults);
    
    if (strcmp(GlobConfig.mode, "tlb") == 0) {
        int total_tlb_accesor = GlobStats.total_tlb_hits + GlobStats.total_tlb_misses;
        float hit_rate = 0.0;
        if (total_tlb_accesor > 0) {
            hit_rate = ((float)GlobStats.total_tlb_hits / total_tlb_accesor) * 100;
        }

        printf("--------------------------------------------------\n");
        printf(" MÉTRICAS TLB:\n");
        printf("  - TLB Hits:   %d\n", GlobStats.total_tlb_hits);
        printf("  - TLB Misses: %d\n", GlobStats.total_tlb_misses);
        printf("  - TLB Hit Rate: %.2f%%\n", hit_rate);
    }

    printf("==================================================\n\n");
}

void export_stats_json(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("No se pudo crear el archivo de reporte");
        return;
    }

    fprintf(f, "{\n");
    fprintf(f, "  \"mode\": \"%s\",\n", GlobConfig.mode);
    fprintf(f, "  \"workload\": \"%s\",\n", GlobConfig.workload);
    fprintf(f, "  \"total_translation_ok\": %d,\n", GlobStats.total_translation_ok);
    fprintf(f, "  \"total_segfaults\": %d,\n", GlobStats.total_segfaults);
    fprintf(f, "  \"tlb_hits\": %d,\n", GlobStats.total_tlb_hits);
    fprintf(f, "  \"tlb_misses\": %d\n", GlobStats.total_tlb_misses);
    fprintf(f, "}\n");

    fclose(f);
    printf(">> Reporte exportado exitosamente a %s\n", filename);
}