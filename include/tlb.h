#ifndef TLB_H
#define TLB_H

#include <stdint.h>

typedef struct {
    int vpn;
    int frame_number;
    int valid;
} tlb_entry;

struct tlb {
    int size;
    int count;
    int next_index;
    tlb_entry *entries;
    int thread_id;
};
typedef struct tlb tlb;

// Funciones
tlb* init_tlb(int size, int thread_id);
int search_tlb(tlb *tlb_ptr, int vpn);
void update_tlb(tlb *tlb_ptr, int vpn, int frame);
void invalidate_tlb_entry(tlb *tlb, uint64_t vpn_quitar);
void destroy_tlb(tlb *tlb_ptr);

#endif