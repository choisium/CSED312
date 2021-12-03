#include <debug.h>
#include <hash.h>
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"
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

struct mmap_file *
get_mmap_file (mapid_t mapid)
{
    struct thread * t= thread_current();
    struct list_elem *e;

    for (e = list_begin (&t->mmap_file_list); e != list_end (&t->mmap_file_list);
        e = list_next (e))
      {
          struct mmap_file *mf = list_entry (e, struct mmap_file, elem);
          if (mf->mapid == mapid)
            return mf;
      }

    return NULL;
}

void
del_mmap_file (struct mmap_file *mf)
{
    struct thread *t = thread_current();
    struct list_elem *e;
    struct page_entry *pe;

    /* Remove page_entry from spt of thread and page_list of mmap_file */
    for (e = list_begin (&mf->page_list); e != list_end (&mf->page_list); )
      {
        pe = list_entry (e, struct page_entry, mmap_elem);
        spt_delete_page(&t->spt, pe);
        list_remove(e);

        if (pe->writable && pagedir_is_dirty(thread_current()->pagedir, pe->vaddr)) {
            file_write_at(pe->file, pe->vaddr, pe->read_bytes, pe->ofs);
        }

        e = list_next(e);
        free(pe);
      }

    list_remove(&mf->elem);
    file_close(mf->file);
    free (mf);
}

void
mmap_file_list_destroy (void)
{
    struct thread *t = thread_current();
    struct list_elem *e;
    struct mmap_file *mf;

    /* Remove page_entry from spt of thread and page_list of mmap_file */
    for (e = list_begin (&t->mmap_file_list); e != list_end (&t->mmap_file_list); )
    {
        mf = list_entry (e, struct mmap_file, elem);
        list_remove(e);
        e = list_next(e);
        del_mmap_file(mf);
    }
}