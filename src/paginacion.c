#include <stdio.h>
#include <stdint.h>
#include "paginacion.h"

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
        return frame * PAGE_SIZE + offset; 
    }
}