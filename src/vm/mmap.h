#ifndef VM_MMAP_H
#define VM_MMAP_H

#include <list.h>
#include "filesys/file.h"
#include "lib/user/syscall.h"

/* Entry for mmap_file_list in thread */
struct mmap_file
{
    mapid_t mapid;          /* Mmapped file identifier. */
    struct file *file;      /* Mmapped file object */
    struct list_elem elem;  /* List elem for thread.mmap_file_list */
    struct list page_list;  /* Mapped page list */
};


#endif