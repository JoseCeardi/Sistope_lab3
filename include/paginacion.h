#ifndef PAGINACION_H
#define PAGINACION_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    int frame_number;
    int valid;            
    int dirty;            
} PageTableEntry;

// Definición con nombre para compatibilidad
struct PageTable {
    PageTableEntry *entries;
    int num_pages;
};
typedef struct PageTable PageTable;

// Prototipos
PageTable* init_page_table(int num_pages);
int getFrameByPage(int page_index, PageTable* TP);
int traductVirtualDir(int page_index, int offset, PageTable* TP);
void destroy_page_table(PageTable* TP);

#endif