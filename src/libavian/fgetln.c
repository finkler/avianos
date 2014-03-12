#include <u.h>
#include <avian.h>

char *
fgetln(FILE *stream) {
  static char line[LINE_MAX+1];
  char c;
  int i, n;

  i = 0;
  while((n = fread(&c, sizeof c, 1, stream)) > 0) {
    if(c == '\b' && i > 0) {
      i--;
      while(i > 0 && line[i] <= 0xC0 && line[i] >= 0x80)
        i--;
      continue;
    }
    if(c == '\n')
      break;
    line[i++] = c;
    if(i == LINE_MAX)
      break;
  }
  if(n < 1)
    return nil;
  line[i] = '\0';
  return line;
}
