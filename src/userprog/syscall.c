#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"

static void syscall_handler (struct intr_frame *);
static void syscall_get_argument (struct intr_frame *, const int, int *);

static void
syscall_get_argument (struct intr_frame *f, const int argc, int *argv) {
  int i;
  for (i = 0; i < argc; i++) {
    argv[i] = *((uint32_t *) f->esp + (i + 1));
  }
}

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f)
{
  int NUMBER = *(uint32_t *) f->esp;
  int args[3] = {0};
  printf ("system call!\n");
  hex_dump((uintptr_t) f->esp, f->esp, PHYS_BASE - f->esp, true);
  syscall_get_argument(f, 3, args);
  thread_exit ();
}
