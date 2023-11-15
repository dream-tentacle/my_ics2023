#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>
// macro concatenation
#define concat_temp(x, y) x##y
#define concat(x, y) concat_temp(x, y)
#define concat3(x, y, z) concat(concat(x, y), z)
#define concat4(x, y, z, w) concat3(concat(x, y), z, w)
#define concat5(x, y, z, v, w) concat4(concat(x, y), z, v, w)

// macro testing
// See
// https://stackoverflow.com/questions/26099745/test-if-preprocessor-symbol-is-defined-inside-macro
#define CHOOSE2nd(a, b, ...) b
#define MUX_WITH_COMMA(contain_comma, a, b) CHOOSE2nd(contain_comma a, b)
#define MUX_MACRO_PROPERTY(p, macro, a, b) \
  MUX_WITH_COMMA(concat(p, macro), a, b)
// define placeholders for some property
#define __P_DEF_0 X,
#define __P_DEF_1 X,
#define __P_ONE_1 X,
#define __P_ZERO_0 X,
// define some selection functions based on the properties of BOOLEAN
// macro
#define MUXDEF(macro, X, Y) MUX_MACRO_PROPERTY(__P_DEF_, macro, X, Y)
#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define DO_NEXT(x) MUXDEF(to_putch, out[re++] = (x), putch(x); re++)

int printf(const char *fmt, ...) {
#define to_putch
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
    }
    if (fmt[cnt] == '%' && fmt[cnt + 1] == 'u') {
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
    }
    if (fmt[cnt] == '%' && fmt[cnt + 1] == '0' && fmt[cnt + 2] == '2' &&
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
        if (i == 2 && neg_flag) continue;
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
#undef to_putch
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
    }
    if (fmt[cnt] == '%' && fmt[cnt + 1] == 'u') {
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
    }
    if (fmt[cnt] == '%' && fmt[cnt + 1] == '0' && fmt[cnt + 2] == '2' &&
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
        if (i == 2 && neg_flag) continue;
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

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
