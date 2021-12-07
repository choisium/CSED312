#include <bitmap.h>
#include "devices/block.h"
#include "vm/swap.h"
#include "threads/vaddr.h"
#include "vm/frame.h"

static struct block* swap_block;
static struct bitmap* swap_slot;

static size_t SECTORS_PER_PAGE = PGSIZE / BLOCK_SECTOR_SIZE;

void
swap_init (void)
{
    swap_block = block_get_role (BLOCK_SWAP);
    ASSERT (swap_block != NULL);

    size_t NUM_PAGE = block_size (swap_block) / SECTORS_PER_PAGE;
    swap_slot = bitmap_create (NUM_PAGE);
    ASSERT (swap_slot != NULL);

    bitmap_set_all (swap_slot, false);
}

swap_index_t
swap_out (struct frame* fr)
{
    size_t i;
    swap_index_t idx = bitmap_scan (swap_slot, 0, 1, false);
    void * paddr = fr->paddr;
    if (idx == BITMAP_ERROR)
      return SWAP_ERROR;

    for (i = 0; i < SECTORS_PER_PAGE; i++)
      {
          block_write (swap_block, SECTORS_PER_PAGE * idx + i, paddr);
          paddr += BLOCK_SECTOR_SIZE;
      }

    bitmap_flip (swap_slot, idx);
    return idx;
}

void
swap_in (swap_index_t used_idx, struct frame* fr)
{
    ASSERT (used_idx != SWAP_ERROR);
    ASSERT (bitmap_test (swap_slot, used_idx) == true);

    size_t i;
    void * paddr = fr->paddr;
    for (i = 0; i < SECTORS_PER_PAGE; i++)
      {
          block_read (swap_block, SECTORS_PER_PAGE * used_idx + i, paddr);
          paddr += BLOCK_SECTOR_SIZE;
      }
    
    bitmap_flip (swap_slot, used_idx);
}

void
delete_slot (swap_index_t idx)
{
  ASSERT (bitmap_test (swap_slot, idx) == true);
  bitmap_flip (swap_slot, idx);
}