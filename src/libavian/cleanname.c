#include <u.h>
#include <avian.h>

char*
cleanname(char *s)
{
  int n;
  char *p;

  while((p = strstr(s, "//")))
    for(n = 0; n < strlen(p); n++)
      p[n] = p[n+1];
  n = strlen(s);
  if(n > 1 && s[n-1] == '/')
    s[n-1] = '\0';
  return s;
}
