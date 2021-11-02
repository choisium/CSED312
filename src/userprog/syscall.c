#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "filesys/filesys.h"

/* Auxiliary functions to implement syscall */
static void syscall_handler (struct intr_frame *);
static void syscall_get_argument (struct intr_frame *, const int, int *);
static bool check_address_validity(const void *);

/* Syscall handlers for each system call numbers */
static void exit (int);
static int write (int, const void *, unsigned);
static bool create (const char *, unsigned);
static bool remove (const char *);
static int open (const char *);
static void close (int);
static int filesize (int);
static void seek (int, unsigned);
static unsigned tell (int);

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
      syscall_get_argument(f, 1, args);
      valid = check_address_validity((void *) args[0]);
      if (!valid) exit(-1);

      f->eax = remove((void *) args[0]);
      break;

    case SYS_OPEN:
      syscall_get_argument(f, 1, args);
      valid = check_address_validity((void *) args[0]);
      if (!valid) exit(-1);

      f->eax = open((char *) args[0]);
      break;

    case SYS_FILESIZE:
      syscall_get_argument(f, 1, args);
      f->eax = filesize(args[0]);
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
      syscall_get_argument(f, 2, args);
      seek(args[0], args[1]);
      break;

    case SYS_TELL:
      syscall_get_argument(f, 1, args);
      f->eax = tell(args[0]);
      break;

    case SYS_CLOSE:
      syscall_get_argument(f, 1, args);
      close(args[0]);
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

static bool
remove (const char *file)
{
  return filesys_remove(file);
}

static int
open (const char *file)
{
  struct file *file_object = filesys_open(file);
  if (file_object == NULL) return -1;
  return process_open_file(file_object);
}

static void
close (int fd)
{
  process_close_file(fd);
}

static int
filesize (int fd)
{
  struct file *file_object = process_get_file(fd);
  if (file_object == NULL) exit(-1);
  return file_length(file_object);
}

static void
seek (int fd, unsigned position)
{
  struct file *file_object = process_get_file(fd);
  if (file_object == NULL) exit(-1);
  file_seek(file_object, position);
}

static unsigned
tell (int fd)
{
  struct file *file_object = process_get_file(fd);
  if (file_object == NULL) exit(-1);
  return file_tell(file_object);
}