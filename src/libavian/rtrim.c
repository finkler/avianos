#include <u.h>
#include <avian.h>

char *
rtrim(char *s) {
  char *p;

  p = s + strlen(s);
  while(isblank(*p))
    p--;
  *p = '\0';
  return s;
}
