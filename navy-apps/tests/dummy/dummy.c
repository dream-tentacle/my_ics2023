#include <stdint.h>

#ifdef __ISA_NATIVE__
#error can not support ISA=native
#endif

#define SYS_yield 1
extern int _syscall_(int, uintptr_t, uintptr_t, uintptr_t);

int main() { return _syscall_(SYS_yield, 0, 0, 0); }
// 这里调用了系统调用号为1的系统调用，c->GPR1=1，
// abstract-machine/am/src/riscv/nemu/cte.c中会根据1将事件设为EVENT_SYSCALL
// 然后会在nanos-lite/src/irq.c中触发case EVENT_SYSCALL
// 然后会调用do_syscall(c)，这里的c是一个Context*，然后会根据c->GPR1来判断是哪个系统调用
// c->GPR1为1，于是调用sys_yield()，然后c->GPRx=0，这里的c->GPRx是返回值，也就是说，这里的返回值是0
// return 0会调用系统调用号为0的系统调用，c->GPR1=0，
// abstract-machine/am/src/riscv/nemu/cte.c中会根据0将事件设为EVENT_SYSCALL
// 然后会在nanos-lite/src/irq.c中触发case EVENT_SYSCALL
// 然后会调用do_syscall(c)，这里的c是一个Context*，然后会根据c->GPR1来判断是哪个系统调用
// c->GPR1为0，于是调用halt(0)