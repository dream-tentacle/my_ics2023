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
    if (j % 30000 == 0)
      Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", arg,
          j);
    j++;
    yield();
  }
}
void context_kload(PCB *pcb, void *entry, void *arg) {
  pcb->cp = kcontext((Area){pcb->stack, pcb + 1}, entry, arg);
}
void context_uload(PCB *pcb, const char *filename, char *const argv[],
                   char *const envp[]) {
  void *newpg = new_page(8) + 32 * 1024; // 用户栈物理地址
  for (int i = 1; i <= 8; i++) {
    map(&pcb->as, pcb->as.area.end - i * 4 * 1024, newpg - i * 4 * 1024,
        0); // 用户栈虚拟地址
  }
  Log("new page area: p %p,%p", newpg - 32 * 1024, newpg);
  Log("new page area: v %p,%p", pcb->as.area.end - 32 * 1024, pcb->as.area.end);
  void *sp = newpg;
  int argc = 0, envc = 0;
  if (argv != NULL)
    while (argv[argc] != NULL) {
      sp -= strlen(argv[argc]) + 1;
      strcpy((char *)sp, argv[argc]);
      argc++;
    }
  if (envp != NULL)
    while (envp[envc] != NULL) {
      sp -= strlen(envp[envc]) + 1;
      strcpy((char *)sp, envp[envc]);
      envc++;
    }
  char *position = (char *)newpg;
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
  uintptr_t entry = loader(pcb, filename);
  Log("User Context set entry = %p", entry);
  pcb->cp = ucontext(&pcb->as, (Area){pcb->stack, pcb + 1}, (void *)entry);
  printf("the new user pdir = %p\n", pcb->as.ptr);
  pcb->cp->GPRx = (int)sp + pcb->as.area.end - newpg;
  // pcb->cp->GPRx = (int)sp;
}
PCB *add_pcb() {
  return &pcb[0];
  // for (int i = 0; i < MAX_NR_PROC; i++) {
  //   if (pcb[i].cp == NULL) {
  //     return &pcb[i];
  //   }
  // }
  // return NULL;
}
void init_proc() {
  char *argv[] = {NULL};
  char *envp[] = {NULL};
  protect(&pcb[0].as);
  context_uload(&pcb[0], "/bin/pal", argv, envp);
  context_kload(&pcb[1], hello_fun, (void *)"kernel");
  switch_boot_pcb();
  yield();
  // load program here
  // naive_uload(NULL, "/bin/pal");
}

Context *schedule(Context *prev) {
  current->cp = prev;
  // int flag = -1;
  // for (int i = 0; i < MAX_NR_PROC; i++) {
  //   if (current == &pcb[i]) {
  //     flag = i;
  //     break;
  //   }
  // }
  // if (flag == -1)
  //   current = &pcb[0];
  // else {
  //   for (int i = flag; i < 2 * MAX_NR_PROC; i++) {
  //     if (pcb[(i + 1) % MAX_NR_PROC].cp != NULL) {
  //       current = &pcb[(i + 1) % MAX_NR_PROC];
  //       break;
  //     }
  //   }
  // }
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  // current = &pcb[0];
  return current->cp;
}
