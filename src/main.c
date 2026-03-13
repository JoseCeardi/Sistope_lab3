#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h> // La librería POSIX clásica que no tira errores en VS Code
#include "simulator.h"
#include "input_manager.h"
#include "reports.h"

int main(int argc, char *argv[]) {
    // Leer y configurar los parámetros de entrada
    parse_arguments(argc, argv);

    // SEGURO DE VIDA ANTI-CRASHEO (Evita el Floating Point Exception)
    if (GlobConfig.page_size == 0) GlobConfig.page_size = 4096;
    if (GlobConfig.pages == 0) GlobConfig.pages = 64;
    if (GlobConfig.num_segments == 0) GlobConfig.num_segments = 4;

    // Inicializar la semilla de aleatoriedad
    srand(GlobConfig.seed);

    // Preparar cronómetros con timeval (No usa CLOCK_MONOTONIC)
    struct timeval start, end;
    
    // Iniciar cronómetro
    gettimeofday(&start, NULL);

    // Ejecutar el simulador principal
    start_simulation(&GlobConfig);

    // Detener cronómetro
    gettimeofday(&end, NULL);
    
    // Calcular métricas de tiempo
    GlobStats.runtime_sec = (end.tv_sec - start.tv_sec) + 
                            (end.tv_usec - start.tv_usec) / 1000000.0;
    
    // Calcular tiempo promedio por operación en nanosegundos
    int total_ops = GlobConfig.num_threads * GlobConfig.ops_per_thread;
    if (total_ops > 0) {
        GlobStats.avg_translation_time_ns = (long)((GlobStats.runtime_sec * 1000000000.0) / total_ops);
    } else {
        GlobStats.avg_translation_time_ns = 0;
    }

    // Generar reportes si se solicitó con el flag --stats
    if (GlobConfig.stats_report) {
        print_stats_report();
        export_stats_json("out/summary.json"); 
    }

    // Limpiar memoria dinámica
    free_config();

    return 0;
}