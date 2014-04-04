#include <u.h>
#include <utf8.h>

int
runeenc(char *buf, rune r)
{
  uchar h;
  int i, n;

  if(r < RUNE_SELF){
    buf[0] = (char)r;
    return 1;
  }
  n = runelen(r);
  h = 0xFF<<(8-n);
  buf[0] = h|(r>>(6*(n-1)));
  for(i = 1; i < n; i++)
    buf[i] = 0x80|((r>>(6*(n-i-1)))&0x3F);
  return i;
}
