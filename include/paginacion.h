#ifndef PAGINACION_H
#define PAGINACION_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define PAGE_SIZE 4096 
#define MAX_THREADS 10 

typedef struct {
    int frame_number;
    int valid;            
    int dirty;            
} PageTableEntry;

typedef struct {
    PageTableEntry *entries;
    int num_pages;
} PageTable;

// Funciones
PageTable* init_page_table(int num_pages);
int getFrameByPage(int page_index, PageTable* TP);
int traductVirtualDir(int page_index, int offset, PageTable* TP);
void destroy_page_table(PageTable* TP);


#endif