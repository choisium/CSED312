#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "filesys/filesys.h"
#include "devices/input.h"
#include "devices/shutdown.h"
#include "vm/page.h"
#include "vm/mmap.h"

/* Auxiliary functions to implement syscall */
static void syscall_handler (struct intr_frame *);
static void syscall_get_argument (struct intr_frame *, const int, int *);
static bool check_address_validity(const void *);
static bool check_mmap_validity(int fd, void *addr);

/* Syscall handlers for each system call numbers */
static void halt (void);
static pid_t exec (const char *);
static int wait (pid_t);
static int read (int, void *, unsigned);
static int write (int, const void *, unsigned);
static bool create (const char *, unsigned);
static bool remove (const char *);
static int open (const char *);
static void close (int);
static int filesize (int);
static void seek (int, unsigned);
static unsigned tell (int);
static mapid_t mmap (int fd, void *addr);
static void munmap (mapid_t);

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
  #ifdef VM
  if (vaddr != NULL && is_user_vaddr(vaddr) && is_user_vaddr(vaddr + 4))
    return true;
  return false;
  #endif

  struct thread *t = thread_current();

  if (vaddr != NULL && is_user_vaddr(vaddr) && is_user_vaddr(vaddr + 4)  // check both start and end
      && pagedir_get_page(t->pagedir, vaddr) != NULL)
    return true;

  return false;
}

void
syscall_init (void) 
{
  lock_init (&file_system_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f)
{
  int number;   // syscall number
  int args[3];  // array to store arguments
  bool valid;   // check address validity

  valid = check_address_validity(f->esp);
  if (!valid) exit(-1);

  number = *(uint32_t *) f->esp;

  #ifdef VM
  /* The processor only saves the stack pointer when an
      exception causes a switch from user to kernel mode.
      Therefore we need to save esp of intr_frame. */
  thread_current()->esp = f->esp;
  #endif

  switch (number) {
    case SYS_HALT:
      halt ();
      break;

    case SYS_EXIT:
      syscall_get_argument(f, 1, args);
      exit(args[0]);
      break;

    case SYS_EXEC:
      syscall_get_argument(f, 1, args);
      valid = check_address_validity((void *) args[0]);
      if (!valid) exit(-1);

      f->eax = exec((void *) args[0]);
      break;
    
    case SYS_WAIT:
      syscall_get_argument(f, 1, args);
      f->eax = wait(args[0]);
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
      syscall_get_argument(f, 3, args);
      valid = check_address_validity((void *) args[1]);
      if (!valid) exit(-1);

      f->eax = read(args[0], (void *) args[1], args[2]);
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

    case SYS_MMAP:
      syscall_get_argument(f, 2, args);
      valid = check_address_validity((void *) args[1]);
      if (!valid) exit(-1);

      f->eax = mmap(args[0], (void *) args[1]);
      break;

    case SYS_MUNMAP:
      syscall_get_argument(f, 1, args);
      munmap(args[0]);
      break;
  }
}

static void
halt (void) 
{
  shutdown_power_off ();
  NOT_REACHED ();
}

void
exit (int status)
{
  struct thread *t = thread_current();
  printf("%s: exit(%d)\n", t->name, status);
  t->exit_status = status;
  t->terminated_by_exit = true;
  thread_exit();
}

static pid_t
exec (const char *cmd_line)
{
  pid_t pid;
  struct thread *child;

  pid = process_execute(cmd_line);
  if (pid == TID_ERROR)
    return -1;

  /* Search child process by pid, and wait for load. */
  child = process_get_child(pid);
  sema_down(&child->load_sema);

  /* If the program cannot run, return -1. 
     Otherwise, return pid. */
  if (!child->load_success)
    return -1;
  else
    return pid;
}

static int 
wait (pid_t pid)
{
  int status;
  status = process_wait (pid);
  return status;
}

static int
read (int fd, void *buffer, unsigned size)
{
  unsigned i;
  int ret;
  char c = 1;

  if (fd == 0)    // stdin
  {
    for (i = 0; i < size && c != '\0'; i++) {
      c = input_getc();
      ((char*) buffer)[i] = c;
    }
    return i;
  }
  else if (fd >= 2)
  {
    struct file *file_object = process_get_file(fd);
    if (file_object == NULL) exit(-1);

    lock_acquire(&file_system_lock);
    ret = file_read(file_object, buffer, size);
    lock_release(&file_system_lock);

    return ret;
  }
  return -1;
}

static int
write (int fd, const void *buffer, unsigned size)
{
  int ret;

  if (fd == 1)    // stdout
  {
    putbuf(buffer, size);
    return size;
  }
  else if (fd >= 2)
  {
    struct file *file_object = process_get_file(fd);
    if (file_object == NULL) exit(-1);

    lock_acquire(&file_system_lock);
    ret = file_write(file_object, buffer, size);
    lock_release(&file_system_lock);

    return ret;
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

static bool
check_mmap_validity(int fd, void *addr)
{
  off_t size, i;
  struct file *f = process_get_file(fd);

  if (f == NULL)
      return false;

  size = file_length(f);
  if (size == 0)
    return false;

  if (!check_address_validity(addr))
    return false;

  if (addr == 0 || addr != pg_round_down(addr))
    return false;

  for (i = 0; i < size; i += PGSIZE)
  {
    if (spt_find_page(&thread_current()->spt, addr + i))
      return false;
  }

  return true;
}

static mapid_t
mmap (int fd, void *addr)
{
  if (!check_mmap_validity(fd, addr))
    return -1;
  struct file *file = process_get_file(fd);
  struct mmap_file *mmap_file = add_mmap_file(file);
  if (!mmap_file) return -1;
  if (!set_mmap_file(mmap_file, file, addr)) return -1;
  return mmap_file->mapid;
}

static void
munmap (mapid_t mapid)
{
  struct mmap_file *mf = get_mmap_file(mapid);
  if (mf != NULL)
    del_mmap_file(mf);
}