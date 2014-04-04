#include <u.h>
#include <avian.h>

uint
longlen(vlong x)
{
  uint n;

  n = 1;
  if(x < 0){
    n = 2;
    x = -x;
  }
  if(x >= 10000000000000000){
    n += 16;
    x /= 10000000000000000;
  }
  if(x >= 100000000){
    n += 8;
    x /= 100000000;
  }
  if(x >= 10000){
    n += 4;
    x /= 10000;
  }
  if(x >= 100){
    n += 2;
    x /= 100;
  }
  if(x >= 10)
    n++;
  return n;
}
