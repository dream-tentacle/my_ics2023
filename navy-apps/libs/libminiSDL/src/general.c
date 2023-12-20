#include <NDL.h>

int SDL_Init(uint32_t flags) {
  panic("SDL_Init() is not implemented!\n");
  return NDL_Init(flags);
}

void SDL_Quit() { NDL_Quit(); }

char *SDL_GetError() { return "Navy does not support SDL_GetError()"; }

int SDL_SetError(const char *fmt, ...) {
  panic("SDL_SetError() is not implemented!\n");
  return -1;
}

int SDL_ShowCursor(int toggle) {
  panic("SDL_ShowCursor() is not implemented!\n");
  return 0;
}

void SDL_WM_SetCaption(const char *title, const char *icon) {
  panic("SDL_WM_SetCaption() is not implemented!\n");
}
