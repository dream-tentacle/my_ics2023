#include <am.h>
#include <nemu.h>
#include <stdio.h>
#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t tmp = inl(KBD_ADDR);
  printf("%u", tmp);
  kbd->keydown = (tmp & KEYDOWN_MASK) != 0;
  kbd->keycode = tmp & ~KEYDOWN_MASK;
}
