#include <u.h>
#include <avian.h>

char*
ltrim(char *s)
{
  while(isblank(*s))
    s++;
  return s;
}
