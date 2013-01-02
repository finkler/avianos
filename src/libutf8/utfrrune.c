#include <u.h>
#include <utf8.h>

char *
utfrrune(char *s, rune r) {
  char buf[UTF_MAX], *p;
  int n;

  n = runeenc(buf, r);
  for(p = s+strlen(s); p != s; p--)
    if(!strncmp(p, buf, n))
      return p;
  return nil;
}