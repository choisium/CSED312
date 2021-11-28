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
    
    struct frame *f = NULL;
    struct list_elem *e;

    lock_acquire(&frame_table->lock);

    /* Remove child from parent's child_list */
    for (e = list_begin (&frame_table->list); e != list_end (&frame_table->list);
        e = list_next (e))
      {
        f = list_entry (e, struct frame, elem);
        if (frame == f)
            list_remove(e);
      }

    lock_release(&frame_table->lock);

    ASSERT (f != NULL);

    if (f == NULL)
        return NULL;

    return f;
}

struct frame *
find_frame (struct page_entry* page)
{
    ASSERT (page != NULL);

    struct list_elem *e;

    lock_acquire(&frame_table->lock);

    for (e = list_begin (&frame_table->list); e != list_end (&frame_table->list);
        e = list_next (e))
        {
        struct frame *f = list_entry (e, struct frame, elem);
        if (page == f->page)
            return f;
        }

    lock_release(&frame_table->lock);
    
    return NULL;
}

struct frame *
allocate_frame (enum palloc_flags flags)
{
    uint8_t *kpage = palloc_get_page (flags);
    if (kpage == NULL)
        return NULL;

    struct frame *f = (struct frame *) malloc (sizeof (struct frame));

    f->paddr = (void *) kpage;
    f->owner = thread_current ();
    f->page = NULL;

    add_frame(f);
    return f;
}

bool
free_frame (struct frame *frame)
{
    ASSERT (frame != NULL);

    struct frame *f = del_frame(frame);
    if (f == NULL)
        return false;
    
    palloc_free_page(f->paddr);
    free (f);
    return true;
}

void
map_page_to_frame (struct frame *fr, struct page_entry *pe)
{
    fr->page = pe;
}