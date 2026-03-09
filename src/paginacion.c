#include <stdio.h>
#include <stdint.h>
#include "paginacion.h"

#define PAGE_SIZE 4096 // Tamaño de página
#define NUM_FRAMES 32
#define MAX_THREADS 10

Frame RAM[NUM_FRAMES];
ColaFifo Global_Fifo;
PageTable* Thread_Tables[MAX_THREADS];

void inicializar_sistema() {
    for (int i = 0; i < NUM_FRAMES; i++) {
        RAM[i].valid = 0;      // Libre
        RAM[i].thread_id = -1;
        RAM[i].page_index = -1;
    }
    Global_Fifo.head = 0;
    Global_Fifo.count = 0;
}


// obtiene el nro de marco de una pagina (si no esta cargada devuelve -1)
int getFrameByPage(int page_index, PageTable* TP){
    if (page_index < 0 || page_index >= TP->num_pages){ //nos pasamos
        return -1;
    }

    // Obtenemos la entrada de la tabla
    PageTableEntry entrada = TP->entries[page_index];

    // vemos si esta cargada la pagina sino retorna -1
    if (entrada.valid == 1){
        return entrada.frame_number; // si esta cargada -> retorna el nro marco
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
        return frame*PAGE_SIZE + offset; //equivalente a concatenar
    }
}


int requestReplaceFrame(int t_id, int page_index) {
    int frame = -1;

    //  Intentar encontrar un marco libre
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (RAM[i].valid == 0) {
            frame = i;
            int pos = (Global_Fifo.head + Global_Fifo.count) % NUM_FRAMES;
            Global_Fifo.history[pos] = i;
            Global_Fifo.count++;
            break;
        }
    }

    //  Hechar al más viejo (Eviction)
    if (frame == -1) {
        frame = Global_Fifo.history[Global_Fifo.head];

        int victim_thread = RAM[frame].thread_id;
        int victim_page = RAM[frame].page_index;

        if (Thread_Tables[victim_thread] != NULL) {
            Thread_Tables[victim_thread]->entries[victim_page].valid = 0;
        }

        Global_Fifo.history[Global_Fifo.head] = frame;
        Global_Fifo.head = (Global_Fifo.head + 1) % NUM_FRAMES;
    }

    // Actualizar la RAM con los datos del nuevo dueño
    RAM[frame].thread_id = t_id;
    RAM[frame].page_index = page_index;
    RAM[frame].valid = 1;

    return frame; 
}



