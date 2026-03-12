#ifndef WORKLOADS_H
#define WORKLOADS_H

#include <stdint.h>

// Genera una dirección virtual simulada según el tipo de carga
uint64_t generate_v_addr(const char* workload_type, unsigned int* seed);

#endif