#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "filesys/file.h"
#include "vm/page.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

int process_open_file (struct file *);
struct file *process_get_file (int fd);
void process_close_file (int fd);

typedef int pid_t;
struct thread *process_get_child (pid_t);
void process_remove_child (struct thread *);
bool demand_page (struct page_entry *);

#endif /* userprog/process.h */
