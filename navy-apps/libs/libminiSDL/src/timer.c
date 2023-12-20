#include <NDL.h>
#include <sdl-timer.h>
#include <stdio.h>

SDL_TimerID SDL_AddTimer(uint32_t interval, SDL_NewTimerCallback callback,
                         void *param) {
  panic("SDL_AddTimer() is not implemented!\n");
  return NULL;
}

int SDL_RemoveTimer(SDL_TimerID id) {
  panic("SDL_RemoveTimer() is not implemented!\n");
  return 1;
}

uint32_t SDL_GetTicks() {
  panic("SDL_GetTicks() is not implemented!\n");
  return 0;
}

void SDL_Delay(uint32_t ms) { panic("SDL_Delay() is not implemented!\n"); }
