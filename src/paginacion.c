#include <stdio.h>
#include <stdint.h>
#include "paginacion.h"


// Tanto la SegTable como la PageTable son privadas de cada proceso,
// es decir que no son datos globables y no requieren proteccion contra problemas
// de concurrencia como CC


PageTable* init_page_table(int num_pages) {
    PageTable* table = (PageTable*)malloc(sizeof(PageTable));
    if (!table) {
        perror("Error al asignar PageTable");
        exit(-1);
    }

    table->num_pages = num_pages;
    table->entries = (PageTableEntry*)malloc(num_pages * sizeof(PageTableEntry));
    
    if (!table->entries) {
        perror("Error al asignar entradas de PageTable");
        free(table);
        exit(-1);
    }

    for (int i = 0; i < num_pages; i++) {
        table->entries[i].valid = 0;
        table->entries[i].frame_number = -1;
        table->entries[i].dirty = 0;
    }

    return table;
}


// obtiene el nro de marco de una pagina (si no esta cargada devuelve -1)
int getFrameByPage(int page_index, PageTable* TP){
    if (page_index < 0 || page_index >= TP->num_pages){ 
        return -1;
    }

    PageTableEntry entrada = TP->entries[page_index];

    if (entrada.valid == 1){
        return entrada.frame_number; 
    } else {
        return -1;
    }
}

// Traduce una direccion virtual a una dirección física en memoria
int traductVirtualDir(int page_index, int offset, PageTable* TP){
    int frame = getFrameByPage(page_index, TP);

    if (frame == -1){
        return -1;
    } else {
        return frame * PAGE_SIZE + offset;  // concatena offset
    }
}


void destroy_page_table(PageTable* TP) {
    if (TP) {
        if (TP->entries) free(TP->entries);
        free(TP);
    }
}