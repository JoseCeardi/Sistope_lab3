#ifndef TLB_H
#define TLB_H

#include <stdint.h>
#include <stdbool.h>

/* La tlb es basicamente una tabla de paginas pero más pequeña
*/

typedef struct{
    uint64_t frame_number; 
    int valid;
    int vpn; // page_number
} tlb_entry; 


typedef struct {
    tlb_entry *entries;
    int size;
    int count;
    int next_index;
    int thread_id; // tecnicamente cada procesador tiene una tlb única
                   // pero para debuggin servirá
} tlb;


tlb* init_tlb(int size, int thread_id);
int search_tlb(tlb* tlb_ptr, int vpn);

void update_tlb(tlb *tlb_ptr, int vpn, int frame);
void invalidate_tlb_entry(tlb *tlb, uint64_t vpn_quitar);
void destroy_tlb(tlb *tlb_ptr);

#endif