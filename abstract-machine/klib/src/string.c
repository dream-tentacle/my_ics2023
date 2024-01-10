#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t i;
  for (i = 0; s[i] != '\0'; i++)
    ;
  return i;
}

char *strcpy(char *dst, const char *src) {
  size_t i;

  for (i = 0; src[i] != '\0'; i++)
    dst[i] = src[i];
  dst[i] = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i;

  for (i = 0; i < n && src[i] != '\0'; i++)
    dst[i] = src[i];
  for (; i < n; i++)
    dst[i] = '\0';

  return dst;
}

char *strcat(char *dst, const char *src) {
  size_t dest_len = strlen(dst);
  size_t i;

  for (i = 0; src[i] != '\0'; i++)
    dst[dest_len + i] = src[i];
  dst[dest_len + i] = '\0';

  return dst;
}

int strcmp(const char *s1, const char *s2) {
  size_t i;
  for (i = 0; s1[i] == s2[i] && s1[i] != '\0'; i++)
    ;
  return s1[i] - s2[i];
}

int strncmp(const char *s1, const char *s2, size_t n) {
  size_t i;
  for (i = 0; i < n && s1[i] == s2[i] && s1[i] != '\0'; i++)
    ;
  return s1[i] - s2[i];
}

void *memset(void *s, int c, size_t n) {
  size_t i;
  unsigned char *ch = (unsigned char *)s;
  for (i = 0; i < n; i++)
    ch[i] = c;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  int i;
  unsigned char *ch = (unsigned char *)dst;
  unsigned char *ch2 = (unsigned char *)src;
  if (ch > ch2) {
    for (i = n - 1; i >= 0; i--)
      ch[i] = ch2[i];
  } else {
    for (i = 0; i < n; i++)
      ch[i] = ch2[i];
  }

  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  size_t i;
  unsigned char *ch = (unsigned char *)out;
  unsigned char *ch2 = (unsigned char *)in;
  for (i = 0; i < n; i++)
    ch[i] = ch2[i];

  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  size_t i;
  const unsigned char *ch1 = s1, *ch2 = s2;
  for (i = 0; i < n; i++)
    if (ch1[i] != ch2[i])
      return ch1[i] - ch2[i];
  return 0;
}

#endif
