#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

struct timeval tv, last;
struct timezone tz;
int main() {
  int a = 0;
  gettimeofday(&tv, &tz);
  last.tv_sec = tv.tv_sec;
  last.tv_usec = tv.tv_usec;
  while (1) {
    a++;
    int flag = 1;
    while (flag) {
      gettimeofday(&tv, &tz);
      if (tv.tv_sec - last.tv_sec >= 1) {
        last.tv_sec = tv.tv_sec;
        last.tv_usec = tv.tv_usec;
        printf("%d\n", a);
        flag = 0;
      }
    }
  }
  return 0;
}