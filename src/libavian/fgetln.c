#include <u.h>
#include <avian.h>

char*
fgetln(FILE *stream)
{
  static char line[LINE_MAX+1];
  int c, n;

  n = 0;
  while((c = fgetc(stream)) != EOF){
    if(c == '\b' && n > 0){
      n--;
      while(n > 0 && line[n] < 0xc1 && line[n] > 0x7f)
        n--;
      continue;
    }
    if(c == '\n')
      break;
    line[n++] = c;
    if(n == LINE_MAX)
      break;
  }
  if(c == EOF && n == 0)
    return nil;
  line[n] = '\0';
  return line;
}
