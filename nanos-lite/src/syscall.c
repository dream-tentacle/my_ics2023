#include <common.h>
#include "syscall.h"

void sys_yield() { yield(); }

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
  case SYS_yield:
    sys_yield();
    for (int i = 0; i < 31; i++) {
      c->gpr[i] = 66;
    }
    break;
  case 3:
    printf("3\n");
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
