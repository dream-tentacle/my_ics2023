#include <proc.h>
#include <loader.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() { current = &pcb_boot; }

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%c' for the %dth time!",
        (uintptr_t)arg, j);
    j++;
    yield();
  }
}
void context_kload(PCB *pcb, void *entry, void *arg) {
  pcb->cp = kcontext((Area){pcb->stack, pcb + 1}, entry, arg);
}
void init_proc() {
  context_kload(&pcb[0], hello_fun, (void *)'a');
  context_kload(&pcb[1], hello_fun, (void *)'c');
  switch_boot_pcb();
  // load program here
}

Context *schedule(Context *prev) {
  printf("schedule\n");
  current->cp = prev;
  for (int i = 0; i < MAX_NR_PROC; i++) {
    if (current == &pcb[i]) {
      current = &pcb[(i + 1) % MAX_NR_PROC];
      break;
    }
  }
  return current->cp;
}
