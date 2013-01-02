#include <u.h>
#include <utf8.h>

int
utfnlen(char *s, int maxlen) {
  int n;

  n = utflen(s);
  return min(n, maxlen);
}