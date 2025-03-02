#include <am.h>
#include <klib.h>
#include <riscv/riscv.h>

static Context *(*user_handler)(Event, Context *) = NULL;
// 一个函数指针，指向一个函数，这个函数的返回值是Context*，参数是Event和Context*。

Context *__am_irq_handle(Context *c) {
  // printf("mcause: %x\n", c->mcause);
  // printf("mstatus: %x\n", c->mstatus);
  // printf("mepc: %x\n", c->mepc);
  // for (int i = 0; i < 32; i++) {
  //   printf("gpr[%d]: %x\n", i, c->gpr[i]);
  // }
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
    case 0xb:
      if (c->GPR1 == -1) {
        ev.event = EVENT_YIELD;
        break;
      } else if (c->GPR1 == 1 || c->GPR1 == 0 || c->GPR1 == 4 || c->GPR1 == 9 ||
                 c->GPR1 == 3 || c->GPR1 == 7 || c->GPR1 == 2 || c->GPR1 == 8 ||
                 c->GPR1 == 19 || c->GPR1 == 13) {
        ev.event = EVENT_SYSCALL;
        break;
      }
    default:
      printf("Unhandled GPR1 = %d\n", c->GPR1);
      ev.event = EVENT_ERROR;

      break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context *(*handler)(Event, Context *)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() { return false; }

void iset(bool enable) {}
