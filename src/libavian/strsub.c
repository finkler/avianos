#include <u.h>
#include <avian.h>

char *
strsub(char *h, char *n, char *s) {
  char *p, *q;
  int d, len;

  p = strstr(h, n);
  if(p == nil)
    return nil;
  d = p - h;
  len = d;
  len += strlen(s);
  p += strlen(n);
  len += strlen(p);
  q = malloc(len+1);
  if(q == nil)
    return nil;
  strncpy(q, h, d);
  q[len] = '\0';
  strcat(q, s);
  strcat(q, p);
  return q;
}
