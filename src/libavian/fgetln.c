#include <u.h>
#include <avian.h>

char *
fgetln(FILE *stream) {
  static char line[LINE_MAX+1];
  char c;
  int i, n;

  i = 0;
  while((n = fread(&c, 1, 1, stream)) > 0) {
    if(c == '\b' && i > 0) {
      i--;
      while(i > 0 && line[i] < 0xc1 && line[i] > 0x7f)
        i--;
      continue;
    }
    if(c == '\n')
      break;
    line[i++] = c;
    if(i == LINE_MAX)
      break;
  }
  if(n < 0 || (n == 0 && i == 0))
    return nil;
  line[i] = '\0';
  return line;
}
