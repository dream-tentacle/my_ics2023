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
      for (int i = offset - 1; i >= 0; i--) {
        putch(c[i]);
        re++;
      }
      cnt += 2;
    }
    if (fmt[cnt] == '%' && fmt[cnt + 1] == '0' &&
        fmt[cnt + 2] == '2' && fmt[cnt + 3] == 'd') {
      int tmp = va_arg(ap, int);
      int int_len = 0;
      char int_string[30];
      while (tmp) {
        int_string[int_len++] = (char)(tmp % 10 + '0');
        tmp /= 10;
      }
      for (int i = 2; i > int_len; i--) {
        putch('0');
        re++;
      }
      for (int i = int_len - 1; i >= 0; i--) {
        putch(int_string[i]);
        re++;
      }
      cnt += 4;
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
  va_end(ap);
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
      int int_len = 0;
      char int_string[30];
      while (tmp) {
        int_string[int_len++] = (char)(tmp % 10 + '0');
        tmp /= 10;
      }
      for (int i = int_len - 1; i >= 0; i--) {
        out[re++] = int_string[i];
      }
      cnt += 2;
    }
    if (fmt[cnt] == '%' && fmt[cnt + 1] == '0' &&
        fmt[cnt + 2] == '2' && fmt[cnt + 3] == 'd') {
      int tmp = va_arg(ap, int);
      int int_len = 0;
      char int_string[30];
      while (tmp) {
        int_string[int_len++] = (char)(tmp % 10 + '0');
        tmp /= 10;
      }
      for (int i = 2; i > int_len; i--) {
        out[re++] = '0';
      }
      for (int i = int_len - 1; i >= 0; i--) {
        out[re++] = int_string[i];
      }
      cnt += 4;
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
  va_end(ap);
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
