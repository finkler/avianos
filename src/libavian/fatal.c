#include <u.h>
#include <avian.h>

static void vlog(char *, va_list);

char *argv0 = nil;

void
__fixargv0(char *s) {
  argv0 = strrchr(s, '/');
  if(argv0)
    argv0++;
  else
    argv0 = s;
}

void
alert(char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  vlog(fmt, ap);
  va_end(ap);
}

void
fatal(int rval, char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  vlog(fmt, ap);
  va_end(ap);
  exit(rval);
}

void
vlog(char *fmt, va_list ap) {
  int n;

  if(argv0)
    fprintf(stderr, "%s: ", argv0);
  vfprintf(stderr, fmt, ap);
  n = strlen(fmt);
  if(n > 0 && fmt[n-1] != '\n')
    fprint(stderr, "\n");
}
