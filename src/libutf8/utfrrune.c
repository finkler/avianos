#include <u.h>
#include <utf8.h>

char*
utfrrune(char *s, rune r)
{
  char buf[UTF_MAX], *p;
  int n;

  n = runeenc(buf, r);
  for(p = s+strlen(s); p != s; p--)
    if(strncmp(p, buf, n) == 0)
      return p;
  return nil;
}
