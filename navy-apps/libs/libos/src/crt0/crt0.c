#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  printf("args = %p\n", args);
  int argc = *args;
  printf("argc = %d\n", argc);
  char **argv = (*args) + 4;
  char **envp = (*args) + 4 * argc + 8;
  environ = envp;
  for (int i = 0; i < argc; i++) {
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  for (int i = 0; envp[i] != NULL; i++) {
    printf("envp[%d] = %s\n", i, envp[i]);
  }
  exit(main(argc, argv, envp));
  assert(0);
}
