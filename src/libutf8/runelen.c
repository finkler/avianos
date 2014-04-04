#include <u.h>
#include <utf8.h>

int
runelen(rune r)
{
  int i;
  uint32 n;

  n = RUNE_SELF;
  for(i = 1; i <= UTF_MAX; i++){
    if(r < n)
      return i;
    if(i == 1)
      n <<= 4;
    else
      n <<= 5;
  }
  return 0;
}
