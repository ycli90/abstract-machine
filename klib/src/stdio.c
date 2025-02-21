#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

enum output_type {STDOUT, BUFFER, BUFFER_N};

static int output_char(char *out, size_t limit, char c, size_t output_count, enum output_type type) {
  switch (type){
  case STDOUT:
    putch(c);
    break;
  case BUFFER:
    out[output_count] = c;
    break;
  case BUFFER_N:
    if (output_count < limit - 1) {
      out[output_count] = c;
    } else if (output_count == limit - 1) {
      out[output_count] = '\0';
    }
    break;
  default:
    panic("output_char: unsupported output_type");
  }
  return c ? output_count + 1 : output_count;
}

static int output_int_dec(char *out, size_t limit, long long d, bool has_sign, char flag, int width, size_t output_count, enum output_type type) {
  unsigned long long ud = d;
  if (has_sign && d < 0) {
    ud = -d;
    output_count = output_char(out, limit, '-', output_count, type);
    if (width > 0) --width;
  }
  char digits[20];
  int n = 0;
  do {
    assert(n < 20);
    digits[n] = ud % 10;
    ud /= 10;
    ++n;
  } while (ud);
  int padding = 0;
  if (width > n) padding = width - n;
  while (padding--) output_count = output_char(out, limit, flag, output_count, type);
  for (int i = n - 1; i >= 0; --i) {
    output_count = output_char(out, limit, '0' + digits[i], output_count, type);
  }
  return output_count;
}

static int output_int_hex(char *out, size_t limit, unsigned long long ud, char flag, int width, bool capital, size_t output_count, enum output_type type) {
  char digits[16];
  int n = 0;
  do {
    assert(n < 16);
    digits[n] = ud & 0xf;
    ud >>= 4;
    ++n;
  } while (ud);
  int padding = 0;
  if (width > n) padding = width - n;
  while (padding--) output_count = output_char(out, limit, flag, output_count, type);
  char char_base = capital ? 'A' : 'a';
  for (int i = n - 1; i >= 0; --i) {
    char c = digits[i] < 10 ? '0' + digits[i] : char_base + digits[i] - 10;
    output_count = output_char(out, limit, c, output_count, type);
  }
  return output_count;
}

static int output_string(char *out, size_t limit, const char *s, size_t output_count, enum output_type type) {
  if (s == NULL) return output_count;
  while (*s) {
    output_count = output_char(out, limit, *s, output_count, type);
    ++s;
  }
  return output_count;
}

static int output_format(char *out, size_t n, const char *fmt, va_list ap, enum output_type type) {
  bool percent = false;
  char flag = 0;
  int width = 0;
  int flag_long = 0;
  int output_count = 0;
  while (*fmt) {
    if (percent) {
      switch (*fmt) {
      case '%':
        output_count = output_char(out, n, '%', output_count, type);
        percent = false;
        break;
      case 'c': {
        char c = va_arg(ap, int);
        output_count = output_char(out, n, c, output_count, type);
        percent = false;
        break;
      }
      case 'd': case 'u': {
        long long d;
        if (flag_long == 2) d = va_arg(ap, long long);
        else if (flag_long == 1) d = va_arg(ap, long);
        else d = va_arg(ap, int);
        output_count = output_int_dec(out, n, d, *fmt == 'd', flag, width, output_count, type);
        percent = false;
        break;
      }
      case 'x': case 'X': {
        unsigned long long ud;
        if (flag_long == 2) ud = va_arg(ap, unsigned long long);
        else if (flag_long == 1) ud = va_arg(ap, unsigned long);
        else ud = va_arg(ap, unsigned int);
        output_count = output_int_hex(out, n, ud, flag, width, *fmt == 'X', output_count, type);
        percent = false;
        break;
      }
      case 'p': {
        output_count = output_string(out, n, "0x", output_count, type);
        unsigned long ud = va_arg(ap, unsigned long);
        output_count = output_int_hex(out, n, ud, flag, width, false, output_count, type);
        percent = false;
        break;
      }
      case 's': {
        const char *s = va_arg(ap, const char *);
        output_count = output_string(out, n, s, output_count, type);
        percent = false;
        break;
      }
      case '0':
        if (flag) {
          if (width == 0) panic("output_format: repeated flag");
          else width = width * 10 + *fmt - '0';
        } else {
          flag = *fmt;
        }
        break;
      case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        width = width * 10 + *fmt - '0';
        break;
      case 'l':
        if (flag_long < 2) flag_long++;
        break;
      default:
        char buf[64];
        sprintf(buf, "output_format: unsupported format: %c", *fmt);
        panic(buf);
      }
    } else {
      if (*fmt == '%') {
        percent = true;
        flag = 0;
        width = 0;
        flag_long = 0;
      } else {
        output_count = output_char(out, n, *fmt, output_count, type);
      }
    }
    ++fmt;
  }
  output_count = output_char(out, n, '\0', output_count, type);
  return output_count;
}

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = output_format(NULL, 0, fmt, ap, STDOUT);
  va_end(ap);
  return ret;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return output_format(out, 0, fmt, ap, BUFFER);
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = vsprintf(out, fmt, ap);
  va_end(ap);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return ret;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  return output_format(out, n, fmt, ap, BUFFER_N);
}

#endif
