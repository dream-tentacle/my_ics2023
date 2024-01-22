#include <am.h>
#include <klib.h>
#include <riscv/riscv.h>

static Context *(*user_handler)(Event, Context *) = NULL;
extern void __am_get_cur_as(Context *c);
extern void __am_switch(Context *c);
// 一个函数指针，指向一个函数，这个函数的返回值是Context*，参数是Event和Context*。
Context *__am_irq_handle(Context *c) {
  printf("-----\nenter handle\n");
  printf("c->sp = %p\n", c->gpr[2]);
  printf("c->np = %p\n", c->np);
  printf("c->mcause = %p\n", c->mcause);
  printf("c->GPR1 = %p\n", c->GPR1);
  int mscratch;
  asm volatile("csrr %0, mscratch" : "=r"(mscratch));
  printf("mscratch = %p\n", mscratch);
  __am_get_cur_as(c);
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
    case 0x80000007:
      ev.event = EVENT_IRQ_TIMER;
      break;
    default:
      printf("Unhandled GPR1 = %d\n", c->GPR1);
      ev.event = EVENT_ERROR;

      break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }
  __am_switch(c);
  printf("c->sp = %p\n", c->gpr[2]);
  printf("c->np = %p\n", c->np);
  int sp;
  asm volatile("mv %0, sp" : "=r"(sp));
  printf("now sp = %p\n", sp);
  printf("quit handle\n-----\n");
  if (c->np == 1)
    assert(0);
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
  Context *c = (Context *)(kstack.end - sizeof(Context));
  c->mepc = (uint32_t)entry;
  c->pdir = NULL;
  c->GPR2 = (uint32_t)arg;
  c->mstatus = 1 << 7;
  return c;
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
