#include <avian.h>

#define INCR 128

char *
fgetln(FILE *stream) {
  char *buf, c;
  int cap, len;
  
  cap = INCR;
  len = 0;
  buf = malloc(cap);
  if(buf == nil)
    return nil;
  for(;;) {
    c = fgetc(stream);
    if(c == EOF || c == '\n')
      break;
    if(len == cap) {
      cap += INCR;
      buf = realloc(buf, cap);
    }
    buf[len++] = c;
  }
  buf[len] = '\0';
  if(len > 0 && !ferror(stream))
    return buf;
  free(buf);
  return nil;
}
