#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int draw_x = 0, draw_y = 0;

struct timeval start;
uint32_t NDL_GetTicks() {
  struct timeval now;
  gettimeofday(&now, NULL);
  return (now.tv_usec - start.tv_usec) / 1000 +
         (now.tv_sec - start.tv_sec) * 1000;
}
int NDL_PollEvent(char *buf, int len) {
  int fd = open("/dev/events", O_RDONLY);
  int nread = read(fd, buf, len);
  buf[nread] = '\0';
  close(fd);
  return nread;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w;
    screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0)
        continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0)
        break;
    }
    close(fbctl);
  } else {
    int fd = open("/proc/dispinfo", O_RDONLY);
    char buf[64];
    int nread = read(fd, buf, 63);
    buf[nread] = '\0';
    sscanf(buf, "WIDTH: %d\nHEIGHT: %d", &screen_w, &screen_h);
    if (*w == 0 && *h == 0) {
      *w = screen_w;
      *h = screen_h;
    } else {
      draw_x = (screen_w - *w) / 2;
      draw_y = (screen_h - *h) / 2;
    }
    printf("--------------------\nNDL_OpenCanvas info:\n");
    printf("screen_w = %d, screen_h = %d\n", screen_w, screen_h);
    printf("draw_x = %d, draw_y = %d\n", draw_x, draw_y);
    printf("--------------------\n");
    close(fd);
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  int fd = open("/dev/fb", O_WRONLY);
  x += draw_x;
  y += draw_y;
  for (int j = 0; j < h; j++) {
    lseek(fd, ((y + j) * screen_w + x) * sizeof(uint32_t), SEEK_SET);
    write(fd, pixels, w * sizeof(uint32_t));
    pixels += w;
  }
  close(fd);
}

void NDL_OpenAudio(int freq, int channels, int samples) {}

void NDL_CloseAudio() {}

int NDL_PlayAudio(void *buf, int len) { return 0; }

int NDL_QueryAudio() { return 0; }

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  gettimeofday(&start, NULL);
  return 0;
}

void NDL_Quit() {}
