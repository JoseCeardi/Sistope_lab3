#include <stdio.h>
#include <string.h>
#include "simulator.h"

void print_stats_report() {
    printf("\n==================================================\n");
    printf("           SIMULADOR DE MEMORIA VIRTUAL           \n");
    if (strcmp(GlobConfig.mode, "seg") == 0) {
        printf("Modo: SEGMENTACION\n");
    } else {
        printf("Modo: PAGINACION\n");
    }
    printf("==================================================\n");
    printf("Configuracion:\n");
    printf("  Threads: %d\n", GlobConfig.num_threads);
    printf("  Ops por thread: %d\n", GlobConfig.ops_per_thread);
    printf("  Workload: %s\n", GlobConfig.workload);
    printf("  Seed: %d\n", GlobConfig.seed);
    
    printf("\nMetricas Globales:\n");
    if (strcmp(GlobConfig.mode, "seg") == 0) {
        printf("  Traducciones OK: %d\n", GlobStats.total_translation_ok);
        printf("  Segfaults: %d\n", GlobStats.total_segfaults);
    } else {
        int total_tlb = GlobStats.total_tlb_hits + GlobStats.total_tlb_misses;
        double hit_rate = total_tlb > 0 ? (double)GlobStats.total_tlb_hits / total_tlb * 100.0 : 0.0;
        printf("  TLB Hits: %d\n", GlobStats.total_tlb_hits);
        printf("  TLB Misses: %d\n", GlobStats.total_tlb_misses);
        printf("  Hit Rate TLB: %.2f%%\n", hit_rate);
        printf("  Page Faults: %d\n", GlobStats.total_page_faults);
        printf("  Evictions: %d\n", GlobStats.total_evictions);
    }
    
    int total_ops = GlobConfig.num_threads * GlobConfig.ops_per_thread;
    double throughput = GlobStats.runtime_sec > 0 ? (double)total_ops / GlobStats.runtime_sec : 0.0;
    
    printf("\nTiempo total: %.3f segundos\n", GlobStats.runtime_sec);
    printf("Throughput: %.2f ops/seg\n", throughput);
    printf("Tiempo prom. traduccion: %ld ns\n", GlobStats.avg_translation_time_ns);
    printf("==================================================\n");
}





void export_stats_json(const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("Error al crear summary.json");
        return;
    }

    // Cálculos matemáticos requeridos
    double hit_rate = 0.0;
    int total_tlb = GlobStats.total_tlb_hits + GlobStats.total_tlb_misses;
    if (total_tlb > 0) {
        hit_rate = (double)GlobStats.total_tlb_hits / total_tlb;
    }

    int total_ops = GlobConfig.num_threads * GlobConfig.ops_per_thread;
    double throughput = 0.0;
    if (GlobStats.runtime_sec > 0) {
        throughput = (double)total_ops / GlobStats.runtime_sec;
    }

    // Construir el JSON
    fprintf(fp, "{\n");
    fprintf(fp, "  \"mode\": \"%s\",\n", GlobConfig.mode);
    fprintf(fp, "  \"config\": {\n");
    fprintf(fp, "    \"threads\": %d,\n", GlobConfig.num_threads);
    fprintf(fp, "    \"ops_per_thread\": %d,\n", GlobConfig.ops_per_thread);
    fprintf(fp, "    \"workload\": \"%s\",\n", GlobConfig.workload);
    fprintf(fp, "    \"seed\": %d,\n", GlobConfig.seed);
    fprintf(fp, "    \"unsafe\": %s,\n", GlobConfig.unsafe ? "true" : "false");
    
    if (strcmp(GlobConfig.mode, "seg") != 0) { // Si es paginación o TLB
        fprintf(fp, "    \"pages\": %d,\n", GlobConfig.pages);
        fprintf(fp, "    \"frames\": %d,\n", GlobConfig.frames);
        fprintf(fp, "    \"page_size\": %d,\n", GlobConfig.page_size);
        fprintf(fp, "    \"tlb_size\": %d,\n", GlobConfig.tlb_size);
        fprintf(fp, "    \"tlb_policy\": \"%s\",\n", GlobConfig.tlb_policy);
        fprintf(fp, "    \"evict_policy\": \"%s\"\n", GlobConfig.evict_policy);
    } else { // Si es segmentación
        fprintf(fp, "    \"segments\": %d\n", GlobConfig.num_segments);
    }
    fprintf(fp, "  },\n");

    fprintf(fp, "  \"metrics\": {\n");
    if (strcmp(GlobConfig.mode, "seg") == 0) {
        fprintf(fp, "    \"translations_ok\": %d,\n", GlobStats.total_translation_ok);
        fprintf(fp, "    \"segfaults\": %d,\n", GlobStats.total_segfaults);
    } else {
        fprintf(fp, "    \"tlb_hits\": %d,\n", GlobStats.total_tlb_hits);
        fprintf(fp, "    \"tlb_misses\": %d,\n", GlobStats.total_tlb_misses);
        fprintf(fp, "    \"hit_rate\": %.3f,\n", hit_rate);
        fprintf(fp, "    \"page_faults\": %d,\n", GlobStats.total_page_faults);
        fprintf(fp, "    \"evictions\": %d,\n", GlobStats.total_evictions);
    }
    // Métricas compartidas por ambos modos
    fprintf(fp, "    \"avg_translation_time_ns\": %ld,\n", GlobStats.avg_translation_time_ns);
    fprintf(fp, "    \"throughput_ops_sec\": %.2f\n", throughput);
    fprintf(fp, "  },\n");

    fprintf(fp, "  \"runtime_sec\": %.3f\n", GlobStats.runtime_sec);
    fprintf(fp, "}\n");

    fclose(fp);
    printf(">> Reporte exportado exitosamente a %s\n", filename);
}