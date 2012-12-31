#include <u.h>
#include <avian.h>

char *argv0 = nil;

void
__fixargv0(char *s) {
  argv0 = strrchr(s, '/');
  if(argv0)
    argv0++;
  else
    argv0 = s;
}
