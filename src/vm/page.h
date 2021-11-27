#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "filesys/file.h"
/* Types of page entry. */
enum page_type
  {
    PG_FILE,            /* Load from Binary. */
    PG_MMAP,            /* Load from Memory Mapped File. */
    PG_SWAP             /* Load from Swap slot. */
  };

struct page_entry
  {
    void* vaddr;               // VPA
    bool is_loaded;            
    bool writable;
    enum page_type type;       // page type

    struct file* file;         // mapped file

    // struct frame* frame
    void* kaddr;

    off_t ofs;                 // file offset
    uint32_t read_bytes;
    uint32_t zero_bytes;
    struct hash_elem elem;     // hash elem for spt hash table
  };

/* SPT Table Initialization */
bool spt_init (struct hash *);
uint32_t spt_hash_func (const struct hash_elem *, void *aux);
bool spt_less_func (const struct hash_elem *,
                             const struct hash_elem *,
                             void *aux);

/* SPT Table Managing Functions */
struct page_entry *spt_find_page (struct hash *, void *);
bool spt_insert_page (struct hash *, struct page_entry *);
bool spt_delete_page (struct hash *, struct page_entry *);
void spt_destroy (struct hash *);
void page_destructor (struct hash_elem *, void *aux);

/* Lazy Loading */
bool set_page_entry (struct file *, off_t, uint8_t *, void *, uint32_t, uint32_t, bool, enum page_type); 
#endif