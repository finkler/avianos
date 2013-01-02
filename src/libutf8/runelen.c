#include <u.h>
#include <utf8.h>

int
runelen(rune r) {
  int i;
  rune n;

  n = RUNE_SELF;
  for(i = 1; i <= UTF_MAX; i++) {
    if(r < n)
      return i;
    if(i == 1)
      n <<= 4;
    else
      n <<= 5;
  }
  return 0;
}