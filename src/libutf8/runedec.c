#include <u.h>
#include <utf8.h>

int
runedec(rune *r, char *s) {
  uchar h, *p;
  int i, n;

  p = (uchar *)s;
  if(*p < RUNE_SELF) {
    *r = *p;
    return 1;
  }
  h = (0xFF<<(8-UTF_MAX))&0xFF;
  for(n = UTF_MAX; n > 1; n--) {
    if((*p&h) == h)
      break;
    h <<= 1;
  }
  if(n == 1) {
    *r = RUNE_ERROR;
    return 1;
  }
  *r = p[0]&(0xFF>>(n+1));
  for(i = 1; i < n; i++) {
    *r <<= 6;
    *r |= p[i]&0x3F;
  }
  return i;
}
