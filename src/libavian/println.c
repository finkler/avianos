#include <u.h>
#include <avian.h>

int
println(char *fmt, ...) {
  static int len, n;
  va_list ap;
  char *s;

  if(len == 0) {
    len = textwidth();
    s = malloc(len+1);
  }
  va_start(s, ap);
  vsnprintf(s, len, fmt, ap);
  va_end(ap);
  if(n+strlen(s) > len) {
    fputc('\n', stdout);
    n = 0;
  }
  if(n)
    fputc(' ', stdout);
  fputs(s, stdout);
  n += strlen(s);
  return n;
}
