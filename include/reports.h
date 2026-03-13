#ifndef REPORTS_H
#define REPORTS_H

#include "simulator.h"

// Imprime un reporte detallado en la consola
void print_stats_report();

// Genera un archivo .json con los resultados
void export_stats_json(const char* filename);

#endif