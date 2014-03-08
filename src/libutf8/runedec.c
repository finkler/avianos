#include <u.h>
#include <utf8.h>

int
runedec(rune *r, char *s) {
  uchar h;
  int i, n;

  if(*s < RUNE_SELF) {
    *r = *s;
    return 1;
  }
  h = 0xFF<<(8-UTF_MAX);
  for(n = UTF_MAX; n > 1; n--) {
    if((*s&h) == h)
      break;
    h <<= 1;
  }
  if(n == 1) {
    *r = RUNE_ERROR;
    return 1;
  }
  *r = s[0]&(0xFF>>(8-n-1));
  for(i = 1; i < n; i++) {
    *r <<= 6;
    *r |= s[i]&0xF;
  }
  return i;
}
