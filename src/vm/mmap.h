#ifndef VM_MMAP_H
#define VM_MMAP_H

#include <list.h>
#include "filesys/file.h"

/* Entry for mmap_file_list in thread */
struct mmap_file
  {
    mapid_t mapid;          /* Mmapped file identifier. */
    struct file *file;      /* Mmapped file object */
    struct list_elem elem;  /* List elem for thread.mmap_file_list */
    struct list page_list;  /* Mapped page list */
  };

bool set_mmap_file (struct mmap_file *, struct file *, void *);
struct mmap_file *add_mmap_file (struct file *);
struct mmap_file *get_mmap_file (mapid_t);
void del_mmap_file (struct mmap_file *);
void mmap_file_list_destroy (void);

#endif