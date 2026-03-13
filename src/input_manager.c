#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "simulator.h"
#include "input_manager.h"

void parse_arguments(int argc, char *argv[]) {
    // Valores por defecto Generales
    GlobConfig.mode = NULL;           
    GlobConfig.num_threads = 1;       
    GlobConfig.ops_per_thread = 1000; 
    GlobConfig.workload = "uniform";  
    GlobConfig.seed = 42;             
    GlobConfig.unsafe = false;        
    GlobConfig.stats_report = false;  
    
    // Valores por defecto Segmentación
    GlobConfig.num_segments = 4;      
    GlobConfig.seg_limits = malloc(4 * sizeof(uint64_t));
    for (int i = 0; i < 4; i++) {
        GlobConfig.seg_limits[i] = 4096;
    }

    // Valores por defecto Paginación
    GlobConfig.pages = 64;
    GlobConfig.frames = 32;
    GlobConfig.page_size = 4096;
    GlobConfig.tlb_size = 16;
    GlobConfig.tlb_policy = "fifo";
    GlobConfig.evict_policy = "fifo";

    int opt;
    int option_index = 0;
    static struct option long_options[] = {
        // Generales
        {"mode",           required_argument, 0, 'm'},
        {"threads",        required_argument, 0, 't'},
        {"ops-per-thread", required_argument, 0, 'o'},
        {"workload",       required_argument, 0, 'w'},
        {"seed",           required_argument, 0, 's'},
        {"unsafe",         no_argument,       0, 'u'},
        {"stats",          no_argument,       0, 'r'}, 
        // Segmentacion
        {"segments",       required_argument, 0, 'g'}, 
        {"seg-limits",     required_argument, 0, 'l'}, 
        // Paginacion
        {"pages",          required_argument, 0, 'p'},
        {"frames",         required_argument, 0, 'f'},
        {"page-size",      required_argument, 0, 'z'},
        {"tlb-size",       required_argument, 0, 'b'},
        {"tlb-policy",     required_argument, 0, 'y'},
        {"evict-policy",   required_argument, 0, 'e'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "m:t:o:w:s:urg:l:p:f:z:b:y:e:", long_options, &option_index)) != -1) {
        switch (opt) {
            // Generales
            case 'm': GlobConfig.mode = strdup(optarg); break;
            case 't': GlobConfig.num_threads = atoi(optarg); break;
            case 'o': GlobConfig.ops_per_thread = atoi(optarg); break;
            case 'w': GlobConfig.workload = strdup(optarg); break;
            case 's': GlobConfig.seed = atoi(optarg); break;
            case 'u': GlobConfig.unsafe = true; break;
            case 'r': GlobConfig.stats_report = true; break;
            // Segmentacion
            case 'g': GlobConfig.num_segments = atoi(optarg); break;
            case 'l': {
                free(GlobConfig.seg_limits); 
                GlobConfig.seg_limits = malloc(GlobConfig.num_segments * sizeof(uint64_t));
                
                int i = 0;
                char *token = strtok(optarg, ",");
                while (token != NULL && i < GlobConfig.num_segments) {
                    GlobConfig.seg_limits[i] = strtoull(token, NULL, 10);
                    token = strtok(NULL, ",");
                    i++;
                }
                break;
            }
            // Paginacion
            case 'p': GlobConfig.pages = atoi(optarg); break;
            case 'f': GlobConfig.frames = atoi(optarg); break;
            case 'z': GlobConfig.page_size = atoi(optarg); break;
            case 'b': GlobConfig.tlb_size = atoi(optarg); break;
            case 'y': GlobConfig.tlb_policy = strdup(optarg); break;
            case 'e': GlobConfig.evict_policy = strdup(optarg); break;
            default:
                fprintf(stderr, "Uso incorrecto de argumentos.\n");
                exit(EXIT_FAILURE);
        }
    }

    // Validar flag obligatorio
    if (GlobConfig.mode == NULL) {
        fprintf(stderr, "Error: El flag --mode {seg|page} es obligatorio.\n");
        exit(EXIT_FAILURE);
    }
}

void free_config() {
    if (GlobConfig.seg_limits != NULL) free(GlobConfig.seg_limits);
    if (GlobConfig.mode != NULL) free(GlobConfig.mode);
    if (strcmp(GlobConfig.workload, "uniform") != 0) free((char*)GlobConfig.workload);
    if (strcmp(GlobConfig.tlb_policy, "fifo") != 0) free((char*)GlobConfig.tlb_policy);
    if (strcmp(GlobConfig.evict_policy, "fifo") != 0) free((char*)GlobConfig.evict_policy);
}