#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <list.h>
#include "vm/page.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/palloc.h"

struct frame
{
    void *paddr;                    // physical address
    struct page_entry *page;        // mapped page entry
    struct thread *owner;           // owner thread
    struct list_elem elem;    // list elem for frame table
};

struct frame_table
{
    struct list list;               // frame table list
    struct frame *clock_hand;       // frame pointer for clock algorithm
    struct lock lock;               // lock for accessing frame table
};

void init_frame_table (void);
void add_frame (struct frame *);
struct frame * del_frame (struct frame *);
struct frame * find_frame (struct page_entry *);

struct frame * alloc_frame (enum palloc_flags flags);
bool free_frame (struct frame *);
#endif