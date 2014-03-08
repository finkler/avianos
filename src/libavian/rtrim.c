#include <u.h>
#include <avian.h>

char *
rtrim(char *s) {
  char *p;

  p = s + strlen(s);
  while(isspace(*p))
    p--;
  *p = '\0';
  return s;
}
