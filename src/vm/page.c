#include <hash.h>
#include "vm/page.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "threads/palloc.h"
#include <string.h>

bool spt_init (struct hash *h)
 {
   return hash_init(h, (void *) spt_hash_func, spt_less_func, NULL);
 }

uint32_t
spt_hash_func (const struct hash_elem *e, void *aux UNUSED)
  {
    const struct page_entry *p = hash_entry (e, struct page_entry, elem);
    return hash_int((uint32_t) p->vaddr);
  }

bool
spt_less_func (const struct hash_elem *a_, const struct hash_elem *b_, void *aux UNUSED) 
  {
    const struct page_entry *a = hash_entry (a_, struct page_entry, elem);
    const struct page_entry *b = hash_entry (b_, struct page_entry, elem);

    return a->vaddr < b->vaddr;
  }

struct page_entry *
spt_find_page (struct hash *spt, const void *vaddr)
 {
   struct page_entry p;
   struct hash_elem *e;

   p.vaddr = pg_round_down (vaddr);
   e = hash_find (spt, &p.elem);
   return e != NULL ? hash_entry (e, struct page_entry, elem) : NULL;
 }

bool 
spt_insert_page (struct hash *spt, struct page_entry *pe)
 {
   if (hash_insert (spt, &pe->elem) != NULL)
    return false;
   else
    return true;
 }

bool 
spt_delete_page (struct hash *spt, struct page_entry *pe)
  {
    if (hash_delete (spt, &pe->elem) != NULL)
      return true;
    else
      return false;
  }

void 
spt_destroy (struct hash *spt)
  {
    hash_destroy (spt, page_destructor);
  }

void
page_destructor (struct hash_elem *e, void *aux UNUSED)
  {
    struct page_entry *p = hash_entry (e, struct page_entry, elem);
    free (p);
  }

bool
set_page_entry (struct file *file, off_t ofs, uint8_t *upage, struct frame *fr,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable, enum page_type type) 
  {
    struct thread *t = thread_current ();

    if (spt_find_page (&t->spt, upage) != NULL)
      return false;
    
    struct page_entry *pe = malloc (sizeof (struct page_entry));
    if (pe == NULL)
      return false;
    
    pe->vaddr = upage;
    pe->frame = fr;
    pe->is_loaded = false;
    pe->writable = writable;
    pe->type = type;
    pe->file = file;
    pe->ofs = ofs;
    pe->read_bytes = read_bytes;
    pe->zero_bytes = zero_bytes;
    pe->swap_index = SWAP_ERROR;
    
    spt_insert_page(&t->spt, pe);

    if (fr != NULL)
      {
        map_page_to_frame(fr, pe);
      }
    
    return true;
  }

/* Load the page from the disk. */
bool
load_file (void *kaddr, struct page_entry *pe)
{
  if (file_read_at (pe->file, kaddr, pe->read_bytes, pe->ofs) 
    != (int) pe->read_bytes)
    {
      palloc_free_page (kaddr);
      return false; 
    }
  
  memset (kaddr + pe->read_bytes, 0, pe->zero_bytes);
  return true;
}

void
map_frame_to_page (struct page_entry *pe, struct frame *fr)
{
  ASSERT (pe->frame == NULL)

  pe->frame = fr;
}