#include <NDL.h>
#include <SDL.h>

#define keyname(k) #k,

static const char *keyname[] = {"NONE", _KEYS(keyname)};

int SDL_PushEvent(SDL_Event *ev) {
  printf("SDL_PushEvent() is not implemented!\n");
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  printf("SDL_PollEvent() is not implemented!\n");
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  int event_len;
  char buf[64];
  event_len = NDL_PollEvent(buf, sizeof(buf));
  while (event_len == 0) {
    event_len = NDL_PollEvent(buf, sizeof(buf));
  }
  if (buf[0] == 'k' && buf[1] == 'd') {
    event->type = SDL_KEYDOWN;
    sscanf(buf, "kd %hhd", &event->key.keysym.sym);
  }
  if (buf[0] == 'k' && buf[1] == 'u') {
    event->type = SDL_KEYUP;
    sscanf(buf, "ku %hhd", &event->key.keysym.sym);
  }
  printf("event: %s\n", buf);
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  printf("SDL_PeepEvents() is not implemented!\n");
  return 0;
}

uint8_t *SDL_GetKeyState(int *numkeys) {
  printf("SDL_GetKeyState() is not implemented!\n");
  return NULL;
}
