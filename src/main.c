#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h> 
#include "simulator.h"

int main(int argc, char *argv[]) {
    // Inicializar GlobConfig con los valores por defecto 
    GlobConfig.mode = NULL;           
    GlobConfig.num_threads = 1;       // Default: 1
    GlobConfig.ops_per_thread = 1000; // Default: 1000
    GlobConfig.workload = "uniform";  // Default: uniform
    GlobConfig.seed = 42;             // Default: 42
    GlobConfig.unsafe = false;        // Default: false (FALSE = modo SAFE)
    GlobConfig.stats_report = false;  // Default: false
    GlobConfig.num_segments = 4;      // Default: 4

    // Memoria por defecto para 4 segmentos de 4096 bytes
    GlobConfig.seg_limits = malloc(4 * sizeof(uint64_t));
    for (int i = 0; i < 4; i++) {
        GlobConfig.seg_limits[i] = 4096;
    }

    
    int opt;
    int option_index = 0;
    static struct option long_options[] = {
        // { "nombre_flag", tiene_argumento, flag_ptr, 'letra_atajo' }
        {"mode",           required_argument, 0, 'm'},
        {"threads",        required_argument, 0, 't'},
        {"ops-per-thread", required_argument, 0, 'o'},
        {"workload",       required_argument, 0, 'w'},
        {"seed",           required_argument, 0, 's'},
        {"unsafe",         no_argument,       0, 'u'},
        {"stats",          no_argument,       0, 'r'}, // r de report
        {"segments",       required_argument, 0, 'g'}, // g de segment
        {"seg-limits",     required_argument, 0, 'l'}, // l de limits
        // TODO: falta agregar flags
        {0, 0, 0, 0}
    };


    // Los ":" indican que esa letra espera un valor después
    while ((opt = getopt_long(argc, argv, "m:t:o:w:s:urg:l:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'm': GlobConfig.mode = strdup(optarg); break;
            case 't': GlobConfig.num_threads = atoi(optarg); break;
            case 'o': GlobConfig.ops_per_thread = atoi(optarg); break;
            case 'w': GlobConfig.workload = strdup(optarg); break;
            case 's': GlobConfig.seed = atoi(optarg); break;
            case 'u': GlobConfig.unsafe = true; break;
            case 'r': GlobConfig.stats_report = true; break;
            case 'g': GlobConfig.num_segments = atoi(optarg); break;
            case 'l': {
                
                free(GlobConfig.seg_limits); // Liberamos el default
                GlobConfig.seg_limits = malloc(GlobConfig.num_segments * sizeof(uint64_t));
                
                int i = 0;
                // strtok corta un texto usando un delimitador (la coma)
                char *token = strtok(optarg, ",");
                while (token != NULL && i < GlobConfig.num_segments) {
                    GlobConfig.seg_limits[i] = strtoull(token, NULL, 10);
                    token = strtok(NULL, ",");
                    i++;
                }
                break;
            }
            default:
                fprintf(stderr, "Uso incorrecto de argumentos.\n");
                exit(EXIT_FAILURE);
        }
    }

    // Validar que ingresaron el flag obligatorio --mode
    if (GlobConfig.mode == NULL) {
        fprintf(stderr, "Error: El flag --mode {seg|page} es obligatorio.\n");
        exit(EXIT_FAILURE);
    }


    srand(GlobConfig.seed);


    start_simulation(&GlobConfig);

    // TODO: Llamar al método que genera los reportes en consola y JSON si GlobConfig.stats_report es true.

    return 0;
}















//main vienj, borrar despues
/* #include <stdio.h>
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
} */