#include <debug.h>
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "userprog/process.h"
#include "filesys/file.h"
#include "vm/page.h"
#include "vm/mmap.h"


struct mmap_file *
add_mmap_file (struct file *file)
{
    ASSERT (file != NULL);

    /* Create new mmap_file for fd */
    struct mmap_file *mmap_file = malloc(sizeof(struct mmap_file));
    if (mmap_file == NULL)
        return NULL;

    struct thread *t = thread_current();

    /* Initialize mmap_file */
    mmap_file->mapid = t->max_mapid++;
    mmap_file->file = file_reopen(file);
    list_init(&mmap_file->page_list);

    /* Add mmap_file to thread's mmap_file_list */
    list_push_back(&t->mmap_file_list, &mmap_file->elem);

    return mmap_file;
}

bool
set_mmap_file (struct mmap_file *mmap_file, struct file *file, void *addr)
{
    ASSERT (mmap_file != NULL && file != NULL);

    off_t size = file_length(file);
    off_t ofs = 0;
    void *upage = addr;
    uint32_t read_bytes, zero_bytes;

    while (size > 0)
    {
        read_bytes = size < PGSIZE ? size : PGSIZE;
        zero_bytes = PGSIZE - read_bytes;

        if(!set_page_entry(mmap_file->file, ofs, upage, NULL,
                        read_bytes, zero_bytes, true, PG_MMAP))
            return false;

        struct page_entry *pe = spt_find_page(&thread_current()->spt, upage);
        list_push_back(&mmap_file->page_list, &pe->mmap_elem);

        size -= read_bytes;
        ofs += read_bytes;
        upage += PGSIZE;
    }

    return true;
}