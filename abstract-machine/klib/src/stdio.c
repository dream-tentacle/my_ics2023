#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define DO_NEXT(x)                                                             \
  do {                                                                         \
    putch(x);                                                                  \
    re++;                                                                      \
  } while (0)

#define DO_NEXT2(x)                                                            \
  do {                                                                         \
    re++;                                                                      \
    out[re] = x;                                                               \
  } while (0)

int printf(const char *fmt, ...) {

  int re = 0;
  char c[10000];
  int cnt = 0;
  va_list ap;
  va_start(ap, fmt);
  while (fmt[cnt] != '\0') {
    if (fmt[cnt] == '%' && fmt[cnt + 1] == 'd') {
      int tmp = va_arg(ap, int);
      int offset = 0;
      if (tmp == 0) {
        DO_NEXT('0');
        cnt += 2;
        continue;
      }
      if (tmp < 0) {
        tmp = -tmp;
        DO_NEXT('-');
      }
      while (tmp != 0) {
        c[offset] = (char)(tmp % 10 + '0');
        tmp /= 10;
        offset++;
      }
      for (int i = offset - 1; i >= 0; i--) {
        DO_NEXT(c[i]);
      }
      cnt += 2;
    } else if (fmt[cnt] == '%' && fmt[cnt + 1] == 'x') {
      int tmp = va_arg(ap, int);
      int offset = 0;
      // DO_NEXT('0');
      // DO_NEXT('x');
      if (tmp == 0) {
        DO_NEXT('0');
        cnt += 2;
        continue;
      }
      if (tmp < 0) {
        tmp = -tmp;
        DO_NEXT('-');
      }
      while (tmp != 0) {
        c[offset] = (char)(tmp % 16 + '0');
        if (c[offset] > '9') {
          c[offset] = (char)(c[offset] - '9' - 1 + 'a');
        }
        tmp /= 16;
        offset++;
      }
      for (int i = offset - 1; i >= 0; i--) {
        DO_NEXT(c[i]);
      }
      cnt += 2;
    } else if (fmt[cnt] == '%' && fmt[cnt + 1] == 'p') {
      unsigned int tmp = va_arg(ap, int);
      int offset = 0;
      // DO_NEXT('0');
      // DO_NEXT('x');
      if (tmp == 0) {
        DO_NEXT('0');
        cnt += 2;
        continue;
      }
      while (tmp != 0) {
        c[offset] = (char)(tmp % 16 + '0');
        if (c[offset] > '9') {
          c[offset] = (char)(c[offset] - '9' - 1 + 'a');
        }
        tmp /= 16;
        offset++;
      }
      for (int i = offset - 1; i >= 0; i--) {
        DO_NEXT(c[i]);
      }
      cnt += 2;
    } else if (fmt[cnt] == '%' && fmt[cnt + 1] == 'u') {
      unsigned tmp = va_arg(ap, unsigned int);
      int offset = 0;
      if (tmp == 0) {
        DO_NEXT('0');
        cnt += 2;
        continue;
      }
      while (tmp != 0) {
        c[offset] = (char)(tmp % 10 + '0');
        tmp /= 10;
        offset++;
      }
      for (int i = offset - 1; i >= 0; i--) {
        DO_NEXT(c[i]);
      }
      cnt += 2;
    } else if (fmt[cnt] == '%' && fmt[cnt + 1] == '0' && fmt[cnt + 2] == '2' &&
               fmt[cnt + 3] == 'd') {
      int tmp = va_arg(ap, int);
      int int_len = 0;
      char int_string[30];
      bool neg_flag = false;
      if (tmp == 0) {
        DO_NEXT('0');
        DO_NEXT('0');
        cnt += 4;
        continue;
      }
      if (tmp < 0) {
        tmp = -tmp;
        DO_NEXT('-');
        neg_flag = true;
      }
      while (tmp) {
        int_string[int_len++] = (char)(tmp % 10 + '0');
        tmp /= 10;
      }
      for (int i = 2; i > int_len; i--) {
        if (i == 2 && neg_flag)
          continue;
        DO_NEXT('0');
      }
      for (int i = int_len - 1; i >= 0; i--) {
        DO_NEXT(int_string[i]);
      }
      cnt += 4;
    } else if (fmt[cnt] == '%' && fmt[cnt + 1] == 's') {
      char *tmp = va_arg(ap, char *);
      while (*tmp != '\0') {
        DO_NEXT(*tmp);
        tmp++;
      }
      cnt += 2;
    } else {
      DO_NEXT(fmt[cnt]);
      cnt++;
    }
  }
  va_end(ap);
  return re;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {

  int re = 0;
  char c[10000];
  int cnt = 0;
  va_list ap;
  va_start(ap, fmt);
  while (fmt[cnt] != '\0') {
    if (fmt[cnt] == '%' && fmt[cnt + 1] == 'd') {
      int tmp = va_arg(ap, int);
      int offset = 0;
      if (tmp == 0) {
        DO_NEXT2('0');
        cnt += 2;
        continue;
      }
      if (tmp < 0) {
        tmp = -tmp;
        DO_NEXT2('-');
      }
      while (tmp != 0) {
        c[offset] = (char)(tmp % 10 + '0');
        tmp /= 10;
        offset++;
      }
      for (int i = offset - 1; i >= 0; i--) {
        DO_NEXT2(c[i]);
      }
      cnt += 2;
    }
    if (fmt[cnt] == '%' && fmt[cnt + 1] == 'u') {
      unsigned tmp = va_arg(ap, unsigned int);
      int offset = 0;
      if (tmp == 0) {
        DO_NEXT2('0');
        cnt += 2;
        continue;
      }
      while (tmp != 0) {
        c[offset] = (char)(tmp % 10 + '0');
        tmp /= 10;
        offset++;
      }
      for (int i = offset - 1; i >= 0; i--) {
        DO_NEXT2(c[i]);
      }
      cnt += 2;
    }
    if (fmt[cnt] == '%' && fmt[cnt + 1] == '0' && fmt[cnt + 2] == '2' &&
        fmt[cnt + 3] == 'd') {
      int tmp = va_arg(ap, int);
      int int_len = 0;
      char int_string[30];
      bool neg_flag = false;
      if (tmp == 0) {
        DO_NEXT2('0');
        DO_NEXT2('0');
        cnt += 4;
        continue;
      }
      if (tmp < 0) {
        tmp = -tmp;
        DO_NEXT2('-');
        neg_flag = true;
      }
      while (tmp) {
        int_string[int_len++] = (char)(tmp % 10 + '0');
        tmp /= 10;
      }
      for (int i = 2; i > int_len; i--) {
        if (i == 2 && neg_flag)
          continue;
        DO_NEXT2('0');
      }
      for (int i = int_len - 1; i >= 0; i--) {
        DO_NEXT2(int_string[i]);
      }
      cnt += 4;
    } else if (fmt[cnt] == '%' && fmt[cnt + 1] == 's') {
      char *tmp = va_arg(ap, char *);
      while (*tmp != '\0') {
        DO_NEXT2(*tmp);
        tmp++;
      }
      cnt += 2;
    } else {
      DO_NEXT2(fmt[cnt]);
      cnt++;
    }
  }
  va_end(ap);
  printf("out=%s\n", out);
  return re;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
