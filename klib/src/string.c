#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  while (*s++) ++len;
  return len;
}

char *strcpy(char *dst, const char *src) {
  size_t len = strlen(src);
  memcpy(dst, src, len + 1);
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t len = strlen(src);
  if (len > n) len = n;
  memcpy(dst, src, len);
  memset(dst + len, 0, n - len);
  return dst;
}

char *strcat(char *dst, const char *src) {
  char *s = dst;
  while (*dst) ++dst;
  strcpy(dst, src);
  return s;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && *s2 && *s1 == *s2) {
    ++s1;
    ++s2;
  }
  return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  if (n == 0) return 0;
  size_t i = 0;
  while (i < n && *s1 && *s2 && *s1 == *s2) {
    ++s1;
    ++s2;
    ++i;
  }
  return (unsigned char)*s1 - (unsigned char)*s2;
}

void *memset(void *s, int c, size_t n) {
  while (n--) {
    *(char*)(s + n) = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  if (dst < src) {
    for (size_t i = 0; i < n; ++i) {
      ((char*)dst)[i] = ((char*)src)[i];
    }
  } else if (dst > src) {
    for (size_t i = n - 1; i < n; --i) {
      ((char*)dst)[i] = ((char*)src)[i];
    }
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    ((char*)out)[i] = ((char*)in)[i];
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    int diff = *(unsigned char*)(s1 + i) - *(unsigned char*)(s2 + i);
    if (diff != 0) return diff;
  }
  return 0;
}

#endif
