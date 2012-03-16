#include <u.h>
#include <avian.h>

char *
fgetln(FILE *stream) {
  static char line[LINE_MAX];
  int n;
  
  if(fgets(line, LINE_MAX, stream) == nil)
    return nil;
  n = strlen(line);
  if(n > 0 && line[n-1] == '\n')
    line[n-1] = '\0';
  return line;
}
