#include <stdlib.h>
#include "tlb.h"


tlb* init_tlb(int size, int thread_id) {
    if (size <= 0) return NULL;
    tlb* new_tlb = malloc(sizeof(tlb));
    new_tlb->size = size;
    new_tlb->count = 0;
    new_tlb->next_index = 0;
    new_tlb->entries = malloc(sizeof(tlb_entry) * size);
    new_tlb->thread_id = thread_id;

    for (int i = 0; i < size; i++) {
        new_tlb->entries[i].valid = 0;
    }
    return new_tlb;
}

// buscar un marco disponible en la tlb
int search_tlb(tlb *tlb_ptr, int vpn){
    if (tlb_ptr == NULL || tlb_ptr->size == 0) return -1;

    for (int i=0; i<tlb_ptr->size; i++){
        if (tlb_ptr->entries[i].valid && tlb_ptr->entries[i].vpn == vpn) {
            return tlb_ptr -> entries[i].frame_number;
        }
    }   
    return -1;
}


// actualizar un elemento de la tlb, si esta llena esto hace eviction con fifo
void update_tlb(tlb *tlb_ptr, int vpn, int frame){
    if (tlb_ptr == NULL || tlb_ptr->size == 0) return;

    int idx = tlb_ptr->next_index;
    tlb_ptr->entries[idx].vpn = vpn;
    tlb_ptr->entries[idx].frame_number = frame;
    tlb_ptr->entries[idx].valid = 1;

    // fifo circular
    tlb_ptr-> next_index = (idx +1) % tlb_ptr-> size;
    if(tlb_ptr-> count < tlb_ptr -> size) tlb_ptr-> count++;
}



void invalidate_tlb_entry(tlb *tlb, uint64_t vpn_quitar){
    if (tlb == NULL || tlb -> size ==0) return;

    for (int i=0; i<tlb->size; i++){
        if (tlb->entries[i].valid == 1 && tlb->entries[i].vpn == (int)vpn_quitar) {
            tlb -> entries[i].valid = 0;
            break;
        }
    }
}



void destroy_tlb(tlb *tlb_ptr) {
    if (tlb_ptr) {
        free(tlb_ptr->entries);
        free(tlb_ptr);
    }
}