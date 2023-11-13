#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

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
      while (tmp) {
        c[offset] = (char)(tmp % 10 + '0');
        tmp /= 10;
        offset++;
      }
      for (int i = 0; i < offset; i++) {
        putch(c[i]);
        re++;
      }
      cnt += 2;
    } else if (fmt[cnt] == '%' && fmt[cnt + 1] == 's') {
      char *tmp = va_arg(ap, char *);
      while (*tmp != '\0') {
        putch(*tmp);
        re++;
        tmp++;
      }
      cnt += 2;
    } else {
      putch(fmt[cnt]);
      cnt++;
      re++;
    }
  }
  return re;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  int re = 0;
  int cnt = 0;
  va_list ap;
  va_start(ap, fmt);
  while (fmt[cnt] != '\0') {
    if (fmt[cnt] == '%' && fmt[cnt + 1] == 'd') {
      int tmp = va_arg(ap, int);
      int offset = 0;
      while (tmp) {
        out[re + offset] = (char)(tmp % 10 + '0');
        tmp /= 10;
        offset++;
      }
      char x;
      for (int i = 0; i < offset / 2; i++) {
        x = out[re + i];
        out[re + i] = out[re + offset - 1 - i];
        out[re + offset - 1 - i] = x;
      }
      re += offset;
      cnt += 2;
    } else if (fmt[cnt] == '%' && fmt[cnt + 1] == 's') {
      char *tmp = va_arg(ap, char *);
      while (*tmp != '\0') {
        out[re] = *tmp;
        re++;
        tmp++;
      }
      cnt += 2;
    } else {
      out[re] = fmt[cnt];
      re++;
      cnt++;
    }
  }
  out[re] = '\0';
  return re;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
