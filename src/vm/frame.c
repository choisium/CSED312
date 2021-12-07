#include <list.h>
#include "vm/frame.h"
#include "userprog/process.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "vm/swap.h"

static struct frame_table* frame_table;
 
void
init_frame_table (void)
{
    frame_table = (struct frame_table*) malloc (sizeof (struct frame_table));
    
    list_init(&frame_table->list);
    lock_init(&frame_table->lock);
    frame_table->clock_hand = NULL;
}

void
add_frame (struct frame* frame)
{   
    list_push_back(&frame_table->list, &frame->elem);
}

struct frame*
del_frame (struct frame* frame)
{
    ASSERT (frame != NULL);
    
    struct frame *fr = NULL;
    struct list_elem *e;
    bool acquire_lock = false;

    if (frame->page != NULL)
      {
        unmap_frame (frame->page);
        unmap_page (frame);
      }
    
    if (!lock_held_by_current_thread (&frame_table->lock)) {
        acquire_lock = true;
        lock_acquire(&frame_table->lock);
    }

    /* Remove child from parent's child_list */
    for (e = list_begin (&frame_table->list); e != list_end (&frame_table->list);
        e = list_next (e))
      {
        fr = list_entry (e, struct frame, elem);
        if (frame == fr) {
            list_remove(e);
            break;
        }
      }

    if (acquire_lock) {
        lock_release(&frame_table->lock);
    }
    ASSERT (fr != NULL);

    if (fr == NULL)
        return NULL;

    return fr;
}

struct frame *
find_frame (struct page_entry* pe)
{
    ASSERT (pe != NULL);
    
    struct frame *fr = NULL;
    struct list_elem *e;

    lock_acquire(&frame_table->lock);

    for (e = list_begin (&frame_table->list); e != list_end (&frame_table->list);
        e = list_next (e))
      {
        fr = list_entry (e, struct frame, elem);
        if (pe == fr->page)
            break;
      }

    lock_release(&frame_table->lock);
    
    return fr;
}

struct frame *
allocate_frame (enum palloc_flags flags)
{
    lock_acquire(&frame_table->lock);

    uint8_t *kpage = palloc_get_page (flags);
    if (kpage == NULL)
      {
        if (!evict_frame ())
          return NULL;
        kpage = palloc_get_page (flags);
      }

    ASSERT (kpage != NULL);

    struct frame *fr = (struct frame *) malloc (sizeof (struct frame));

    fr->paddr = (void *) kpage;
    fr->owner = thread_current ();
    fr->page = NULL;
    fr->pinned = false;

    add_frame(fr);

    lock_release(&frame_table->lock);

    return fr;
}

bool
free_frame (struct frame *frame)
{
    ASSERT (frame != NULL);

    struct frame *fr = del_frame(frame);
    if (fr == NULL)
        return false;
    
    palloc_free_page(fr->paddr);
    free (fr);
    return true;
}

void
destroy_frame_table ()
{
  while (!list_empty (&frame_table->list))
    {
      struct list_elem *e = list_pop_front (&frame_table->list);
      struct frame *fr = list_entry (e, struct frame, elem);
      free_frame (fr);
    }
}

void
map_page_to_frame (struct frame *fr, struct page_entry *pe)
{
    ASSERT (fr->page == NULL)

    fr->page = pe;
}

void
unmap_page (struct frame *fr)
{
  fr->page = NULL;
}

struct frame *
choose_victim (void)
{
    ASSERT (lock_held_by_current_thread (&frame_table->lock));

    struct frame *victim = NULL;
    struct list_elem *clock_hand_elem;
    struct list_elem *e;

    if (frame_table->clock_hand == NULL)
      clock_hand_elem = list_begin(&frame_table->list);
    else
      clock_hand_elem = &frame_table->clock_hand->elem;

    int count = 0;
    while (count < 2) {

    /* Choose Victim */
    for (e = clock_hand_elem; e != list_end (&frame_table->list);
        e = list_next (e))
      {
        struct frame *fr = list_entry (e, struct frame, elem);
        if(fr->pinned)
          continue;

        if (!pagedir_is_accessed(fr->owner->pagedir, fr->page->vaddr))
          {
            victim = fr;
            clock_hand_elem = list_next (e);
            break;
          }
        else
          {
            pagedir_set_accessed(fr->owner->pagedir, fr->page->vaddr, false);
          }
      }

    if (victim != NULL)
      {
        frame_table->clock_hand = list_entry (clock_hand_elem, struct frame, elem);
        return victim;
      }

    /* If not found until list_end, start from begin */
    for (e = list_begin (&frame_table->list); e != clock_hand_elem;
          e = list_next (e))
      {
        struct frame *fr = list_entry (e, struct frame, elem);
        if(fr->pinned)
          continue;
        
        if (!pagedir_is_accessed(fr->owner->pagedir, fr->page->vaddr))
          {
            victim = fr;
            clock_hand_elem = list_next (e);
            break;
          }
        else
          {
            pagedir_set_accessed(fr->owner->pagedir, fr->page->vaddr, false);
          }
      }
    
    if (victim != NULL)
    {
      frame_table->clock_hand = list_entry(clock_hand_elem, struct frame, elem);
      return victim;
    }
    count++;
    }

    frame_table->clock_hand = list_entry (clock_hand_elem, struct frame, elem);
    
    return victim;
}

bool
evict_frame (void)
{
    ASSERT (lock_held_by_current_thread (&frame_table->lock));
    
    struct frame *victim = choose_victim ();
    ASSERT (victim != NULL);
    /* If victim is dirty, Swap out. */
    if (pagedir_is_dirty (victim->owner->pagedir, victim->page->vaddr))
      {
          /* If PG_MMAP, just write at file. */
          if (victim->page->type == PG_MMAP)
            {
                if (victim->page->writable) 
                  {
                    file_write_at(victim->page->file, victim->page->vaddr, 
                                victim->page->read_bytes, victim->page->ofs);
                  }
            }
          
          /* Swap out the others. */
          else
            {
                victim->page->type = PG_SWAP;

                swap_index_t swap_idx = swap_out (victim);
                if (swap_idx == SWAP_ERROR)
                    return false;
                
                victim->page->swap_index = swap_idx;
            }
          
          pagedir_set_dirty (victim->owner->pagedir, victim->page->vaddr, false);
      }
    
    pagedir_clear_page (victim->owner->pagedir, victim->page->vaddr);
    victim->page->frame = NULL;

    if (!free_frame(victim))
      return false;

    return true;
}
