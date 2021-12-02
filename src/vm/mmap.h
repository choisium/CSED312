#ifndef VM_MMAP_H
#define VM_MMAP_H

#include <list.h>
#include "filesys/file.h"

/* Map region identifier. */
typedef int mapid_t;
#define MAP_FAILED ((mapid_t) -1)

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

#endif