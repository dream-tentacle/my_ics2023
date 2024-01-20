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
  int len = strlen(cmd);
  char *copy = (char *)malloc(sizeof(char) * (len + 1));
  strcpy(copy, cmd);
  copy[len] = '\0';
  int count = 0;
  for (int i = 0; i < len; i++) {
    if (copy[i] == ' ') {
      copy[i] = '\0';
      count++;
    }
  }
  char **argv = (char **)malloc(sizeof(char *) * (count + 1));
  argv[count] = NULL;
  count = 0;
  for (int i = 0; i < len; i++) {
    if (copy[i] == '\0') {
      argv[count] = &copy[i + 1];
      printf("arg[%d] = %s\n", count, argv[count]);
      count++;
    }
  }
  execvp(copy, argv);
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
