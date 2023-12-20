#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {"NONE", _KEYS(keyname)};

int SDL_PushEvent(SDL_Event *ev) {
  printf("SDL_PushEvent() is not implemented!\n");
  return 0;
}

int SDL_PollEvent(SDL_Event *event) {
  int event_len;
  char buf[64];
  event_len = NDL_PollEvent(buf, sizeof(buf));
  if (buf[0] == 'k' && buf[1] == 'd') {
    event->type = SDL_KEYDOWN;
    char key[64];
    sscanf(buf, "kd %s\n", key);
    for (int i = 0; i < 83; i++) {
      if (strcmp(key, keyname[i]) == 0) {
        event->key.keysym.sym = i;
        break;
      }
    }
  } else if (buf[0] == 'k' && buf[1] == 'u') {
    event->type = SDL_KEYUP;
    char key[64];
    sscanf(buf, "ku %s\n", key);
    for (int i = 0; i < 83; i++) {
      if (strcmp(key, keyname[i]) == 0) {
        event->key.keysym.sym = i;
        break;
      }
    }
  } else {
    printf("SDL_PollEvent read: %s\n", buf);
  }
  return 1;
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
    char key[64];
    sscanf(buf, "kd %s\n", key);
    for (int i = 0; i < 83; i++) {
      if (strcmp(key, keyname[i]) == 0) {
        event->key.keysym.sym = i;
        break;
      }
    }
  }
  if (buf[0] == 'k' && buf[1] == 'u') {
    event->type = SDL_KEYUP;
    char key[64];
    sscanf(buf, "ku %s\n", key);
    for (int i = 0; i < 83; i++) {
      if (strcmp(key, keyname[i]) == 0) {
        event->key.keysym.sym = i;
        break;
      }
    }
  }
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
