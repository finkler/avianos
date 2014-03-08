#include <u.h>
#include <avian.h>

char *
ltrim(char *s) {
  while(isspace(*s))
    s++;
  return s;
}
