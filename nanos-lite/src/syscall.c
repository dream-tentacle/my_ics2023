#include <common.h>
#include "syscall.h"
#include "fs.h"
#include "ramdisk.h"
void sys_yield() { yield(); }
void sys_exit(int code) { halt(code); }
int sys_open(const char *path, int flags, int mode) {
  return fs_open(path, flags, mode);
}
int sys_write(int fd, void *buf, size_t count) {
  return fs_write(fd, buf, count);
}
int sys_read(int fd, void *buf, size_t count) {
  return fs_read(fd, buf, count);
}
int sys_lseek(int fd, size_t offset, int whence) {
  return fs_lseek(fd, offset, whence);
}
void sys_brk(int addr) {}
void sys_gettimeofday(int *tv, int *tz) {
  if (tv != NULL) {
    tv[0] = io_read(AM_TIMER_UPTIME).us / 1000000;
    tv[1] = io_read(AM_TIMER_UPTIME).us;
  }
  if (tz != NULL) {
    tz[0] = 0;
    tz[1] = 0;
  }
}

#define STRACE
#ifdef STRACE
#define strace(s, ...) printf("> " s " <\n", ##__VA_ARGS__)
#else
#define strace(...)
#endif
extern Finfo file_table[];
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  int tmp;
  switch (a[0]) {
  case SYS_yield:
    sys_yield();
    c->GPRx = 0;
    strace("sys_yield, return 0");
    break;
  case SYS_exit:
    strace("sys_exit, return %d", c->GPR2);
    sys_exit(c->GPR2);
    break;
  case SYS_write:
    tmp = c->GPR2;
    c->GPRx = sys_write(c->GPR2, (void *)c->GPR3, c->GPR4);
    strace("sys_write, write %s, return %d", file_table[tmp].name, c->GPRx);
    break;
  case SYS_read:
    tmp = c->GPR2;
    c->GPRx = sys_read(c->GPR2, (void *)c->GPR3, c->GPR4);
    strace("sys_read, read from %s, return %d", file_table[tmp].name, c->GPRx);
    break;
  case SYS_brk:
    sys_brk(c->GPR2);
    c->GPRx = 0; // 以后PA4动态申请内存时还会调整
    strace("sys_brk, return 0");
    break;
  case SYS_close:
    strace("sys_close, close %s, return 0", file_table[c->GPR2].name);
    c->GPRx = 0;
    break;
  case SYS_lseek:
    tmp = c->GPR2;
    c->GPRx = sys_lseek(c->GPR2, c->GPR3, c->GPR4);
    strace("sys_lseek, lseek %s, offset set to %d, return %d",
           file_table[tmp].name, file_table[tmp].open_offset, c->GPRx);
    break;
  case SYS_open:
    char *path = (char *)c->GPR2;
    c->GPRx = sys_open((const char *)c->GPR2, c->GPR3, c->GPR4);
    strace("sys_open, open file %s, return %d", path, c->GPRx);
    break;
  case SYS_gettimeofday:
    sys_gettimeofday((int *)c->GPR2, (int *)c->GPR3);
    c->GPRx = 0;
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
