#include <u.h>
#include <utf8.h>

int
utflen(char *s) {
  int n;
  char *p;

  n = 0;
  for(p = s; p; p++)
    if(*p < RUNE_SELF || runestart(*p))
      n++;
  return n;
}