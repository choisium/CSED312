#include <bitmap.h>
#include "devices/block.h"
#include "vm/swap.h"
#include "threads/vaddr.h"

static struct block* swap_block;
static struct bitmap* swap_slot;

static size_t SECTORS_PER_PAGE = PGSIZE / BLOCK_SECTOR_SIZE;

void
swap_init ()
{
    swap_block = block_get_role (BLOCK_SWAP);
    ASSERT (swap_block != NULL);

    size_t NUM_PAGE = block_size (swap_block) / SECTORS_PER_PAGE;
    swap_slot = bitmap_create (NUM_PAGE);
    ASSERT (swap_slot != NULL);

    bitmap_set_all (swap_slot, false);
}