#include "workloads.h"
#include <stdlib.h>
#include <string.h>

// Espacio virtual total: 64 páginas de 4096 bytes = 262,144 bytes
#define TOTAL_VIRTUAL_SPACE (64 * 4096)
// El 20% más usado (Hot Zone) = 52,428 bytes
#define HOT_ZONE_LIMIT (TOTAL_VIRTUAL_SPACE / 5) 

uint64_t generate_v_addr(const char* workload_type, unsigned int* seed) {
    
    // MODO: 80-20 (Principio de localidad)
    if (strcmp(workload_type, "8020") == 0 || strcmp(workload_type, "80-20") == 0) {
        int probabilidad = rand_r(seed) % 100;
        
        if (probabilidad < 80) {
            // El 80% de los accesos caen en el 20% de la memoria (páginas 0 a 12 aprox)
            return rand_r(seed) % HOT_ZONE_LIMIT;
        } else {
            // El 20% de los accesos caen en el resto de la memoria (páginas 13 a 63)
            uint64_t cold_zone_size = TOTAL_VIRTUAL_SPACE - HOT_ZONE_LIMIT;
            return HOT_ZONE_LIMIT + (rand_r(seed) % cold_zone_size);
        }
    }

    // MODO POR DEFECTO: Uniforme
    // Todos los accesos se reparten equitativamente en toda la memoria
    return rand_r(seed) % TOTAL_VIRTUAL_SPACE;
}