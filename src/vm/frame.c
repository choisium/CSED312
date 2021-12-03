#include <list.h>
#include "vm/frame.h"
#include "userprog/process.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"

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
    lock_acquire(&frame_table->lock);
    list_push_back(&frame_table->list, &frame->elem);
    lock_release(&frame_table->lock);
}

struct frame*
del_frame (struct frame* frame)
{
    ASSERT (frame != NULL);
    
    struct frame *fr = NULL;
    struct list_elem *e;

    lock_acquire(&frame_table->lock);

    /* Remove child from parent's child_list */
    for (e = list_begin (&frame_table->list); e != list_end (&frame_table->list);
        e = list_next (e))
      {
        fr = list_entry (e, struct frame, elem);
        if (frame == fr)
            list_remove(e);
      }

    lock_release(&frame_table->lock);

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
    uint8_t *kpage = palloc_get_page (flags);
    if (kpage == NULL)
        return NULL;

    struct frame *fr = (struct frame *) malloc (sizeof (struct frame));

    fr->paddr = (void *) kpage;
    fr->owner = thread_current ();
    fr->page = NULL;

    add_frame(fr);
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
map_page_to_frame (struct frame *fr, struct page_entry *pe)
{
    fr->page = pe;
}