#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() { sh_printf("sh> "); }
const char *PATH[10];
static void sh_handle_cmd(const char *cmd) {
  // 在空格处分开，第一个是filepath，后面是argv，最终调用execvp
  char *argv[10];
  int argc = 0;
  char *p = strtok((char *)cmd, " ");
  while (p) {
    char *arg = new char[strlen(p) + 1];
    strcpy(arg, p);
    argv[argc++] = arg;
    p = strtok(NULL, " ");
  }
  argv[argc] = NULL;
  if (argc == 0)
    return;
  // 打印各个参数
  for (int i = 0; i < argc; i++) {
    printf("argv[%d]=%p %s\n", i, argv[i], argv[i]);
  }
  execvp(argv[0], argv);
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();
  setenv("PATH", "/bin", 0);
  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        char *res = (char *)term->keypress(handle_key(&ev));
        if (res) {
          if (res[strlen(res) - 1] == '\n')
            res[strlen(res) - 1] = '\0';
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
