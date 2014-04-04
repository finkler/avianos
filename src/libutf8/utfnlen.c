#include <u.h>
#include <utf8.h>

uint
utfnlen(char *s, uint maxlen)
{
  uint n;

  n = utflen(s);
  return min(n, maxlen);
}
