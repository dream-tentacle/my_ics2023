#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

struct timeval tv, last;
int main() {
  int a = 0;
  gettimeofday(&tv, 0);
  last.tv_sec = tv.tv_sec;
  last.tv_usec = tv.tv_usec;
  while (1) {
    a++;
    while (1) {
      gettimeofday(&tv, 0);
      if (tv.tv_usec - last.tv_usec >= 500) {
        last.tv_sec = tv.tv_sec;
        last.tv_usec = tv.tv_usec;
        printf("%d\n", a);
      }
    }
  }
  return 0;
}