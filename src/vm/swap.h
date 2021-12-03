#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <stddef.h>
#include <stdint.h>
#include "devices/block.h"

typedef size_t swap_index_t;
#define SWAP_ERROR SIZE_MAX

void swap_init (void);

struct frame;
swap_index_t swap_out (struct frame *);
void swap_in (swap_index_t, struct frame*);
#endif