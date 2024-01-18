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
    Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", arg, j);
    j++;
    yield();
  }
}
void context_kload(PCB *pcb, void *entry, void *arg) {
  pcb->cp = kcontext((Area){pcb->stack, pcb + 1}, entry, arg);
}
void init_proc() {
  naive_uload(NULL, "/bin/nslider");
  context_kload(&pcb[0], hello_fun, (void *)"a");
  context_kload(&pcb[1], hello_fun, (void *)"b");
  context_kload(&pcb[2], hello_fun, (void *)"c");
  context_kload(&pcb[3], hello_fun, (void *)"d");
  switch_boot_pcb();
  // load program here
}

Context *schedule(Context *prev) {
  current->cp = prev;
  int flag = 0;
  for (int i = 0; i < MAX_NR_PROC; i++) {
    if (current == &pcb[i]) {
      current = &pcb[(i + 1) % MAX_NR_PROC];
      flag = 1;
      break;
    }
  }
  if (flag == 0)
    current = &pcb[0];
  return current->cp;
}
