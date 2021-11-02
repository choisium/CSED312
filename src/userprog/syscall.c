#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"

static void syscall_handler (struct intr_frame *);
static void syscall_get_argument (struct intr_frame *, const int, int *);
static void exit (int);

/* Get arguments from interrupt frame and store it in argv */
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
  int number = *(uint32_t *) f->esp;  // syscall number
  int args[3];  // array to store arguments
  // hex_dump((uintptr_t) f->esp, f->esp, PHYS_BASE - f->esp, true);

  switch (number) {
    case SYS_HALT:
      printf("SYS_HALT\n");
      break;
    case SYS_EXIT:
      printf("SYS_EXIT\n");
      syscall_get_argument(f, 1, args);
      exit(args[0]);
      break;
    case SYS_WAIT:
      printf("SYS_WAIT\n");
      break;
    case SYS_CREATE:
      printf("SYS_CREATE\n");
      break;
    case SYS_REMOVE:
      printf("SYS_REMOVE\n");
      break;
    case SYS_OPEN:
      printf("SYS_OPEN\n");
      break;
    case SYS_FILESIZE:
      printf("SYS_FILESIZE\n");
      break;
    case SYS_READ:
      printf("SYS_READ\n");
      break;
    case SYS_WRITE:
      printf("SYS_WRITE\n");
      break;
    case SYS_SEEK:
      printf("SYS_SEEK\n");
      break;
    case SYS_TELL:
      printf("SYS_TELL\n");
      break;
    case SYS_CLOSE:
      printf("SYS_CLOSE\n");
      break;
  }

  thread_exit ();
}

static void
exit (int status)
{
  struct thread *t = thread_current();
  printf("%s: exit(%d)\n", t->name, status);
  thread_exit();
}
