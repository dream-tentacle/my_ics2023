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
void context_uload(PCB *pcb, const char *filename, char *const argv[],
                   char *const envp[]) {
  uintptr_t entry = loader(pcb, filename);
  Log("User Context set entry = %p", entry);
  pcb->cp = ucontext(NULL, (Area){pcb->stack, pcb + 1}, (void *)entry);
  int sp = (int)heap.end;
  int argc = 0, envc = 0;
  while (argv[argc] != NULL) {
    sp -= strlen(argv[argc]) + 1;
    strcpy((char *)sp, argv[argc]);
    argc++;
  }
  while (envp[envc] != NULL) {
    sp -= strlen(envp[envc]) + 1;
    strcpy((char *)sp, envp[envc]);
    envc++;
  }
  for (int i = envc; i >= 0; i--) {
    sp -= 4;
    *(char *)sp = (int)envp[i];
  }
  for (int i = argc; i >= 0; i--) {
    sp -= 4;
    *(char *)sp = (int)argv[i];
  }
  sp -= 4;
  *(int *)sp = argc;
  printf("sp = %p\n", sp);
  pcb->cp->GPRx = sp;
}
void init_proc() {
  context_kload(&pcb[0], hello_fun, (void *)"a");
  // context_kload(&pcb[1], hello_fun, (void *)"b");
  context_kload(&pcb[2], hello_fun, (void *)"c");
  context_kload(&pcb[3], hello_fun, (void *)"d");
  char *argv[] = {"--skip", "1234567890", NULL};
  char *envp[] = {"12345", NULL};
  context_uload(&pcb[1], "/bin/bird", argv, envp);
  // context_uload(&pcb[2], "/bin/bird");   This will cause failure, why? The
  // answer is in the next chapter
  switch_boot_pcb();
  // load program here
  // naive_uload(NULL, "/bin/pal");
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
