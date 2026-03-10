#ifndef PAGINACION_H
#define PAGINACION_H

#include <stdint.h>
#include <stdbool.h>

#define PAGE_SIZE 4096 
#define MAX_THREADS 10 

typedef struct {
    uint64_t frame_number;
    int valid;            
    int dirty;            
} PageTableEntry;

typedef struct {
    PageTableEntry *entries;
    int num_pages;
} PageTable;

// Funciones
int getFrameByPage(int page_index, PageTable* TP);
int traductVirtualDir(int page_index, int offset, PageTable* TP);

#endif