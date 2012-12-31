#include <u.h>
#include <avian.h>

char *
vstradd(char *s, ...) {
  va_list ap;
  char *buf, *p;
  uint n;

  n = 1;
  va_start(ap, s);
  for(p = s; p; p = va_arg(ap, char *))
    n += strlen(p);
  va_end(ap);
  buf = malloc(n);
  if(buf == nil)
    return nil;
  *buf = 0;
  va_start(ap, s);
  for(p = s; p; p = va_arg(ap, char *))
    strcat(buf, p);
  va_end(ap);
  return buf;
}
