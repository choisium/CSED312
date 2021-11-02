#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "filesys/filesys.h"

/* Auxiliary functions to implement syscall */
static void syscall_handler (struct intr_frame *);
static void syscall_get_argument (struct intr_frame *, const int, int *);
static bool check_address_validity(const void *);

/* Syscall handlers for each system call numbers */
static void exit (int);
static int write (int, const void *, unsigned);
static bool create (const char *file, unsigned initial_size);


/* Get arguments from interrupt frame and store it in argv */
static void
syscall_get_argument (struct intr_frame *f, const int argc, int *argv) {
  int i;
  for (i = 0; i < argc; i++) {
    argv[i] = *((uint32_t *) f->esp + (i + 1));
  }
}

/* Check address given from user is valid or not */
static bool
check_address_validity (const void *vaddr) {
  struct thread *t = thread_current();

  if (vaddr != NULL && is_user_vaddr(vaddr)
      && pagedir_get_page(t->pagedir, vaddr) != NULL)
    return true;

  return false;
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
  bool valid;   // check address validity
  // hex_dump((uintptr_t) f->esp, f->esp, PHYS_BASE - f->esp, true);

  switch (number) {
    case SYS_HALT:
      printf("SYS_HALT\n");
      break;
    case SYS_EXIT:
      syscall_get_argument(f, 1, args);
      exit(args[0]);
      break;
    case SYS_WAIT:
      printf("SYS_WAIT\n");
      break;
    case SYS_CREATE:
      syscall_get_argument(f, 2, args);
      valid = check_address_validity((void *) args[0]);
      if (!valid) exit(-1);

      f->eax = create((void *) args[0], args[1]);
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
      syscall_get_argument(f, 3, args);
      valid = check_address_validity((void *) args[1]);
      if (!valid) exit(-1);

      f->eax = write(args[0], (void *) args[1], args[2]);
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
}

static void
exit (int status)
{
  struct thread *t = thread_current();
  printf("%s: exit(%d)\n", t->name, status);
  thread_exit();
}

static int
write (int fd, const void *buffer, unsigned size)
{
  if (fd == 1) {
    putbuf(buffer, size);
    return size;
  }
  return -1;
}

static bool
create (const char *file, unsigned initial_size)
{
  return filesys_create(file, initial_size);
}