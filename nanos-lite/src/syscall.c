#include <common.h>
#include "syscall.h"

void sys_yield() { yield(); }
void sys_exit(int code) { halt(code); }
int sys_write(int fd, void *buf, size_t count) {
  if (fd == 1 || fd == 2) {
    char *str = (char *)buf;
    for (int i = 0; i < count; i++) {
      putch(str[i]);
    }
    return count;
  }
  return -1;
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  switch (a[0]) {
  case SYS_yield:
    sys_yield();
    c->GPRx = 0;
    printf("sys_yield, return 0\n");
    break;
  case SYS_exit:
    printf("sys_exit\n");
    sys_exit(c->GPR2);
    break;
  case SYS_write:
    c->GPRx = sys_write(c->GPR2, (void *)c->GPR3, 60);
    // printf("sys_write, return %d\n", c->GPRx);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
