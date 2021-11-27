#include <hash.h>
#include "vm/page.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"

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

/* Compare function for wakeup_tick in acending order*/
bool
spt_less_func (const struct hash_elem *a_, const struct hash_elem *b_, void *aux UNUSED) 
  {
    const struct page_entry *a = hash_entry (a_, struct page_entry, elem);
    const struct page_entry *b = hash_entry (b_, struct page_entry, elem);

    return a->vaddr < b->vaddr;
  }

struct page_entry *
spt_find_page (struct hash *spt, void *vaddr)
 {
   struct page_entry p;
   struct hash_elem *e;

   p.vaddr = pg_round_down (vaddr);
   e = hash_find (spt, &p.elem);
   return e != NULL ? hash_entry (e, struct page_entry, elem) : NULL;
 }

bool 
spt_insert_page (struct hash *spt, struct page_entry *page)
 {
   if (hash_insert (spt, &page->elem) != NULL)
    return false;
   else
    return true;
 }

bool 
spt_delete_page (struct hash *spt, struct page_entry *page)
  {
    if (hash_delete (spt, &page->elem) != NULL)
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