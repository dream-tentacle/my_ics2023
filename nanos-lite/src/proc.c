#include <proc.h>
#include <loader.h>
#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

unsigned int switch_to = 0;
unsigned int fg_pcb = 1;
void switch_boot_pcb() { current = &pcb_boot; }
void *new_page(size_t nr_page);
void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    // if (j % 10000 == 0)
    Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", arg, j);
    j++;
    switch_to = fg_pcb;
    yield();
  }
}
void context_kload(PCB *pcb, void *entry, void *arg) {
  pcb->cp = kcontext((Area){pcb->stack, pcb + 1}, entry, arg);
  printf("pcb->cp = %p\n", pcb->cp);
}
void context_uload(PCB *pcb, const char *filename, char *const argv[],
                   char *const envp[]) {
  void *newpg = new_page(8) + 32 * 1024; // 用户栈物理地址
  for (int i = 1; i <= 8; i++) {
    map(&pcb->as, pcb->as.area.end - i * 4 * 1024, newpg - i * 4 * 1024,
        0); // 用户栈虚拟地址
  }
  memset(newpg - 32 * 1024, 0, 32 * 1024);
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
  printf("User as->ptr = %p\n", pcb->as.ptr);
  pcb->cp->GPRx = (int)sp + pcb->as.area.end - newpg;
  pcb->cp->gpr[2] = pcb->cp->GPRx; // c->sp 设为用户栈顶（虚拟地址）
  pcb->cp->np = 1;                 // c->np 设为1，表示用户态退出
  // pcb->cp->GPRx = (int)sp;
}
PCB *add_pcb() { return current; }
void init_proc() {
  context_kload(&pcb[0], hello_fun, (void *)"kernel1");
  char *argv[] = {NULL};
  char *envp[] = {NULL};
  protect(&pcb[1].as);
  context_uload(&pcb[1], "/bin/pal", argv, envp);
  protect(&pcb[2].as);
  context_uload(&pcb[2], "/bin/bird", argv, envp);
  protect(&pcb[3].as);
  context_uload(&pcb[3], "/bin/nterm", argv, envp);
  switch_boot_pcb();
  yield();
  // load program here
  // naive_uload(NULL, "/bin/pal");
}
Context *schedule(Context *prev) {
  if (current)
    current->cp = prev;
#ifdef TIME_SHARING
  for (int i = 0; i < MAX_NR_PROC; i++) {
    if (current == &pcb[i]) {
      switch_to = (i + 1) % MAX_NR_PROC;
      break;
    }
  }
#endif
  current = &pcb[switch_to];
  return current->cp;
}
