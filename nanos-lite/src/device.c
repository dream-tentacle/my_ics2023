#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
#define MULTIPROGRAM_YIELD() yield()
#else
#define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) [AM_KEY_##key] = #key,

static const char *keyname[256]
    __attribute__((used)) = {[AM_KEY_NONE] = "NONE", AM_KEYS(NAME)};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for (int i = 0; i < len; i++) {
    putch(((char *)buf)[i]);
  }
  return len;
}
extern unsigned int switch_to, fg_pcb;
size_t events_read(char *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) {
    return 0;
  }
  if (ev.keydown) {
    if (strcmp(keyname[ev.keycode], "F1") == 0) {
      switch_to = 4;
      fg_pcb = 1;
      MULTIPROGRAM_YIELD();
    }
    if (strcmp(keyname[ev.keycode], "F2") == 0) {
      switch_to = 4;
      fg_pcb = 2;
      MULTIPROGRAM_YIELD();
    }
    if (strcmp(keyname[ev.keycode], "F3") == 0) {
      switch_to = 4;
      fg_pcb = 3;
      MULTIPROGRAM_YIELD();
    }
    return sprintf(buf, "kd %s\n", keyname[ev.keycode]);
  } else {
    return sprintf(buf, "ku %s\n", keyname[ev.keycode]);
  }
}

size_t dispinfo_read(char *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T gpu_config = io_read(AM_GPU_CONFIG);
  return sprintf(buf, "WIDTH: %d\nHEIGHT: %d\n", gpu_config.width,
                 gpu_config.height);
}
size_t fb_write(const void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T gpu_config = io_read(AM_GPU_CONFIG);
  int w = (offset / 4) % gpu_config.width;
  int h = (offset / 4) / gpu_config.width;
  void *tmp = (void *)buf;
  io_write(AM_GPU_FBDRAW, w, h, tmp, len / sizeof(uint32_t), 1, true);
  return len;
}
void *buf;
char buf_flag = 0;
extern void *new_page(size_t nr_page);
void gpu_init() {
  if (buf_flag == 0) {
    buf_flag = 1;
    buf = (void *)new_page(400 * 300 / 1024 + 1);
  }
  printf("buf = %p\n", buf);
  memset(buf, 0, 4 * 400 * 300);
  printf("buf = %p\n", buf);
  io_write(AM_GPU_FBDRAW, 0, 0, buf, 400, 300, true);
}
void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
