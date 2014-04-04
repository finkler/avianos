#include <u.h>
#include <utf8.h>

int
runestart(int c)
{
  if(c > 0xC0)
    return 1;
  return 0;
}
