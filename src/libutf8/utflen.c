#include <u.h>
#include <utf8.h>

uint
utflen(char *s)
{
  uint n;
  char *p;

  n = 0;
  for(p = s; *p; p++)
    if(*p < RUNE_SELF || runestart(*p))
      n++;
  return n;
}
