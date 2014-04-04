#include <u.h>
#include <utf8.h>

int
fullrune(int c)
{
  uchar h;
  int n;

  if(c < RUNE_SELF)
    return 1;
  h = (0xFF<<(8-UTF_MAX))&0xFF;
  for(n = UTF_MAX; n > 1; n--){
    if((((uchar)c)&h) == h)
      break;
    h <<= 1;
  }
  if(n == 1)
    return 0;
  return n;
}
