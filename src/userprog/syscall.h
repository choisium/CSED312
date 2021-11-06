#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "threads/synch.h"

struct lock file_system_lock;

void syscall_init (void);

void exit (int);

#endif /* userprog/syscall.h */
