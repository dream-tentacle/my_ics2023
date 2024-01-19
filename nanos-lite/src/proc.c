#include <proc.h>
#include <loader.h>
#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() { current = &pcb_boot; }
void *new_page(size_t nr_page);
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
  int sp = (int)new_page(8);
  printf("sp: %p\n", sp);
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
  char *position = (char *)heap.end;
  sp -= 4 * (argc + 1 + envc + 1);
  for (int i = 0; i < argc; i++) {
    position -= strlen(argv[i]) + 1;
    *(char **)(sp + 4 * i) = position;
  }
  *(char **)(sp + 4 * argc) = 0;
  for (int i = 0; i < envc; i++) {
    position -= strlen(envp[i]) + 1;
    *(char **)(sp + 4 * (argc + 1 + i)) = position;
  }
  *(char **)(sp + 4 * (argc + 1 + envc)) = 0;
  sp -= 4;
  *(int *)sp = argc;
  pcb->cp->GPRx = sp;
}
PCB *add_pcb() {
  for (int i = 0; i < MAX_NR_PROC; i++) {
    if (pcb[i].cp == NULL) {
      return &pcb[i];
    }
  }
  return NULL;
}
void init_proc() {
  context_kload(&pcb[0], hello_fun, (void *)"kernel");
  char *argv[] = {"--skip", "123", NULL};
  char *envp[] = {"123111", NULL};
  context_uload(add_pcb(), "/bin/pal", argv, envp);
  switch_boot_pcb();
  // load program here
  // naive_uload(NULL, "/bin/pal");
}

Context *schedule(Context *prev) {
  current->cp = prev;
  int flag = -1;
  for (int i = 0; i < MAX_NR_PROC; i++) {
    if (current == &pcb[i]) {
      flag = i;
      break;
    }
  }
  if (flag == -1)
    current = &pcb[0];
  else {
    for (int i = flag; i < 2 * MAX_NR_PROC; i++) {
      if (pcb[(i + 1) % MAX_NR_PROC].cp != NULL) {
        current = &pcb[(i + 1) % MAX_NR_PROC];
        break;
      }
    }
  }
  return current->cp;
}
