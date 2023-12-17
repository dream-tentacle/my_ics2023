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
#define STRACE
#ifdef STRACE
#define strace printf
#else
#define strace(...)
#endif
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  switch (a[0]) {
  case SYS_yield:
    sys_yield();
    c->GPRx = 0;
    strace("sys_yield, return 0\n");
    break;
  case SYS_exit:
    strace("sys_exit, return %d\n", c->GPR2);
    sys_exit(c->GPR2);
    break;
  case SYS_write:
    c->GPRx = sys_write(c->GPR2, (void *)c->GPR3, c->GPR4);
    strace("sys_write, return %d\n", c->GPRx);
    break;
  case SYS_read:
    c->GPRx = sys_read(c->GPR2, (void *)c->GPR3, c->GPR4);
    strace("sys_read, return %d\n", c->GPRx);
    break;
  case SYS_brk:
    sys_brk(c->GPR2);
    c->GPRx = 0; // 以后PA4动态申请内存时还会调整
    strace("sys_brk, return 0\n");
    break;
  case SYS_close:
    c->GPRx = 0;
    strace("sys_close, return 0\n");
    break;
  case SYS_lseek:
    c->GPRx = sys_lseek(c->GPR2, c->GPR3, c->GPR4);
    strace("sys_lseek, return %d\n", c->GPRx);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
