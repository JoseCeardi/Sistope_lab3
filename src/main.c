#include <stdio.h>
#include <stdlib.h>
#include "simulator.h"
#include "input_manager.h"
#include "reports.h"

int main(int argc, char *argv[]) {
    // Leer y configurar los parámetros de entrada
    parse_arguments(argc, argv);

    // Inicializar la semilla de aleatoriedad
    srand(GlobConfig.seed);

    // Ejecutar el simulador
    start_simulation(&GlobConfig);

    if (GlobConfig.stats_report) {
        print_stats_report();
        export_stats_json("reporte.json"); // <--- AÑADE ESTA LÍNEA AQUÍ
    }

    // Limpiar memoria dinámica
    free_config();

    return 0;
}