#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  int argc = *args;
  char **argv = *(args) + 4;
  char **envp = *(args) + 4 * argc + 8;
  environ = envp;
  printf("argc = %d\n", argc);
  exit(main(argc, argv, envp));
  assert(0);
}
