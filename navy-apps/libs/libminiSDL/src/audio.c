#include <NDL.h>
#include <SDL.h>

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
  panic("SDL_OpenAudio() is not implemented!\n");
  return 0;
}

void SDL_CloseAudio() { panic("SDL_CloseAudio() is not implemented!\n"); }

void SDL_PauseAudio(int pause_on) {
  panic("SDL_PauseAudio() is not implemented!\n");
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
  panic("SDL_MixAudio() is not implemented!\n");
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec,
                           uint8_t **audio_buf, uint32_t *audio_len) {
  panic("SDL_LoadWAV() is not implemented!\n");
  return NULL;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
  panic("SDL_FreeWAV() is not implemented!\n");
}

void SDL_LockAudio() { panic("SDL_LockAudio() is not implemented!\n"); }

void SDL_UnlockAudio() { panic("SDL_UnlockAudio() is not implemented!\n"); }
