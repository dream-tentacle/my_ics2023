#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  printf("args = %p\n", args);
  // int argc = *args;
  // char **argv = *(args) + 4;
  // char **envp = *(args) + 4 * argc + 8;
  // environ = envp;
  exit(main(0, 0, 0));
  assert(0);
}
