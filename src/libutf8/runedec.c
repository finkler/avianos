#include <u.h>
#include <utf8.h>

int
runedec(rune *r, char *s)
{
  int i, n;

  n = fullrune(*s);
  if(n == 1){
    *r = *s;
    return 1;
  }
  if(n == 0){
    *r = RUNE_ERROR;
    return 1;
  }
  *r = s[0]&(0xFF>>(n+1));
  for(i = 1; i < n; i++){
    *r <<= 6;
    *r |= s[i]&0x3F;
  }
  return i;
}
