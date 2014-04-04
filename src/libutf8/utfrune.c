#include <u.h>
#include <utf8.h>

char*
utfrune(char *s, rune r)
{
  char buf[UTF_MAX+1];
  int n;

  n = runeenc(buf, r);
  buf[n] = '\0';
  return strstr(s, buf);
}
